# SteamMaster Tape — complete single-module prep & DSP audit

**Review date:** 10 November 2026  
**Source:** `1000032034.png` (1536 × 864). Trimmed physical panel: 1536 × 469.  
**Inspected branch:** `vst3/v1.7-performance-character` in `ThomCofficial/SteamMaster-` (read-only).  
**No GitHub, DSP, parameter range, audio routing or source file was changed.**

## Verified mappings

| Visible control | Exact JUCE/APVTS ID | Code values | Artwork | Verdict |
|---|---|---|---|---|
| IN | `tape_in` | Bool, default ON | IN | Match |
| TYPE | `tape_type` | Studio / Vintage / Modern; default Studio | STUDIO / VINTAGE / MODERN | Match |
| SPEED | `tape_speed` | 7.5 / 15 / 30 IPS; default 15 | 7.5 / 15 / 30 | Match |
| DRIVE | `tape_drive_db` | −10..+10 dB; default 0 | −10..+10 dB in steps of 2 | Match |
| BIAS | `tape_bias` | −5..+5; default 0 | −5..+5 | Match |
| HF TRIM | `tape_hf_trim_db` | −6..+6 dB; default 0 | −6..+6 dB in steps of 2 | Match |
| MIX | `tape_mix` | 0..100%; default 100 | 0%, 50%, 100% | Match |

**Result: 7/7 live controls have an existing mapping and their visible endpoint ranges agree.** No need for an extra knob or a DSP adjustment. The wood/tape reel decorations are not DSP parameters.

## Visual completeness

- Full brass perimeter, teal ornamental flourishes, wood panel and nameplate are intact.
- Both tape reels present; **rotating reels** will need an independently composited rotor and stationary background/tape path.
- Back-lit IN button present.
- All six rotary controls present, with two type/speed selector knobs and four mastering knobs.
- The central amber tape window is **decorative illumination**, not an audio meter. Avoid fabricating telemetry.
- Scale ticks, numeric values and labels visible; code should re-render precise typography to avoid duplicate print.

## Actual asset readiness

Included: full artwork, transparent-exterior module reference, upper/lower source sections, 7 UI-control cutout references, 2 reel reference cutouts, exact occlusion masks, full-module annotated coordinate review, and machine-readable parameter/position/texture metadata.

**Not claimed finished:** clean control-free faceplate, reel-free wood underlay, tape-path separation, or frame-accurate rotor sprites. The source is a flattened raster, so hidden background pixels cannot be recovered just by cropping. These will be reconstructed conservatively during the JUCE assembly stage, preserving design and proportions. The cutouts are **reference crops**, not clean final animation sprites.

## JUCE implementation notes

- Uniformly scale 1536 × 469 virtual pixels for this module within the vertical, resizable rack; no scrollbars.
- Bind the 7 controls with `AudioProcessorValueTreeState` attachments using the exact IDs listed above.
- Use the native three-position choices for Type and Speed; do not invent intermediate DSP positions.
- Quantize display ticks as drawn without modifying continuous DSP step sizes.
- Keep reel motion animation only on the message thread (or animation clock); never repaint or allocate images on the audio thread.
- Reel spin is decorative: reflect tape speed qualitatively only; do not claim transport RPM or tape length measurement.
- Reconstruct/control-mask the static source background so knobs and moving reels never double-render.

**Next unit:** Stereo Image + Limiter (combined final panel).
