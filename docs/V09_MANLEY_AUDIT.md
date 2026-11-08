# Ocean v0.9: Manley Variable Mu reference audit

Reference: Manley official Variable Mu owner's manual (rev 5.2.1), https://www.manley.com/s/VARIABLE-MU-MANUAL-WEB-REV-521.pdf

## Verified published hardware facts
- COMP: nominal 1.5:1 soft knee; LIMIT: nominal 4:1 firmer knee, rising to a maximum 20:1 under strong limiting (above approximately 12 dB reduction).
- Five recovery settings: 0.2, 0.4, 0.6, 4, 8 seconds.
- Continuous attack approximately 25–70 ms on standard model; mastering edition may have extended stepped attack.
- Fully differential all-tube design with transformer-balanced input/output; tube complement and revisions vary.
- Stereo linking and sidechain high-pass are documented hardware features.
- Published THD specification <0.1% at 1 kHz; does not specify a unique harmonic distribution at each drive level.

## v0.9 implemented
- More appropriate COMP/LIMIT-inspired progressive static curves; no longer arbitrary generic 8 dB knee.
- Existing five recovery settings and 25–70 ms attack preserved.
- Gentle differential push-pull inspired nonlinearity, conservatively mixed into oversampled wet path.
- All six existing SteamMaster ratio positions retained to avoid redesigning locked UI; intermediate ratios are NOT Manley hardware modes.

## Explicit limitations
No physical Manley unit has been measured. No matched transfer curves, harmonic-vs-level plots, frequency/phase response, input/output transformer model, calibration to dBu, or verified tube-bias transfer functions. This is a documented topology-informed approximation, NOT a circuit-level clone or a claim of identical hardware sound. Current Ocean stereo detector uses max-channel linked detection. Tube transfer is intentionally low-level and should be listening-tested.

## Next measurements
Capture steady-state 1 kHz input/output at multiple levels in COMP and LIMIT, 40/60/100 Hz and 10 kHz tone-burst gain reduction and recovery, harmonics at -18/-12/-6 dBFS for a calibrated reference level, and stereo-linked asymmetrical transients. Compare against both the real unit and a known validated emulation.