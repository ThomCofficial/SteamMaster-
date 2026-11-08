# SteamMaster DSP v1.4 — tape magnetic memory and repro calibration

**DSP-only milestone: zero UI changes; no VST3 build.** The Tape unit now implements a purpose-built, independently authored magnetic-memory / slew / bias nonlinearity with three individually voiced Studio, Vintage and Modern personalities. Playback head-bump and high-frequency losses depend on speed (7.5 / 15 / 30 IPS), bias affects saturation and HF loss, and model-derived 1 kHz makeup calibration maintains similar low-level gain between modes. Existing drive, bias, HF Trim, mix, IN, speed and oversampling controls are all preserved; no hiss, wow/flutter or additional knobs were introduced. All 11 DSP suites include new Tape characterization and safety tests.

**Not a hardware clone:** the tape model is *hardware-informed*, not component-for-component, and there are no measured tape-deck transfer functions or external blind listening validations. See [Tape v1.4 research / test audit](docs/V14_TAPE_MODEL_AUDIT.md) and [model-generated response CSV](docs/V14_TAPE_MODELED_RESPONSE.csv). **No third-party DSP was copied.**

---

# SteamMaster DSP v1.3 — Dangerous BAX EQ response refinement

**DSP-only engineering checkpoint. No VST3 binary or GUI changes.**

v1.3 replaces the generic Q=0.707 Bax EQ shelf biquads with independently authored **first-order analog-prototype Baxandall shelves** using symmetric pole/zero placement, 12 ms smoothed gain/frequency automation, and sample-exact unity gain. The approved low/high selector positions, per-row Mid/Side or Left/Right processing, 12 dB/oct HPF/LPF and bypass/output controls are preserved. Added a 96 kHz modeled-response CSV, a response exporter, and tests at 44.1/48/96/192 kHz.

**Not measurement-matched:** faceplate-to-corner mapping is based on the manufacturer's description of broad shoulders, not privately held hardware traces. The hardware's very low published distortion supports remaining clean rather than adding arbitrary saturation. Ultrasonic cutoff modeling above Nyquist and real hardware listening comparisons remain open. Details and source URLs: [Bax v1.3 research and tests](docs/V13_BAX_HARDWARE_AUDIT.md).

---

# SteamMaster DSP v1.0 — reference-informed hardware voicing

This repository contains standalone C++ DSP engines and tests for the nine-module SteamMaster mastering chain. **No VST3 or GUI yet; no UI changes in v1.0.**

**v1.0 highlights:** published Manley progressive COMP/LIMIT gain-reduction characteristics implemented as non-allocating prepared LUTs; SSL 2:1 knee widened based on SSL documentation (numeric width remains a hypothesis); Shadow Hills discrete DUAL now independently implements fast 80% and slow 20% recovery components; EQP-1A-inspired response uses genuine low-frequency shelving attenuation, manufacturer maximum gains and coupled bass-dip behavior. All six original DSP suites plus independent hardware-reference tests run via CTest.

**Caution:** these are manufacturer-data-anchored models, **not proven measurement-matched clones**. Physical-unit transfer, harmonic and dynamic response datasets are still missing. The GPLv3 EQP-WDF-1A academic implementation is referenced, not copied. See `docs/V10_HARDWARE_RESEARCH.md` and `docs/V10_MODELED_CURVES.csv` for exact provenance and modeled trace data.

# SteamMaster DSP v0.7 — analog voicing experiment

DSP-only update. See `docs/V07_ANALOG_AUDIT.md` for limitations.

# SteamMaster DSP Foundation — engineering checkpoint 0.5.0

**DSP ONLY. No new GUI work, graphics, JUCE/VST3 wrapper or DAW-ready plugin has been added.**

v0.5 adds a second 8x FIR post-limiter true-peak correction stage with fixed additional lookahead, and differentiates Studio/Vintage/Modern Tape models using measured harmonic-fingerprint regression tests. The deterministic reconstructed 8x and 16x limiter stress suites now enforce a **+0.05 dB tolerance**, rather than permitting the earlier +0.75 dB overshoot. An external FFmpeg true-peak check of a 3-second adversarial stereo probe measured **−1.1 dBTP** for a −1 dBTP setting. **These passes do not establish formal BS.1770 compliance or guarantee arbitrary-signal ceilings.**

**Current full-rack latency:** `280 + ceil(0.005*fs) + ceil(0.002*fs)` samples (590/616/898/952/1624 at 44.1/48/88.2/96/192 kHz). No VST3 host latency reporting yet.

The post-limiter stage costs additional CPU and latency and the primary limiter still calls an emergency sample clipper in rare harsh stress cases. Accurate final gain-reduction reporting, physical analog-model calibration, subjective listening, additional independent true-peak validation, VST3 and Cubase tests remain open.

See [`docs/V05_TEST_REPORT.md`](docs/V05_TEST_REPORT.md) for test evidence and reproduction steps.

---

## Previous engineering checkpoints (historical)

# SteamMaster DSP Foundation — engineering checkpoint 0.4.0

**DSP-only development checkpoint; NOT a finished VST3 or mastering-ready product. The first six approved GUI designs remain untouched.**

**v0.4 changes:** an 8x FIR limiter detection/metering path, a lookahead-aware gain release hold, safety-clip event telemetry, and independent spectral/true-peak stress tests. The measured maximum intersample overshoot in the new stress set is still ~0.63 dB above the chosen ceiling. **The limiter is not yet compliant with a guaranteed true-peak ceiling.** See `docs/V04_TEST_REPORT.md`.

# SteamMaster — DSP Foundation v0.3.0

**Nine-module DSP engineering checkpoint, NOT a finished VST3 or certified mastering suite.**

