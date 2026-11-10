# SteamMaster — Stereo Image + Limiter complete module handoff

## Verified against repository (read-only)
`ThomCofficial/SteamMaster-` → `vst3/v1.7-performance-character`

**No GitHub writes, DSP changes, plugin builds, Cubase tests, or JUCE implementations.**

## Package overview

- `00_source/`: untouched approved high-resolution module art (1536×512).
- `01_panels/`: full stereo/limiter/decorative **reference crops** preserving context.
- `02_control_references/`: 15 transparent reference cutouts (8 knobs, 3 toggles, 1 auto button, 1 mode radio area, 2 meters). These are **reference-only**; not finished rotating sprites or needle-free meter faces.
- `03_masks/`: individual alpha masks and whole-panel control-region mask to guide later clean faceplate reconstruction.
- `04_layout/`: JSON parameter mapping with exact processor IDs/ranges/defaults, and coordinates CSV.
- `05_QA/`: annotated original and all-control reference sheet.

## Exact DSP mapping

### Stereo Image
- `stereo_mono_maker_hz`: 0=Off; 0..300 Hz. Artwork labels Off,20,40,60,100,150,200,300.
- `stereo_width_percent`: 0..200%, default 100%.
- `stereo_width_freq`: 800..5000 Hz, default 2000 Hz.
- `stereo_in` control **unresolved:** no dedicated parameter exists. Do not link it to `global_in` (that bypasses the entire suite).
- Correlation meter: `GlobalStereo::correlation()` exists in DSP but NOT in a GUI-safe metering bridge.

### Limiter
- `limiter_in`: bool
- `limiter_threshold_db`: -24..0 dB
- `limiter_ceiling_dbtp`: **-2..0 dBTP**
- `limiter_release_ms`: 10..1000 ms
- `limiter_auto_release`: bool
- `limiter_lookahead_ms`: 0.1..5.0 ms
- `limiter_stereo_link`: 0..100%
- `limiter_true_peak`: bool
- `limiter_mode`: Clean/Punch/Modern
- Gain-reduction meter: `MasterLimiter::gainReductionDb()` exists but is not connected to the editor safely.

## Art audit: actual errors

1. **Threshold:** -16 and -20 labels are out of order clockwise on the left side of the knob. Code must correct the scale.
2. **Ceiling:** -3.0 appears despite the DSP minimum of -2.0, and -1.0 is duplicated. Exact corrected scale suggested: -2.0, -1.5, -1.0, -0.5, 0.0.
3. The stereo panel's IN toggle is not represented by an independent processor parameter; leaving it visually present is not the same as claiming it works.
4. Both meters currently show static needles. Dynamic needle/meter face separation is a later UI construction step.

Other visible labels and modules are consistent with ranges found in processor code.

## Art processing status

**Ready:** untouched source, full region references, individual reference cutouts, mask coordinates, parameter/data spec.
**Not yet production-ready:** clean behind-control faceplate, rotating knob masters, independently animated toggle/lamp states, needle-free meter faces, vector corrected scale typography.

These cannot be reliably isolated merely by cutting pixels from a flattened image. Do NOT publish the enclosed reference cutouts as final JUCE assets or claim the editor is implemented.

## Next integration milestone

After final module artwork handoff, start a standalone `ui-development` branch (with approval) and build the **Global Header first** to prove real controls, correct scale render, resizing, automation and editor lifecycle. Then progress in rack order. Keep DSP untouched.
