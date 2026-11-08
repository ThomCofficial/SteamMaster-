# Controls audit — approved text vs visual reference (8 Nov 2026)

**Highest precedence for DSP mapping: explicit written specification, unless the user confirms a correction.** All six artworks are visually LOCKED. Do not redraw, rescale or replace their knob designs.

| Module | Confirmed / now visible | Unresolved or contradictory |
|---|---|---|
| Ocean | Ratios **1.2:1, 1.5:1, 2:1, 3:1, 4:1, 6:1**; Attack **25–70 ms**; Recovery **0.2/0.4/0.6/4/8 s**; HPF appears **OFF/40/80/160/320/500 Hz**; Mix **0–100%**; IN | Threshold shows labeled ticks around **−30..0 dB**; exact continuous min/max and gain-detector calibration need user approval; no Auto recovery. |
| Furnace | Threshold **−15..+15 dB**; Ratio **2/4/10**; Attack **0.1/0.3/1/3/10/30 ms**; Release **0.1/0.3/0.6/1.2 s / AUTO**; SC HPF appears **OFF/40/80/160/320/500 Hz**; Mix and IN | Confirm threshold reference and compressor calibration. |
| Parametric EQ | Four bands, three knobs each (Gain/Freq/Q); ±12 dB Gain; shelf/peak only on first/last band; Output artwork appears **±12 dB**, Mix **0–100%** | Full knob frequency boundaries and Q law/ticks need validation. Do not redesign the 12 knobs. |
| Pultec | Text spec low frequency **30/60/100 Hz**; boost/cut **0..10**; high boost freq **8/12/16 kHz**, width, high attenuation freq **5/10/20 kHz**, amount, output, mix, IN | Artwork also depicts **20 Hz** low and **3/4/5/10 kHz** high positions, **not in written spec**. Artwork has no independent *high attenuation frequency* selector. **No GUI binding for this unresolved control is authorized.** Coefficient calibration of 0–10 amounts is open. |
| Shadow | Optical+Discrete serial sections, independent IN, master IN, 1 VU, meter selector Optical/Discrete/Output; Discrete ratio **1.2/2/3/4/6/Flood20**; attack image appears **0.1/0.3/1/3/10/30 ms**; recovery image appears **0.1/0.2/0.4/0.6/0.8/1.2 s/Dual** | Threshold/gain knob face labels **1..24** require calibration to actual dB units. Meter scale/mode need final numerical definition. No saturation/transformer mode. |
| Exciter | Mode **Tube/Tape/Transformer**, two drives, two frequency selections, Crossover, Output, Mix, IN. Artwork low frequency appears **30/60/100/200/400 Hz**; high frequency **1.5/3/5/8/12 kHz**; crossover **250/400/800/1200/2000/3000 Hz**; Drive 0..10; Output ±12 dB | Confirm which frequency control shapes harmonic emphasis vs crossover topology. No extra Odd/Even, Presence or SC HPF. |

**In code:** unresolved UI-to-DSP scales are represented as **direct, unit-labeled DSP values**, with comments documenting that the UI binding is not approved. Some neutral/default values and a few musically plausible shape calibrations are engineering prototypes; they are NOT settled product specifications.
