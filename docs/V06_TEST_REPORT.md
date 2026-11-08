# v0.6 DSP gain-reduction telemetry

Scope: no GUI, no VST3, no new controls.

- `MasterLimiter::gainReductionDb()` now includes both the first limiting stage and the post-correction gain stage.
- Reported reduction accounts for each stage's actual bypass blend, rather than presenting a gain reduction while the processor is bypassed.
- The threshold pre-gain is intentionally excluded from reduction metering.
- Added `SteamMasterMeteringTests` at 44.1/48/96/192 kHz: finite and nonnegative readings, linked channel symmetry, nonzero active reduction, zero settled bypass reduction, and reset clearing.
- Original four CTest suites remain unchanged and passed locally in Release.

This is not yet a validated integrated loudness or true-peak meter. Meter readings are instantaneous effective attenuation of the current envelopes, not an exact per-sample ratio of output/input (which would be undefined around zero crossings and delayed by lookahead).
