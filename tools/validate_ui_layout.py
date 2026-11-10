#!/usr/bin/env python3
"""Fail fast when the visual UI omits, duplicates or misplaces an audio control."""
import json
import re
from pathlib import Path
from PIL import Image

root=Path(__file__).resolve().parents[1]
layout=json.loads((root/"tools/ui_layout.json").read_text())["modules"]
source=(root/"plugin/PluginProcessor.cpp").read_text()
all_ids=re.findall(r'juce::ParameterID\{"([^"]+)"',source)
if len(all_ids)!=107 or len(set(all_ids))!=107:
    raise AssertionError(f"Expected 107 stable, unique APVTS parameter IDs; found {len(all_ids)}")
covered=["global_in","global_input_db","global_output_db","global_oversampling"]
for module,info in layout.items():
    original=root/"plugin/assets"/info["source"]
    if not original.is_file():raise AssertionError(f"Original approved module source missing: {original}")
    with Image.open(original) as img:
        if img.size!=(info["width"],info["height"]):
            raise AssertionError(f"Unexpected source image dimensions: {module} {img.size}")
    controls=info["controls"]
    if not controls:raise AssertionError(f"Empty module: {module}")
    for c in controls:
        covered.append(c["id"])
        if c["kind"] not in ("knob","cycle","button"):
            raise AssertionError(f"Unsupported {module}:{c['id']} kind")
        if not (0<=c["x"]<info["width"] and 0<=c["y"]<info["height"]):
            raise AssertionError(f"Out-of-bounds visual control: {module}:{c['id']}")
missing=set(all_ids)-set(covered)
extra=set(covered)-set(all_ids)
duplicate=[x for x in covered if covered.count(x)>1]
if missing or extra or duplicate:
    raise AssertionError(f"UI<>DSP mismatch: missing {sorted(missing)}, extra {sorted(extra)}, duplicates {sorted(set(duplicate))}")
print(f"PASS: all {len(all_ids)} DSP parameter IDs mapped exactly once across ten modules.")
print("PASS: nine module artwork sources match their original dimensions.")
