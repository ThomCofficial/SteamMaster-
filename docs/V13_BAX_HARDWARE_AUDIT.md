# SteamMaster Bax EQ v1.3: hardware-informed response audit

## Scope
**DSP ONLY.** No artwork, GUI, controller mapping, or interface changes. Reference target is **Dangerous Music BAX EQ** (hardware). Existing approved Mid/Side and Left/Right functionality, both independent rows, bypass switches, exact frequency labels, and +/-5 dB in 0.5 dB steps are preserved.

## Primary manufacturer and engineering references
1. Dangerous Music, *BAX EQ User Guide*, v6, esp. pp. 3–9: https://www.dangerousmusic.com/s/BAX-EQ-Manual-v6.pdf
2. Dangerous Music, BAX EQ hardware specifications: https://www.dangerousmusic.com/product/bax-eq/
3. Tape Op review hosted by Dangerous Music (high vs low shoulder descriptions): https://www.dangerousmusic.com/news-press/2010/09/15/tapeop-bax-eq-review
4. Alan Silverman, *Dangerous Music BAX EQ*, Mix Online, 2011 (feedback-network topology context): https://www.mixonline.com/recording/dangerous-music-bax-eq
5. Dangerous Music Digital BAX plugin: https://www.dangerousmusic.com/product/baxeq-plugin — official digital product states its calibration involved two physical units, Audio Precision measurements, and repeated listening. The proprietary measurement tables **are not public** and were not used here.

### Hardware features established by references
- Broad, low-phase-shift low and high Baxandall shelves, rather than narrow Q=0.707 biquad shelves. Faceplate low-frequency labels are at the *bottom shoulder*; high-frequency labels are in the *middle of the slope*, with the upper shoulder approximately 2x the marking.
- 8 low shelf values 74, 84, 98, 116, 131, 166, 230, 361 Hz; 8 high shelf values 1.6, 1.8, 2.1, 2.5, 3.4, 4.8, 7.1, 18 kHz.
- 12 dB/oct high-pass at 12/18/24/30/36/43/54 Hz and low-pass at 70/28/18/12.6/11.1/9/7.5 kHz plus OUT; relays bypass their analog circuits entirely in OUT.
- Hardware is designed to be clean: quoted THD+N <0.002%, IMD <0.003%, and nominal flat 10 Hz–20 kHz response within +/-0.1 dB when unaffected.

## Independent DSP implementation (not a reverse-engineered hardware clone)
The old low/high shelves used second-order Q=0.707 TPT shelf responses with frequencies treated as generic biquad corners. The new `BaxShelf` is a **single-pole/single-zero analog shelving prototype** transformed into a stable, trapezoidal-integrator digital filter.

Define `G = 10^(gainDb/20)` and `w = 2*pi*f_center`. The two complementary analog shelf responses are:

- Low: `H_low(s) = (s + w*sqrt(G)) / (s + w/sqrt(G))`.
- High: `H_high(s) = (G*s + w*sqrt(G)) / (s + w*sqrt(G))`.

The pole placement adjusts with gain, making boosts and cuts reciprocal in the analog limit. A stable, one-pole TPT lowpass plus an aligned direct branch implements either shelf with no artificial resonance and no induced harmonics. Unity setting yields **sample-exact bypass** at zero shelf gain, even with nonzero internal filter memory. All pole coefficients and gains are smoothed over 12 ms; no allocations or trigonometric functions run in the per-sample `process()` path.

Because published plots are illustrations rather than numerical calibration files, the faceplate-to-prototype mappings remain **engineering hypotheses**: low `f_center = 2.0 * labelHz`, high `f_center = 0.85 * labelHz`. These create broad shoulders and are *not* asserted to equal the precise Dangerous hardware curves. Frequency mapping can be recalibrated if actual numeric Audio Precision traces become available.

The existing 12 dB/oct Butterworth-style TPT HPF/LPF remain in place with independent smooth OUT transitions. The existing DSP policy of disabling low-pass cutoff positions above 0.45 x sample rate (e.g., 28 kHz at 44.1 kHz) remains an explicit accuracy limitation: a real ultrasonic filter can still affect frequencies below Nyquist. A future improved digital approximation should preserve the original analog in-band magnitude and phase without excessive warping near Nyquist.

## Verification performed
- **10 complete CTest suites** pass in C++17 Release and optimized ASan/UBSan on Linux.
- `TestBaxReference` checks 44.1/48/96/192 kHz; sample-exact unity in L/R and M/S; low/high shelf ±5 dB asymptotes; reciprocal boost/cut; 12 dB/oct 54 Hz HPF and 7.5 kHz LPF behavior; M/S-L/R routing and independent bypass; rapid parameter automation and finite bounded outputs.
- `BaxResponseProbe` produces `V13_BAX_MODELED_RESPONSE.csv` at 96 kHz. Examples: low +5 @74 Hz -> +4.91 dB at 20 Hz / +3.94 dB at 74 Hz / +0.11 dB at 1 kHz. High +5 @1.6 kHz -> +2.89 dB at 1.6 kHz / +4.29 dB at 3.4 kHz / +4.91 dB at 10 kHz. These are **model outputs**, not hardware measurements.
- Entire project builds without copying GPL-licensed, closed-source, or proprietary BAX DSP.

## Remaining before claiming measurement-matched hardware sound
1. Obtain a physical BAX or complete calibration files, then capture amplitude/phase at all 8 frequency positions, all shelf gain steps, and filter cut positions on an Audio Precision or equivalent setup; match at several hardware calibration levels.
2. Compare step/impulse and phase/group-delay responses, frequency interaction of simultaneous low/high controls, stereo tracking, and Mid/Side routing.
3. Compare THD+N and IMD at multiple calibrated dBu levels. We have **not** added saturation, noise or analog transformer coloration to a unit specified to be unusually transparent.
4. Reproduce ultrasonic cutoff effects when the analog cutoff exceeds the digital Nyquist limit, especially at 44.1/48 kHz.
5. Run level-matched ABX listening against a physical unit. None has yet been done.

This is a meaningful hardware-informed topology improvement, not an independently proven component-level hardware reproduction.
