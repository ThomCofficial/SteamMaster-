# SteamMaster — Ocean + Furnace asset prep v1

## Scope
Read-only asset preparation from the uploaded Ocean/Furnace source image. No GitHub changes. No DSP changes.

## Source quality
- Combined source image size: 1536 × 864 px
- Working crops:
  - Ocean: 768 × 391 px
  - Furnace: 768 × 391 px

## What is ready
- Complete source crop for each module
- Approximate control coordinate map for JUCE layout
- Visual inventory of controls and decorative regions

## Extraction approach
For **both** modules, treat the panel as:
1. **Static layer**
   - brass frame
   - cream faceplate
   - wave/flame top illustration
   - labels and dividers
   - screw heads and ornaments
2. **Interactive layers**
   - 4 large brass knobs
   - 2 small black knobs
   - SC HPF button
   - IN button
   - AUTO lamp
   - optional bypass/illumination states

## Important mismatch warning
These uploaded module images visually differ from the earlier collage / prior control audit.

### Ocean visual differences
- Threshold scale appears different from earlier notes
- Attack knob visually shows a different progression
- Release labeling includes values not matching the old draft

### Furnace visual differences
- Ratio ring appears to include more positions than the current DSP draft
- Threshold scale appears different from the earlier code-oriented audit
- Attack / release presentation differs from the earlier collage draft

**Conclusion:** use these new module images as the visual authority for artwork, but do not finalize DSP binding until each visible scale is reconciled against the code.

## Immediate next step
Proceed to the next module upload, or refine these two modules into separated faceplate/control assets only if you want deeper extraction before coding.
