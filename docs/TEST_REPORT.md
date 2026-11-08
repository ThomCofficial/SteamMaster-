# DSP test report — engineering checkpoint 0.1.0

Verified locally on Linux with GCC 14, CMake, Ninja and CTest (not Cubase or Windows):

- Release/default C++17 static library builds without compiler warnings after cleanup.
- `SteamMasterDSPTests`: EQ neutral and ±6 dB bell response, ±5 dB shelves; three compression engines gain-reduction output; Shadow stereo-link and independent bypass; Pultec and Exciter nonzero processing; module bypass click-rate test; full chain for 44.1/48/88.2/96/192 kHz; silence, full-scale/phase-inverted patterns, NaN/Inf protection; rapid parameter automation; benchmark.
- `AddressSanitizer + UndefinedBehaviorSanitizer` build and tests pass under the local environment.
- Approximately 400k assertions across tests. **These are basic software and DSP smoke/stress assertions, not exhaustive validation.**
- The microbenchmark reports a real-time factor for this container only. It is not a Windows or Cubase CPU guarantee.

NOT tested:

- Real-time memory-allocation instrumentation, audio-driver faults, zero-buffer calls or VST3 host lifecycle.
- True-peak output compliance (no limiter exists yet), LUFS measurement, intersample overs, DC drift over hours, long-tail denormal CPU load.
- High-rate oversampling, high-drive alias rejection, mixed-rate crossover phase alignment, stereo bass preservation under limiter activity.
- Objective compression knee/attack/release conformance vs measured hardware references.
- DAW save/reload, arbitrary parameter state serialization, GUI control mapping, buffer sizes and sample-rate reinitialization in Cubase.

**Exit criteria for product readiness:** must add VST3/Windows CI and a Windows Cubase validation matrix; host lifecycle regression; comprehensive audio measurements and offline golden WAVs; oversampling and latency; gain ceiling; finalized control calibrations; listening assessment. A successful CTest run is necessary but insufficient.