This is a portable C++17 library and standalone regression harness. **No plugin binary, GUI, JUCE adapter, release assets or Cubase-host tests are included.** The first six visually approved module designs have not been modified.

## What's implemented

1. **Ocean:** stereo-linked variable-mu-inspired compressor and oversampled gentle coloration.
2. **Furnace:** stereo-linked VCA bus compression with stepped attack/release and AUTO.
3. **Parametric EQ:** four continuous TPT bands and 1/4 shelf/peak topology.
4. **Pultec:** broad boost/attenuation controls with experimental EQ-curve mapping.
5. **Shadow:** independent serial optical/discrete compressors with stage telemetry.
6. **Exciter:** dual-band Tube/Tape/Transformer behavior, selective FIR oversampling.
7. **Baxandall (new):** all documented discrete HPF, shelf and LPF positions, stepped 0.5 dB shelf gains, independent MID/SIDE bypass, L/R vs M/S processing, smooth routing crossfade. LINK is a **future controller-layer coordination behavior**, not an audio effect.
8. **Tape (new):** Studio/Vintage/Modern nonlinear transfer functions with per-quality history, speed-dependent head-bump voicings, drive, bias, HF trim, stereo-linked transient compression and 2x/4x/8x nonlinear oversampling. **Speed and saturation response calibrations are provisional.**
9. **Master Limiter (new, experimental):** Clean/Punch/Modern envelope variants, adjustable threshold, ceiling, release/AUTO, lookahead, 0–100% stereo link, true-peak detector switch, bypass and gain-reduction/peak telemetry. Sample-peak output is constrained while active; **an absolute ITU/EBU true-peak ceiling is not certified**. Final limiter panel and voicings remain unapproved.

Global core: input/output gain staging (output before limiter), complementary high-side widening, low-side suppression (Mono Maker), conservative fully-linked limiter behavior while Mono Maker is enabled, mono-friendly routing, global IN bypass with fixed latency, NaN/Inf containment and emergency ±4 sample guards.

## Audio architecture and latency

`Global Input → first six → Baxandall → Tape → Global Stereo → Global Output → Limiter → Metering`.

- FIR oversampling around nonlinear Ocean, Exciter and Tape only. No pointless global EQ oversampling.
- Known DSP latency is `210 + limiterLatency` host samples; `limiterLatency = 30 + ceil(0.005 × sampleRate)` samples, constant across quality, limiter lookahead and true-peak modes **at a given sample rate**. At 44.1/48/88.2/96/192 kHz: **461 / 480 / 681 / 720 / 1200** samples.
- These latencies are **not yet reported to Cubase**, because the VST3 wrapper has not been developed.
- Setters operate on the audio thread at block boundaries; host/UI parameter transfer has not been implemented.

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

CI includes Windows/Linux Debug/Release and Linux AddressSanitizer/UndefinedBehaviorSanitizer. `tests/TestMastering.cpp` covers all five requested sample rates, bypass, routing, stereo, nonlinear switching, gain reduction, nonfinite inputs, and fixed latency.

## IMPORTANT—remaining work

- The limiter still needs **independent reconstructed true-peak certification**, overshoot/ISP stress sweeps, audible transient calibration, proper ITU-R BS.1770 / EBU R128 metering, and validated fast ceiling automation. `measuredOutputTruePeak` is provisional telemetry.
- Global stereo processing uses **frequency-selective attenuation, not a brick-wall mono cutoff**; mono fold-down below 20–300 Hz is progressive. The design prevents widening of low frequencies but cannot assert mathematical zero side energy below a cutoff.
- Tape emulation is an **engineering voicing**, not a measured calibration against a particular tape deck. No wow/flutter or noise is introduced.
- Baxandall's L/R interpretation (the two rows become Left/Right) is an implementation assumption pending final approval. LPF positions above Nyquist are treated as effectively OUT, without relabeling controls.
- Pultec and Shadow visual-control mapping discrepancies remain **unresolved**. No GUI mapping has been invented or locked.
- A/B, presets, state serialization, LUFS/correlation meters, UI binding and Windows/Cubase lifecycle testing remain outstanding.
- The emergency guard is a fault-containment measure, **not guaranteed hearing protection** or an output true-peak ceiling when the limiter is bypassed.

Review `docs/ARCHITECTURE.md`, `docs/CONTROL_AUDIT.md`, `docs/V03_TEST_REPORT.md` before VST3 integration.

## v0.6 metering correction

Limiter `gainReductionDb()` now reports the effective gain attenuation from both the main and post-correction stages, including smooth bypass/post-stage crossfades. It excludes the user threshold's intentional input pre-gain. A new metering test checks finite and stereo-linked readings, active reduction, bypass recovery and reset at 44.1/48/96/192 kHz. No GUI work or parameter additions. This is DSP telemetry only; peak ceiling certification and host integration remain outstanding.

### v1.1 — independently authored Pultec passive-network model

The Pultec low section no longer combines generic digital shelving/peaking
filters. It now solves an original **reduced passive RC circuit** with
trapezoidal capacitor current memories and a two-node nodal matrix, followed by
an **RLC-derived high-boost resonator** and an **RC-derived high-cut shelf**.
The published switch capacitor/inductor values informed the modeling; this is
**not** a clone of the complete EQP-1A circuit or hardware-measurement matched.
Read [`docs/V11_PULTEC_CIRCUIT_AUDIT.md`](docs/V11_PULTEC_CIRCUIT_AUDIT.md).
No GPL-3.0 source from `ABSounds/EQP-WDF-1A` was used or copied. The
parametric EQ and every approved GUI component remain unchanged.
