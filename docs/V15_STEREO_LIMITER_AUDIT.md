# SteamMaster v1.5 — Stereo Image and Master Limiter DSP audit

**DSP-only change.** Existing Mono Maker (OFF/20–300 Hz), Stereo Width (0–200%; 100% neutral), Width Frequency (800 Hz–5 kHz), Limiter Clean/Punch/Modern, threshold/ceiling/release/lookahead/link/TP/bypass controls are unchanged. No UI, artwork, JUCE, or VST3 code is included.

## Stereo Image — actual implementation

- M/S encoding is mathematically exact. The Mid channel is untouched throughout. The output sums to the original L+R waveform under any stereo settings, including automation.
- Mono Maker is a smoothly engaged 12 dB/octave high-pass on the **side** channel, at the existing adjustable cutoff, before frequency-dependent widening. It attenuates low side energy progressively; it is **not** a brick-wall operation that guarantees zero side energy at every frequency below cutoff.
- Stereo widening uses a **trapezoidal/TPT first-order complementary crossover**. Low side and upper side derive from the same sample/state as `upper = side - low`; then `processedSide = side + (width - 1) * upper`. This replaces the previous two-pole low-pass subtraction, whose complex response could cause crossover-dependent image exaggeration. Low side is substantially unchanged, and widening occurs mainly in the higher band. The crossover coefficient is smoothed over 18 ms, and width over 12 ms. The crossover has no extra latency.
- The Mono Maker filtering occurs before the widener so low side removed by Mono Maker cannot be reintroduced by the widening stage. The mono side processing filter is kept live during bypass transitions to avoid reset transients.
- Added read-only 250 ms smoothed stereo correlation telemetry: `GlobalStereo::correlation()`, bounded to [-1,1]. This is **not** a new GUI control and does not feed back into processing.
- Width at 100% and Mono Maker OFF is sample-exact transparent, even while the crossover filter is running.

**Measured DSP only:** at 44.1/48/96/192 kHz, Width 200%, 800 Hz crossover, 40 Hz side amplitude is 1.0025× versus neutral, while 4 kHz is approximately 1.962×. With Mono Maker 200 Hz and Width 200%, the 40 Hz side component is substantially suppressed (new regression requires <0.065× baseline). This is not a certified listening preference or a hardware-emulation measurement.

## Master Limiter — actual implementation

- Replaced the previous AUTO release setting based only on instantaneous input peak amplitude with a two-time-constant model driven by **requested gain reduction**. Fast demand history uses 8 ms rise / 80 ms fall; sustained demand uses 60 ms rise / 280 ms fall. Their ratio controls release between roughly 60 and 610 ms before personality scaling.
- `Clean`: conservative recovery; `Punch`: faster transient recovery; `Modern`: fast transient recovery that settles more slowly on sustained limiting. Modes therefore have measurably different gain envelopes, not just renamed switch states. The existing *fixed* release parameter still works; mode selection influences time as previously, without adding controls.
- Maintains the existing lookahead gain hold, 8× FIR true-peak detection, 8× independent output correction and safety telemetry. No new limiter latency.
- Added `MasterLimiter::effectiveReleaseMs()` for engineering diagnostics only, not user control.
- The gain reduction meter now reads the **post-delay-aligned effective primary gain** from a fixed-size ring alongside its corresponding audio sample, before adding the actual post-stage correction. Previously the meter could be temporally offset under fast automation because it used the primary gain of the current incoming sample rather than the delayed output sample.
- Four envelope coefficients are precomputed at `prepare()`; adaptive release still requires a small number of exponentials per sample. No memory allocations or locks in audio processing.

**Model-generated measurements at 48 kHz:** after a ~6 ms burst, adaptive effective release was approximately 86 / 60 / 63 ms for Clean / Punch / Modern. After sustained limiting (~196 ms), it was approximately 587 / 411 / 637 ms, respectively. An intermittent tone-burst regression measured mean absolute waveform differences of ~0.0186 between Clean/Punch and ~0.0017 between Clean/Modern, with level matched settings; they are not perceptual listening scores.

**Peak stress:** a 48/96 kHz test with independent 8× reconstruction at -1 dBTP ceiling, 0.1 ms lookahead and all three modes showed approximately -1.15 dBTP output in the tested sample sets. Other preexisting tests perform independent 8× and 16× reconstruction across a larger set of rates and ceiling combinations. Safety clip counters are asserted to remain nonpathological. **This is not formal ITU-R BS.1770 true-peak certification or a guarantee for all arbitrary input/automation combinations.** Full in-host validation, CPU benchmarking, EBU R128 integrated LUFS, calibrated listening and harsh-signal tests remain necessary.

## Tests and build

The new suite `SteamMasterStereoLimiterTests` validates sample-exact neutral stereo behavior, crossover isolation, unchanged mid sum, anti-reintroduction of LF side, mono correlation, harsh automation, program-dependent release, mode sonic differences, linked GR behavior, and all-mode true peak stress. Pre-existing 11 DSP suites remain in place.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Release: 12/12 suites passed locally. AddressSanitizer + UndefinedBehaviorSanitizer (`RelWithDebInfo`, `-O1 -g -fsanitize=address,undefined`): 12/12 suites passed locally. GitHub CI checks Windows/Linux Debug/Release plus sanitizers.

## Remaining limitations

No JUCE / VST3, no Cubase host lifecycle, and no GUI integration. Correlation is monitor-only, not an automatic phase protector. Mono Maker is a progressive filter; Width 0% collapses the high band but intentionally leaves the unaffected bass side until Mono Maker is engaged. Limiter mode voicings need level-matched listening, high-LUFS music tests and full independent TP testing for arbitrary audio. Safety hard clipping remains a documented, counted last resort under pathological conditions. No exact hardware emulation is claimed for these digital processing units.
