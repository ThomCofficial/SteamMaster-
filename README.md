# SteamMaster — DSP Foundation v0.2.0

**Engineering checkpoint, not a finished VST3 or mastering-ready product.** This is a standalone, cross-platform C++17 DSP library with automated tests. The first six approved module GUIs remain **locked and unchanged**; the repository includes **no GUI, JUCE adapter, installer, Cubase binary, limiter or new artwork**.

## What's new in v0.2

- Implemented real **OFF / 2x / 4x / 8x** FIR oversampling around the **Ocean tube-coloration stage** and **both independent Exciter nonlinear bands**. It does **not** oversample the linear EQs or the VCA compression envelope calculations.
- Fixed-latency, linear-phase 81-tap Blackman-Harris halfband FIR up/down filters; 70 samples per participating module. The six-module rack currently reports a fixed **140-sample DSP delay** at every quality setting (Ocean + Exciter). **The future VST3 adapter must report that latency to Cubase; no host PDC is implemented yet.**
- Preconstructed DSP paths with warmup and ~12 ms, latency-aligned crossfades for changing oversampling modes. Rapid requests are queued; no allocation, synchronization or filter reinitialization occurs in sample processing.
- Phase-aligned dry/wet and module-bypass paths; module and rack resets preserve selected oversampling quality and clear relevant detector/FIR states.
- Optimized nonlinear work to process only actively weighted Exciter personalities and precompute transfer constants; preallocated FIR ring buffers with fast indices.
- Shadow optical and discrete gain-reduction getters now reflect stage IN fade/bypass states.
- Added independent anti-aliasing, delay, finite-sample, bypass, sample-rate, rapid-switching and safety regression tests.

## Current module coverage

1. **Ocean**: stereo-linked Vari-Mu-style dynamics, program-dependent recovery, gentle oversampled tube coloration.
2. **Furnace**: stereo-linked VCA compression, stepped timings, AUTO release.
3. **Parametric EQ**: four adjustable bands with stable TPT filters.
4. **Pultec**: simultaneous broad low boost/cut and separate high shaping; coefficient calibration remains experimental.
5. **Shadow**: independent optical + discrete compression stages in series, stage bypasses and telemetry.
6. **Exciter**: Tube/Tape/Transformer transfer functions, low/high harmonic enhancement and local nonlinear oversampling.

The first six modules perform actual DSP, but the analogue models have **not** been calibrated against reference hardware. Preserve all locked GUI designs and unresolved UI mappings. See `docs/CONTROL_AUDIT.md`.

## Build and tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Run `steammaster_tests` and `steammaster_oversampling_tests` directly for measurement output. GitHub Actions covers Windows/Linux Debug/Release and Linux ASan/UBSan.

## Important limitations

- This is **not** a runnable VST3 plugin and has **no Cubase host integration**.
- No Baxandall, Tape module, limiter, true-peak ceiling, global stereo stage, preset/A-B serialization, LUFS/correlation meters or GUI control binding yet.
- FIR oversampling has been validated with impulse alignment, several sample rates and a **single 9 kHz nonlinear alias probe at 48 kHz**; this is **not** a comprehensive production anti-aliasing specification. Final audio quality, 192 kHz CPU budgets and Windows/Cubase lifecycle stability remain unverified.
- `SixModuleRack`'s ±4 sample clamp is emergency fault containment, **not** a safe listening level, dBTP ceiling or substitute for a real limiter.
- Certain control ranges and model response shapes remain provisional DSP-native engineering values, **not** approved GUI mappings.

Read `docs/ARCHITECTURE.md`, `docs/CONTROL_AUDIT.md`, and `docs/TEST_REPORT.md` before adding VST3 bindings.
