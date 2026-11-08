# SteamMaster DSP v0.5 — Post-Limiter True-Peak Correction and Tape Fingerprinting

**Scope:** DSP library, offline tests, build configuration and documentation only. **No GUI assets, GUI integration, controls, VST3 binaries or JUCE adapter changed.**

## Engineering changes

1. Added a second 8x FIR reconstruction detector *after* the existing sample-domain limiter and its last-resort clipping stage. It observes peaks introduced by changing gain envelopes and intersample reconstruction, rather than relying only on the pre-limiter detector.
2. Added a fixed-latency **35-sample FIR delay + 2ms lookahead** correction envelope with short attack, 45ms recovery and a small **0.15 dB** engineering guard margin. This is not a new GUI parameter or proposed control range. True Peak OFF and limiter bypass follow the same latency-aligned path and smoothly fade out this correction.
3. The original limiter's user-selected lookahead, ceiling, threshold, three modes and stereo-link settings remain unchanged. Global output gain still precedes limiting. Stereo linking remains forced on when Mono Maker is active.
4. Revoiced existing Tape transfer functions (no additional modes or controls): Studio preserves the original smooth tanh response; Vintage now has stronger memory/asymmetry and a separate atan drive mapping; Modern has lower nonlinear drive and reduced memory. No assertion of emulating a measured physical deck.
5. Added 16x offline output-reconstruction gates, varied ceiling/lookahead stress cases and Tape harmonic-fingerprint regression tests. The prior **+0.75 dB overshoot allowance has been removed** from the characterization test.

## Latency

Limiter DSP latency is **70 + ceil(0.005 × fs) + ceil(0.002 × fs)** native samples. This is fixed for a given sample rate across True Peak ON/OFF, bypass and lookahead selection.

Full rack DSP latency is **280 + ceil(0.005 × fs) + ceil(0.002 × fs)** samples (44.1/48/88.2/96/192 kHz: **590 / 616 / 898 / 952 / 1624** samples). This is internal only; no host-side latency reporting is available before VST3 integration.

## Measured outcomes

- Strict 8x and 16x **reconstruction** gates for the deterministic stress corpus now check output against the selected ceiling **within +0.05 dB**. The previous ~0.63 dB worst-case overshoot under that corpus no longer occurs.
- Additional near-Nyquist random/burst sweeps exercise −2/−0.5/0 dBTP ceilings, 0.1/5 ms lookahead, all three limiter algorithms, and independent stereo gain control.
- Independently measured a 3-second 48 kHz stereo high-frequency/transient probe via **FFmpeg `ebur128=peak=true`**, reporting **−1.1 dBTP** for a **−1 dBTP** target. Reproduce the raw float file using `steammaster_tp_probe` and the FFmpeg command documented below.
- At high drive, the measured 1 kHz third-harmonic amplitudes on the 48 kHz / 8x test are approximately **0.178 Studio / 0.165 Vintage / 0.084 Modern**. A biased Vintage tape produces more second-harmonic energy than biased Modern in that signal. These are algorithm-fingerprint measurements, not hardware calibration or perceptual tests.
- Original compressor, EQ, oversampling and full-rack test suites remain part of CTest.

## Reproduction

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/steammaster_tp_probe tp-test.f32
ffmpeg -f f32le -ar 48000 -ac 2 -i tp-test.f32 -filter_complex "ebur128=peak=true" -f null -
```

## Limits: still **not mastering-product certification**

- The independent 16x test path uses the same family of halfband FIR filters, although it maintains independent state and additional interpolation. Formal BS.1770 conformance, *independent measurement implementations*, longer adversarial corpora, different sample-rate converters, and true-peak behavior under rapidly changing ceiling and ON/OFF settings remain to be validated.
- The original sample-domain emergency clipping still occurs in a small number of deliberately extreme synthetic transients and may have audible consequences; this is not yet a guaranteed transparent limiter. Gain reduction reporting currently describes the first gain stage and needs to include the post-correction stage for trustworthy final GUI metering.
- The post-correction stage incurs extra latency and CPU work. Profile Windows/Cubase at 192 kHz/8x before release.
- Tape EQ, transient envelope and nonlinear coefficients are provisional musical designs. No hardware-reference transfer curves, blind level-matched listening tests, intermodulation distortion measurements or plugin-in-host DAW tests have been done.
- No controls are added; no existing visually locked module or limiter-proposal parameter range is changed.
