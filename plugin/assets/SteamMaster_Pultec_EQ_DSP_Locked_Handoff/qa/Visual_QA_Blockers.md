# Visual QA — not yet passed

The approved artwork and all 11 UI parameter mappings are preserved.

Known blockers to production assets:
1. Generated reference includes incorrect text baked into three frequency rings.
2. Existing transparent rotor prototypes include baked labels / background pixels.
3. Existing background underlays show incomplete reconstruction of removed knobs.

**Reject** an attempted flat teal oval patch; the preview exposed residual labels and was removed from this package.

Resolution in the JUCE UI build: separate background from controls, reconstruct only the affected faceplate regions to match its patina, and draw DSP scale numbers/ticks accurately as vector text. Keep the approved knob aesthetics. User should not need to approve the underlying DSP calibration again.

This handoff completes the 1:1 parameter and asset inventory but not pixel-perfect visual production. No DSP algorithm or branch modified.
