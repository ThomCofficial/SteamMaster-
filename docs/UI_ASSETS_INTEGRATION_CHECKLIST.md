# SteamMaster approved UI integration checklist (2026-11-10)

Base: `vst3/v1.7-performance-character`. Preserve audio DSP and the approved module designs.

Nine archives were supplied in chat (230 files, 205 verified PNGs); archives are not yet committed here. Their controls/masks are reference materials, not all production-ready sprites. Do not substitute a flattened PNG for live controls.

## Ordered tasks
1. Build proportion-preserving resizable rack and Global Header using existing APVTS parameters. Verify host automation/session recall.
2. Reconstruct knob-free, text-correct static faceplates while preserving locked artwork. Draw scales/ticks in JUCE and use true animated rotary/buttons. Never rotate scale-bearing raster cutouts.
3. Ocean/Furnace: reconcile thresholds, ratio and time scales to existing DSP ranges.
4. Parametric: reconcile band mode/range mappings.
5. Pultec: correct three frequency rings; avoid prototype underlays and rotor sprites in final UI.
6. Shadow: use 13 exact APVTS mappings; use meaningful GR metering (OUTPUT selector is not yet backed by output telemetry).
7. Exciter: low/high drive, frequency, crossover artwork scales differ from DSP; render DSP-accurate values; rebuild VU needle.
8. Baxandall: preserve original 14-knob locked module and independent mid/side bypass/link/routing controls.
9. Tape: seven live DSP controls map; reels need separate stationary underlay and decorative animation.
10. Stereo Image/Limiter: stereo IN is not yet an independent parameter; correct threshold/ceiling scales; add GUI-safe correlation and gain-reduction telemetry.
11. Assemble all units; inspect host automation, presets/state, resizing, rendering stability, channel routing, offline render and meter update overhead.
12. Build Windows VST3; run test suites and plugin validation; final Cubase 13 hands-on validation needed.

All DSP mismatches in baked artwork must be repaired visually, not by changing proven DSP. Keep rendering/telemetry off the audio thread. Do not claim the finished plugin until the approved visuals, interactive controls, VST3 build and validation have all passed.
