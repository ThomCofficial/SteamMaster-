# SteamMaster — Parametric EQ asset prep v1

## Scope
Read-only asset preparation from the uploaded Parametric EQ module image.
No GitHub changes. No DSP changes.

## Source
- Image size: 1536 × 512 px

## Quick visual check
This module looks coherent and complete.

### Confirmed visible structure
- 4 bands: **Band 1 / Band 2 / Band 3 / Band 4**
- Per-band large knobs: **Gain / Freq / Q**
- Band lamps present for all 4 bands
- Band 1: **Shelf + Peak**
- Band 2: **Peak**
- Band 3: **Peak**
- Band 4: **Shelf + Peak**
- Global controls: **IN**, **Output**, **Mix**

## Noted visible scales
- Gain rings appear to run roughly **-12 to +12 dB**
- Q rings appear to show **0.3 / 0.5 / 0.7 / 1 / 1.4 / 2 / 3 / 5**
- Frequency labels appear as:
  - Band 1: 20 / 50 / 80 / 120 / 200 / 300 / 500 Hz
  - Band 2: 100 / 150 / 250 / 400 / 630 / 1k / 2.5k
  - Band 3: 630 / 1k / 1.6k / 2.5k / 4k / 6.3k
  - Band 4: 2.5k / 6.3k / 10k / 16k / 20k
- Output appears **-12 to +12 dB**
- Mix appears **0% to 100%**

## Asset separation plan
Treat the module as:

### Static layer
- outer brass frame
- cream faceplate
- turquoise scroll ornaments
- titles, band plaques, band text, dividers
- printed scales/ticks/labels
- screws and decorative separators

### Interactive layers
- 14 brass rotary knobs total
  - 12 band knobs
  - 1 output knob
  - 1 mix knob
- 6 buttons total
  - Band 1 Shelf
  - Band 1 Peak
  - Band 2 Peak
  - Band 3 Peak
  - Band 4 Shelf
  - Band 4 Peak
- 1 module IN button
- 4 lamps

## Important mapping note
Visually, this module is clear enough to proceed with asset prep.
Final UI-to-DSP binding still needs to be checked against the code branch before implementation, especially:
- exact band type mapping for shelf/peak switches
- exact frequency ranges per band
- whether the visible scales are authoritative or need reconciliation with current DSP ranges

## Files in this package
- parametric_eq_module_source.png
- parametric_eq_control_map.json
- parametric_eq_review_sheet.png
