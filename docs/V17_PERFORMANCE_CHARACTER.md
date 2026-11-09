# SteamMaster v1.7 — Performance & Character (experimental)

**Reference sound:** protected branch [`v1.6-golden-sound`](https://github.com/ThomCofficial/SteamMaster-/tree/v1.6-golden-sound), pinned at commit `0111eaeda0018d5214aaf62f766463afd2b797ec`. Do not merge experimental DSP changes into this reference.

## Changes

- CPU: mathematically equivalent symmetric-coefficient folding in the existing **81-tap linear-phase halfband FIR** for 2×/4×/8× oversampling. No change to filter coefficients, latency, OS selection, tube/tape distortion curves, tape transport, or any other processing stage.
- Furnace: modest ratio-dependent knee adjustments (2:1 from 24 to 22 dB; 4:1 from 6 to 5.5 dB; 10:1 from 2 to 1.8 dB). Small effective threshold offsets (−0.45 dB at 2:1, −0.10 dB at 4:1). No change to attack/release law. These numbers are engineering fits, **not measured hardware values**.
- Exciter Transformer ONLY: add a slower, program-dependent core-remanence state alongside the existing fast flux; subtle frequency-dependent magnetic coupling, smooth asymmetric saturation and a drive-dependent wet fraction. **Tube and Tape transfer algorithms unchanged**.
- All unrelated module voices, parameters and UI remain untouched.

## Evidence and verification

Local Release-mode reference benchmark of one oversampled saturator at 48 kHz:

| Oversampling | v1.6 (ns/sample) | optimized (ns/sample) |
|---|---:|---:|
| 2× | 198 | 184 |
| 4× | 455 | 422 |
| 8× | 930 | 796 |

The isolated 8× oversampling path was ~14% faster on this local environment. **This is not a claim of 14% lower Cubase CPU usage.** Full-plugin load depends on which modules are enabled and the host.

A 48k-sample, stereo, nonlinear-saturation test comparing the previous and optimized resampler showed output null residuals of −309.9 dB (2×), −307.5 dB (4×), −305.9 dB (8×), at double precision. No changes in the reference resampling response were detectable at practical resolution.

Regression gates: existing DSP tests, oversampling latency/alias/transient tests, full-chain stress, Furnace ratio-knee checks and transformer harmonic checks at 1×/2×/4×/8×. Windows VST3 build and JUCE host test must pass independently. Cubase/Plugin Doctor listening and stability checks remain required.

## SSL documentation

SSL says the 2:1 bus-compressor knee is exceptionally soft and lower ratio settings shift the effective threshold downward. Their guidance does **not** give calibrated knee widths in dB. Links:
- https://support.solidstatelogic.com/hc/en-gb/articles/4408132343057-Why-does-the-Bus-Compressor-reduce-gain-when-the-threshold-is-fully-up
- https://www.solidstatelogic.com/assets/uploads/downloads/Duality_Fuse_Operator_s_Manual.pdf

## Rollback

Do not overwrite the golden branch. Compare v1.7 with v1.6 using matched-level A/B and Plugin Doctor. Reinstall the golden build if the v1.7 tonal changes are not preferred.
