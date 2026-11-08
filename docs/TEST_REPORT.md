# DSP test report — engineering checkpoint 0.2.0

Verified locally on Linux with GCC 14, CMake, Ninja and CTest (not Cubase or Windows):

- Release/default C++17 static library builds without compiler warnings after cleanup.
- `SteamMasterDSPTests`: EQ neutral and ±6 dB bell response, ±5 dB shelves; three compression engines gain-reduction output; Shadow stereo-link and independent bypass; Pultec and Exciter nonzero processing; module bypass click-rate test; full chain for 44.1/48/88.2/96/192 kHz; silence, full-scale/phase-inverted patterns, NaN/Inf protection; rapid parameter automation; benchmark.
- `AddressSanitizer + UndefinedBehaviorSanitizer` build and tests pass under the local environment.
- Approximately 400k assertions across tests. **These are basic software and DSP smoke/stress assertions, not exhaustive validation.**
- The microbenchmark reports a real-time factor for this container only. It is not a Windows or Cubase CPU guarantee.


## v0.2 local Linux measurements

- Both executables built and passed on Linux (C++17 Release): `SteamMasterDSPTests`
  and `SteamMasterOversamplingTests`.
- 81-tap FIR **OFF/2x/4x/8x** linear-path impulse peaks align at precisely
  70 samples; net DC gain is 1 (within measurement tolerances).
- In a **9 kHz, 0.75-amplitude tanh clip at 48 kHz**, the 21 kHz third-harmonic
  alias fell from approximately **0.2826 (OFF)** to **0.00234 (8x)**, or
  **41.6 dB of reduction**. This is one probe only, not a blanket alias guarantee.
- Repeated quality changes every 133 samples under constant drive were finite
  and did not produce a measured step spike; 48 kHz/5 sample-rate matrices,
  exact 140-sample rack bypass delay, stereo balance, and rapid control
  automation tests passed.
- Linux AddressSanitizer + UndefinedBehaviorSanitizer initially passed before
  final documentation. Final CI must also pass before merging.
- Machine-specific, synthetic 0.2-second stereo render of all six engines
  at 48 kHz with Ocean compression and Exciter drive 7: approx. **0.064 RTF
  (OFF), 0.088 (2x), 0.112 (4x), 0.225 (8x)**. At 192 kHz, 8x measured
  about **0.80 RTF** on this container. These numbers are **not** a Cubase
  CPU qualification; CPU performance is a remaining release risk.

NOT tested:

- Real-time memory-allocation instrumentation, audio-driver faults, zero-buffer calls or VST3 host lifecycle.
- True-peak output compliance (no limiter exists yet), LUFS measurement, intersample overs, DC drift over hours, long-tail denormal CPU load.
- Full alias-spectrum/high-drive reference qualification, listening trials, and stereo bass preservation under limiter activity.
- Objective compression knee/attack/release conformance vs measured hardware references.
- DAW save/reload, arbitrary parameter state serialization, GUI control mapping, host block sizes and sample-rate reinitialization in Cubase.

**Exit criteria for product readiness:** must add VST3/Windows CI and a Windows Cubase validation matrix; host lifecycle regression; comprehensive audio measurements and offline golden WAVs; oversampling and latency; gain ceiling; finalized control calibrations; listening assessment. A successful CTest run is necessary but insufficient.
