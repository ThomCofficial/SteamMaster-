# SteamMaster v1.2 — Exciter: three independent hardware-informed engines

DSP-only revision; the approved cream/brass single-row Exciter UI is **untouched**. No new parameters, GUI assets, host integration, or third-party processing code.

## Hardware references and evidence

1. **Tube — Black Box Analog Design HG-2**. Manufacturer manual rev 3.6: https://blackboxanalog.com/wp-content/uploads/2021/05/HG-2-manual-REV-3.6.pdf . The documented input transformer -> Pentode -> Triode -> output transformer main path has a parallel saturation path taken from input, mixed back before the Pentode; Pentode/Triode controls drive those stages. **Our implementation follows this logical order**, with two biased nonlinear valve-style transfers and a separate parallel saturation blend. We have **not** modeled the physical transformer or precise 12AT7/triode/pentode circuits. No tube measurements were imported.
2. **Tape — Crane Song HEDD Quantum**. HEDD Quantum manual: https://manuzoid.com/manuals/de5Q8-Crane%20Song%20HEDD%20Quantum%20User%20manual . The published Tape description attributes odd-order harmonics (particularly 3rd and 5th), progressively stronger distortion, high-frequency compression, and record/repro EQ effects. Our originally authored model uses a symmetric odd-order soft-transfer function, drive-dependent slew damping and a 9 ms magnetic-memory proxy. This is **not** Crane Song's proprietary model. No HEDD measurements or code have been copied.
3. **Transformer — Rupert Neve Designs Silk**. Shelford Channel official manual: https://rupertneve.com/s/775-000275035-Manual_RevB.pdf . At 100 Hz and +20 dBu, the manual reports Silk Red at maximum Texture **1.66% THD+N, predominantly 2nd harmonic**, and Silk Blue at maximum Texture **0.941%**, predominantly 2nd. We implement a magnetization-memory proxy and a bias-asymmetric, second-harmonic-dominant transfer with restrained drive. Because only one Transformer mode exists in the locked interface, this is a **fixed Red-leaning voicing**, not a Red/Blue control. No transformer equations, winding loss measurements, core parameters, or equivalent magnetic circuit are available.

## Mapping into the approved interface

- Existing mode buttons remain `Tube`, `Tape`, `Transformer`.
- Low Drive / Frequency and High Drive / Frequency independently control two selected parallel excitation bands. Crossover and Output/Mix/IN behavior is unchanged.
- The zero-drive transfer is exactly a *fixed-latency unity path* at any oversampling setting, without noise addition or extra hiss. The voice curves normalize slope around silence.
- Mode changes use the existing 15 ms coefficient crossfades; oversampling changes use the existing 12 ms FIR crossfade/warmup. Tape and transformer states are independent per band **and per sample-rate quality path**. The nonlinear circuits run only in the two excitation bands.
- At max Drive, harmonic soft clipping can lower perceived peaks, but this is **not** a final output limiter or a guaranteed true-peak ceiling. The final global limiter still handles that responsibility.

## Independent measurements from v1.2's FFT/DFT regression harness

- 48 kHz, 125 Hz, 0.60 amplitude, Low Drive 10, High Drive 0, 8x: measured 2nd / 3rd harmonic amplitude (post full Exciter routing): Tube ~0.011064 / 0.041105; Tape ~0 / 0.039835 with 5th ~0.008021; Transformer ~0.009232 / 0.001212. Tape is odd-dominant, transformer second-harmonic dominant; all three differ.
- 48 kHz, 100 Hz, input 0.631 amplitude, Low Drive 10, 8x: **Transformer model measured ~1.686% THD2+3**. If one assumes +24 dBu at digital 0 dBFS, this roughly corresponds to a +20 dBu input level. The numerical proximity to Silk Red's 1.66% specification is a **single-point engineering target**, NOT a validated hardware match. Neve specifies THD+N in an actual analog setup; our number is harmonic THD2+3 in a digital test. The dBu mapping is hypothetical and the figures are not equivalent quantities.
- 48 kHz, 10.5 kHz near Nyquist, High Drive 10: at the 16.5 kHz alias bin, the oversampled 8x path suppresses the 1x folding in all three modes; test asserts >=9 dB suppression (actual considerably more in tested case).
- Low/high drive isolation, mode/quality automation, neutral path, state reset and 9 full library tests pass in Release + ASan/UBSan builds.

## What is explicitly not claimed

These are **original hardware-inspired DSP algorithms**, not exact clones of the HG-2, HEDD or Silk. A genuine hardware-match would require calibrated input/output transfer sweeps, harmonics vs. drive/frequency, phase/group-delay and transient tests, mapped against independent hardware recordings. The original Aphex-style filtered parallel excitation concept is architectural inspiration, not copied implementation. No third-party DSP code was imported.

## Next engineering tasks

1. Compare output captured from physical references at matched calibration and gain staging, adjust topology constants, and report per-level/Hz errors.
2. Real-world ABX listening (with level matching) on mixes and stems; assess whether full-drive coloration is too strong for mastering.
3. CPU profiling for 8x processing and DAW host automation after VST3 integration (not yet implemented).
4. Measure true peak on music content across band controls; avoid use of the Exciter as the safety limiter.
