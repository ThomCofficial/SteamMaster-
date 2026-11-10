# SteamMaster — Pultec DSP-locked UI handoff

**DSP: unchanged. GitHub: untouched.**

The Pultec is now reconciled at the **parameter specification** level. All 11 controls are accounted for, including the dedicated high attenuation frequency selector.

## Corrected three selector scales
- Low Frequency: **30 / 60 / 100 Hz**
- High Boost Frequency: **8 / 12 / 16 kHz**
- High Attenuation Frequency: **5 / 10 / 20 kHz**

Original AI-generated visual inaccuracies (20 Hz, duplicated 10 kHz etc.) are **never to be used as JUCE scales**. All other 8 controls follow the exact existing code ranges in `spec/Pultec_Exact_DSP_Controls.csv`.

## What's inside
- The approved art, untouched, for comparison.
- Working control/faceplate layer prototypes are included and explicitly marked **not production quality**. A failed patch preview was removed instead of shipping another flawed asset.
- A small **compilable C++ value header** for exact code-rendered labels (non-DSP).
- A 1:1 JSON and CSV parameter map.
- Automated assertion results.

## Deferred to JUCE integration
Baked background scales must be erased without damaging decorative details; a professional interactive layer must use clean rotors (the prior matte cutouts still contain background artifacts). No inaccurate revised preview is included. Fix the three selector backgrounds during implementation, then render scales programmatically. JUCE and Cubase have not been tested.

**No new DSP behavior. No branch changed. No need to hold up the next module.**
