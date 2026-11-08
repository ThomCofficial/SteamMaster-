# SteamMaster v0.7 analog voicing audit

- Pultec: nonlinear pot response; simultaneous bass boost and cut now create low shelf plus broad upper-bass attenuation dip. High boost uses broad presence peak. These are **Pultec-inspired** curves, not measured or verified EQP-1A hardware response. No new controls.
- Parametric EQ: unchanged clean digital TPT filters.
- Tape: Airwindows-inspired slew rounding and memory-dependent nonlinear response, authored independently (no Airwindows source copied). The Airwindows repository is MIT licensed; source and specific algorithms should be evaluated separately before any port. Existing Studio/Vintage/Modern voicings remain provisional.
- Furnace/Shadow: compressor ratio, knee and time behavior remain existing approximations. SSL/Shadow Hills accuracy requires hardware/authorized reference sweeps with static input/output curves at each ratio and multiple signal levels, plus burst and step tests for attack/recovery, auto release, stereo link and harmonic response. No assertion of measured hardware authenticity.
- Six local Release CTest suites passed. Real hardware comparison, perceptual listening, high-rate CPU profiling, Cubase VST3 validation and arbitrary-signal stress testing remain outstanding.
