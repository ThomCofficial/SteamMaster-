# SteamMaster Baxandall — complete module preparation, v1

**Approved image:** `01_reference/baxandall_full_source.png` (1536 × 1087). No aesthetic alterations or DSP changes.
**Code inspected:** `ThomCofficial/SteamMaster-` / `vst3/v1.7-performance-character`: `plugin/PluginProcessor.cpp`, `include/steammaster/Mastering.hpp`, `src/Mastering.cpp`.

## QA result
- Two rows **MID** and **SIDE**, 7 knobs each = **14 knobs**; each row has a BYPASS lamp. Header has **M/S routing** and **LINK** buttons.
- All **18 APVTS parameters** have matching visible controls (14 knobs + 2 row IN + routing + link). Source labels contain no obvious missing numeric detents compared with the DSP.
- **HPF:** OUT/12/18/24/30/36/43/54 Hz.
- **Low shelf:** 74/84/98/116/131/166/230/361 Hz; gain −5..+5 dB.
- **High shelf:** 1.6/1.8/2.1/2.5/3.4/4.8/7.1/18 kHz; gain −5..+5 dB. IMPORTANT: **the printed arc is reversed relative to the ascending APVTS choice list**. Use value-to-angle lookup rather than index-to-angle mapping.
- **LPF:** OUT/70/28/18/12.6/11.1/9/7.5 kHz.
- **OUTPUT:** −4..+4 dB. DSP rounds shelf gains to 0.5 dB increments.

## Implementation details to catch early
1. **M/S vs L/R mode:** The current artwork permanently says MID/SIDE, but the DSP supports Left/Right routing. Make the two row plaques and M/S button dynamic code-rendered text/state, so L/R reads LEFT/RIGHT. No DSP change.
2. **LINK:** `stereo_link` exists as a boolean APVTS parameter, but `BaxandallParameters.link` is a UI/controller feature without DSP side effects. Controller-side paired updates and automation behavior require implementation. Do not claim LINK is working merely because the button lights.
3. **BYPASS lamps:** Existing `bax_mid_in` / `bax_side_in` booleans are the source of truth; verify lamp semantics visually before wiring lit=ON or lit=BYPASS. No new DSP parameters.
4. **Actual rotors are baked into the image.** The 14 transparent cutouts are **reference crops**, NOT finished isolation: perimeter may include pixels from the original cream plate. Do **not** stack the cutout on top of the original panel in the final JUCE UI, which would duplicate baked control imagery. Reconstruct an unoccupied faceplate once, then reuse vector/sprite knobs and exact JUCE text/scales.
5. Preserve ornate brass border, teal ornamentation and glass lamps; never use AI to resize/reassemble the panel. Scale one proportional design coordinate system with code.

## Contents
- Full untouched source, plus trimmed source reference.
- Fourteen transparent rotor reference crops in `02_controls/knobs`.
- Eight button/plaque/lamp reference crops in `02_controls/plaques_buttons_lamps`.
- Whole-unit interactive-region mask for later targeted cleanup.
- Numbered control preview and fourteen-rotor contact sheet.
- Complete 18-ID DSP map, exact range/choice values, approx centers, state notes.

## Status
**Asset audit and layout handoff complete; clean editable faceplate and production-ready rotor isolation still need finishing during JUCE implementation.** No source code, GitHub branch, DSP, or Cubase changes were made.
