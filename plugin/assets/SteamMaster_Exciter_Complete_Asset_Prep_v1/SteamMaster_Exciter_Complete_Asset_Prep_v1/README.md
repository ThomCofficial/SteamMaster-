# SteamMaster — Exciter complete module handoff (asset preparation v1)

**Approved design: preserved. DSP: unchanged. GitHub: read-only. Cubase tests: not performed.**

## Material delivered

- Original **1536×512** user artwork, and full **1536×402** physical module crop.
- Reference cutouts for **all seven knob rotors** (2 drives, 2 frequencies, crossover, output, mix).
- Reference cutouts for three mode buttons plus IN, two decorative lamps, and the full VU meter.
- Full-size geometry/control-removal masks and a **read-only prototype** inpaint, labeled **NOT PRODUCTION**.
- Machine-readable coordinate manifest, verified parameter names/types/ranges/defaults, and exact scale data for JUCE rendering.
- Full-module QA review and control reference contact sheet.

## Double-check results

| Control | Current image | Actual processor DSP mapping | Action before JUCE release |
|---|---|---|---|
| MODE | Tube / Tape / Transformer | `exciter_mode`, same choices | Pass |
| IN | On/off | `exciter_in`, bool | Pass |
| LOW DRIVE | 0–10 printed | `exciter_low_drive`, **0–20** | **Replace scale in JUCE** |
| LOW FREQ | Printed 30–400 Hz | `exciter_low_freq`, **20–800 Hz** | **Replace scale in JUCE** |
| HIGH DRIVE | 0–10 printed | `exciter_high_drive`, **0–20** | **Replace scale in JUCE** |
| HIGH FREQ | Printed 1.5–12 kHz | `exciter_high_freq`, **1–18 kHz** | **Replace scale in JUCE** |
| CROSSOVER | Printed 250–3000 Hz | `exciter_crossover`, **100–5000 Hz** | **Replace scale in JUCE** |
| OUTPUT | −12..+12 dB | `exciter_output_db`, **−12..+12 dB** | Pass |
| MIX | 0–100% | `exciter_mix`, **0–100%** | Pass |
| VU | Needle baked into artwork | No parameter ID or published meter API | Separate physical face and dynamic JUCE needle, then add *read-only* meter telemetry without modifying DSP processing |

## Rendering strategy

Keep ornate brass and teal/cream decorative faceplate as artwork. **All numeric scales, ticks, and changing values must be drawn in code** so the original source's incorrect numbers are never used in a production UI. Rendering should not paste a dynamic knob on top of a baked static knob. The inpainted placeholder is *only* a working mask validation; it is not a finished underlay. Need a texture-faithful knob-free panel with manual cleanup or original layered assets before implementation. 

Rotor references are pixel extracts for texture reference and rotational prototypes. They have not yet been tested for rotating without baked-ring artifacts; rebuild using the same style if necessary. Meter needle must be independent, and never fake its movement from button state.

The controls are geometry-measured references only: final hitboxes and scaling must be checked using actual JUCE editor screenshots.

## Source code verified, no changes

`ThomCofficial/SteamMaster-` → `vst3/v1.7-performance-character` → `plugin/PluginProcessor.cpp` (parameter layout and mapping). Existing APVTS state management is reused; no new parameter IDs are proposed.
