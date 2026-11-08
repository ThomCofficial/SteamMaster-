# SteamMaster DSP Foundation — engineering checkpoint 0.1.0

**This is a portable C++17 DSP library and test harness, NOT a finished VST3, and NOT a mastering-ready audio product.** No GUI assets, graphical controls, Cubase binaries, JUCE adapters or unapproved interfaces are included. The six approved module designs remain untouched.

## What is implemented

- Ocean: stereo-linked soft-knee program-dependent compressor, user-selectable ratios/recovery, attack, sidechain HPF, mix, bypass and gentle nonlinear color.
- Furnace: stereo-linked VCA compression, exact discrete attack/release/ratio selections, automatic release, SC HPF, mix, bypass.
- Parametric EQ: four topology-preserving TPT EQs with peak/shelf constraints, gain, Q, frequency, output, mix, bypass.
- Pultec: independent interactive low boost/attenuation and high boost/attenuation paths, variable high-boost bandwidth, output/mix/bypass. **Response-curve calibration is experimental and awaiting approval.**
- Shadow: serial optical/discrete stereo-linked compression, independent IN stages, global IN, SC HPF, mix/output, per-stage gain-reduction telemetry. **1–24-style knob-to-dB mapping is unresolved.**
- Exciter: distinct Tube/Tape/Transformer nonlinear paths, selectable low/high frequency emphasis, band-dependent drives, crossover, mode crossfade, output/mix/bypass. **Anti-aliasing quality is NOT yet production-grade; oversampling remains to be implemented.**
- Shared: fixed-memory sample processing, parameter smoothing, module bypass crossfades, nonfinite containment, emergency >+12 dBFS sample bounding (not a true-peak limiter), denormal suppression and telemetry.

## Tests

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Run the executable directly for subtest detail and a machine-specific microbenchmark.

## Not yet done, by design

- JUCE/VST3 and Cubase 13 integration or host lifecycle testing.
- Full calibrated analog models and listening/AB measurements against hardware.
- Oversampling 2x/4x/8x, phase/latency compensation, alias rejection, or safe switching among oversampling modes.
- Baxandall, Tape, limiter, global stereo width and Mono Maker.
- True-peak ceiling, LUFS meters, clipping indicator, A/B state or preset serialization.
- Comprehensive automation tests, DAW sessions, windows binary tests and continuous CPU qualification.

The current module code is an **early real DSP implementation, not placeholder silence or parameter pass-through**. However, passing unit tests alone does not establish release-grade audio quality, real-time stability in Cubase or safe mastering ceiling enforcement.

Read `docs/CONTROL_AUDIT.md`, `docs/ARCHITECTURE.md`, and `docs/TEST_REPORT.md` before adding host bindings.
