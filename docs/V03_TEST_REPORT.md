# SteamMaster DSP v0.3 validation checkpoint

Date: 2026-11-08. These are **local standalone C++ tests**, not Windows/Cubase plugin testing.

## Green local tests

- Release CMake / CTest: `SteamMasterDSPTests`, `SteamMasterOversamplingTests`, `SteamMasterMasteringTests` passed.
- Mastering tests: **1,035,101 assertions** across **44,100 / 48,000 / 88,200 / 96,000 / 192,000 Hz** sample rates.
- The new whole-chain test checks bypass and latency equality, Baxandall M/S-to-L/R control switching, independent band changes, exact neutral routing, Tape type/speed/quality combinations, stereo invariance of the nonlinear model under identical stereo input, Mono Maker low Side attenuation, high-only widening, limiter gain reduction / sample ceiling, rapid module automation, reset, and NaN/Inf containment.
- A local AddressSanitizer/UndefinedBehaviorSanitizer run of **SteamMasterMasteringTests** passed. The original v0.1/v0.2 tests passed Release. Full CI suite will be evaluated on GitHub.

| Sample rate | Full rack fixed latency, samples |
|---:|---:|
| 44.1 kHz | 461 |
| 48 kHz | 480 |
| 88.2 kHz | 681 |
| 96 kHz | 720 |
| 192 kHz | 1200 |

## NOT certified

- True-peak ceiling compliance on full-band arbitrary/intersample-overloaded audio, ITU-R BS.1770 loudness, extremely rapid threshold/ceiling automation, and no-audible-artefacts guarantees.
- Cubase DAW load/save, host delay compensation, plugin insertion/removal, editor lifecycle, preset/A-B serialization.
- Commercial-level analog calibration and CPU headroom on actual Windows 11 machines.

The limiter sample guard and finite-value checks are safety mitigations, **not substitutes for validated output-true-peak control or monitoring levels**.
