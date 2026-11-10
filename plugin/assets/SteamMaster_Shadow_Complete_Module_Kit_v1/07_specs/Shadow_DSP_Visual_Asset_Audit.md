# SteamMaster — Shadow Compressor complete module handoff

**Status:** complete control inventory + clean-cropped rotor **core texture samples** + JUCE geometry; **not a final production faceplate or animation-ready knob set**. DSP & GitHub unchanged.
**Reference:** approved Shadow artwork, 1536 × 768 RGB pixels. The image itself remains the canonical design reference.
**Repo examined (read only):** `ThomCofficial/SteamMaster-`, `vst3/v1.7-performance-character`, `plugin/PluginProcessor.cpp` and `include/steammaster/DSP.hpp`.

## Full visual inventory
- One ornate steel/bronze faceplate, title/plaque/scrollwork, 4 corner screws and 2 decorative hanging lamps.
- Optical stage: IN lighted button, Threshold large knob, Gain large knob.
- Discrete stage: IN lighted button, Threshold, Ratio, Attack, Recovery, Gain knobs.
- Shared controls: IN lighted button, SC HPF, Output, Mix knobs.
- Centre: large 0–20 dB gain-reduction VU, and 3-position Optical/Discrete/Output selector.
- **10 knob instances; 3 IN buttons; 2 stage lamps (visually fused with buttons); 1 VU, 1 UI-only 3-way selector.**

## VERIFIED JUCE APVTS bindings (13)
| UI control | Exact code ID | Allowed values |
|---|---|---|
| Master IN | `shadow_in` | boolean |
| Master Mix | `shadow_mix` | 0–100% |
| Optical IN | `shadow_optical_in` | boolean |
| Optical Threshold | `shadow_optical_threshold` | −40 to +10 dB |
| Optical Gain | `shadow_optical_makeup` | −12 to +18 dB |
| Discrete IN | `shadow_discrete_in` | boolean |
| Discrete Threshold | `shadow_discrete_threshold` | −40 to +10 dB |
| Discrete Gain | `shadow_discrete_gain` | −12 to +18 dB |
| Discrete Ratio | `shadow_discrete_ratio` | 1.2 / 2 / 3 / 4 / 6 / 20 (artwork label FLOOD) |
| Discrete Attack | `shadow_discrete_attack` | 0.1 / 0.3 / 1 / 3 / 10 / 30 ms |
| Discrete Recovery | `shadow_discrete_recovery` | 0.1 / 0.2 / 0.4 / 0.6 / 0.8 / 1.2 s / Dual |
| Sidechain HPF | `shadow_sc_hpf` | OFF / 40 / 80 / 160 / 320 / 500 Hz |
| Output | `shadow_output_db` | −12 to +12 dB |

## Visual issues caught **before approving production artwork**
1. **The optical and discrete Threshold/Gain knobs show 1–24 scale values; those do not represent the APVTS dB ranges.** They must be rendered as exact DSP-native dB values in JUCE. Do not adapt DSP to the wrong artwork.
2. **The VU face says Gain Reduction, but selector has OUTPUT as a mode.** Existing `Shadow::opticalReductionDb()` and `Shadow::discreteReductionDb()` provide stage GR telemetry. OUTPUT mode needs either a separate read-only output level measurement AND meter scale change, or a UI-only label change to **TOTAL** GR. Recommend TOTAL, pending design confirmation; don't invent an output scale.
3. The original panel **bakes in every knob, scale and illuminated control**. The source-core textures deliberately exclude numeric labels, but are **not** finished animated rotors; rebuild each brass/black knob silhouette and pointer in deterministic code. The mask shows where the old raster controls must be covered by rebuilt faceplate regions. Auto-inpainting large holes was rejected after QA and is not shipped.
4. The full-size source is still only 1536x768, so quality relies on code-rendered typography, anti-aliased vector ticks, proportional layout and real JUCE controls rather than raster upscaling.

## Module assembly instructions
- Add `ShadowPanel` as an independently resizeable layout group to the SteamMaster virtual rack canvas.
- Render the decorative original art only on **noninteractive** areas; finish an accurate static panel before layering active controls so there are never duplicate knobs.
- Show one rotated `ShadowKnob` per APVTS float/choice, draw scales and numbers in JUCE from canonical `choices`/ranges, and use `SliderAttachment`/equivalent value attachments to ensure host automation and session recall.
- Toggle 3 IN controls with `ButtonAttachment`. Meter selector is GUI state, not a DSP parameter.
- A GUI timer reads a **read-only audio-thread-to-message-thread telemetry bridge** for gain reduction; do not call painting from `processBlock`.
- Preserve DSP implementation and current branch. No GitHub writes performed by this handoff.

## Included files
- `01_reference`: locked original panel
- `02_rotary_controls`: 10 clean-cropped RGBA core texture references (not finished rotary sprites)
- `03_buttons_lamps`: button ON crops and OFF prototypes; lamp ON crops and OFF prototypes
- `04_meter`: original VU and a vector needle preview. The source needle must still be removed or the dial reconstructed cleanly
- `05_masks`: removal mask only. No unapproved damaged background included
- `06_qa`: full marked up controls + review image
- `07_specs`: all UI centers in px + param IDs/values

## Next action
**Proceed to Exciter source image upload.** Full Shadow reconstruction/polish and JUCE integration remain for the later dedicated UI implementation stage; do not confuse a working source extraction with finished render assets.
