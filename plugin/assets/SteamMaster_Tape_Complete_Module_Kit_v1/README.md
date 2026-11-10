# What's inside

- `source/`: unmodified original and correctly cropped panel, plus separately sliced visual references. These are the approved appearance and are **not knob-free**.
- `control_refs/`: transparent rotor reference PNGs and illuminated ON switch appearance.
- `reel_refs/`: two transparent reel **references**; they include flattened artwork and need a reconstructed underlay before spinning.
- `masks/`: exact areas in the faceplate that will require background rebuilding. They are masks, **not reconstructed textures**.
- `qa/`: entire panel QA and full control/reel review plus a reference contact sheet.
- `spec/`: read-only checked DSP mapping and machine-readable JUCE positions.

**Production warning:** No output is represented as final animation-ready controls or a clean faceplate. This kit deliberately does not destroy scales or faceplate details with automatic inpainting.
