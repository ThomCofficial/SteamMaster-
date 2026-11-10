STEAMMASTER — BAXANDALL EQ / TWIN TIDES ASSET PACK

Included:
- Latest NO-NUMERIC-VALUES faceplate (unaltered original and rack-strip crop)
- 7 individual brushed-silver knob sprites; use one per parameter on MID and SIDE (14 physical knob instances)
- 7 separate circular value-ring concept art sprites; reuse on both channels
- 12 individual routing, link, IN, BYPASS button-state sprites
- All four original art sheets to verify against
- The agreed control-to-parameter mapping as JSON

IMPORTANT IMPLEMENTATION NOTES:
1. The supplied new faceplate no longer contains printed numeric values, but it STILL contains visual button artwork in the routing and MID/SIDE areas. Overlaying button PNGs may duplicate controls unless these background button regions are cleaned first.
2. The values on generated value-ring concept art have NOT been validated against DSP or exact parameter lists and some are demonstrably incorrect (e.g. the HPF image includes 45 rather than the specified 36 and 43/54). Render numerical labels and detents deterministically in JUCE instead of treating these as production-ready scales.
3. The left and right channels' knob graphics are reused; matching 2x7 alignment must be set in code.
4. The background is an image-generation artwork, not a source-code asset specification; visually verify masking, transparency, scale and alignment before using in the final plugin.
5. DSP was not modified; this ZIP does not include DSP or VST3 binaries.
