THOMC STEAMMASTER — PULTEC EQ ART ASSETS
=====================================
Included:
- Faceplate: new faceplate art based on the supplied Pultec reference
- Dials: one sheet containing 10 labeled circular value overlays
- Controls: knobs and IN button sprite sheet
- Decor: decorative nautical ornaments sheet
- Reference: original Pultec screenshot supplied by the user

IMPORTANT — VISUAL REVIEW NEEDED BEFORE JUCE IMPLEMENTATION
The supplied original reference has NINE visible rotary positions:
Low Frequency, Low Boost, Low Atten, High Boost Frequency, High Boost,
High Atten Frequency, High Atten, Output, and Mix.
The intended DSP parameter inventory has TEN rotary controls because it also
includes Bandwidth (0–1 / Narrow to Broad), between High Atten and Output.
The generated 10-dial value sheet includes Bandwidth, but the generated
faceplate is missing a separate Bandwidth position. Do not lock this faceplate
for implementation until that is corrected and coordinates are verified.
The images are sprite SHEETS, not individual crops; transparency, scale,
legibility and mapping should be verified before use in JUCE.

Expected Pultec controls:
1 Low Frequency: 30, 60, 100 Hz
2 Low Boost: 0–10
3 Low Attenuation: 0–10
4 High Boost Frequency: 8, 12, 16 kHz
5 High Boost: 0–10
6 High Attenuation Frequency: 5, 10, 20 kHz
7 High Attenuation: 0–10
8 Bandwidth: 0–1 (Narrow–Broad)
9 Output: -12 to +12 dB
10 Mix: 0–100%
+ illuminated IN button

No DSP code has been changed.
