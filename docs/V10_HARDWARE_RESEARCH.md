# SteamMaster v1.0 — hardware evidence and calibration audit (2026-11-08)

**Scope:** Ocean (Manley Variable Mu), Furnace (SSL G-series bus compressor), Shadow (Shadow Hills Mastering Compressor), Pultec (EQP-1A). **No artwork, UI, or VST3 code altered.**

## Evidence policy — do not claim a measurement-matched clone

Three distinct levels of information must not be conflated:

1. **Documented hardware specifications:** manufacturer ratios, control ranges, circuit topologies, performance limits.
2. **Published plots and isolated measurements:** transfer curves in manuals, frequency response traces, bench THD+N captures under partially documented conditions. Some plots are qualitative or can only be digitized with limited precision.
3. **Complete device fingerprints:** controlled level-calibrated *multidimensional* sweeps and time-domain, frequency-domain, and harmonic responses at many drive, ratio, threshold, attack, release, signal-level, temperature, channel and load settings. **These are not publicly available as a validated set for the specific variants.**

SteamMaster v1.0 applies level-1 and partial level-2 information. The output CSV `V10_MODELED_CURVES.csv` is produced by **SteamMaster DSP**, **not** hardware. It must not be presented as measured equipment data. Identical harmonic spectra and transient envelopes cannot be certified without genuine reference signals and hardware measurements.

## Ocean — Manley Variable Mu

**Manufacturer's printed gain-reduction plots (Figure p.13)**: https://gzhls.at/blob/ldb/4/2/e/9/a5541d97bfa29d6f9bdf29428244250efbb7.pdf

- COMP: wide, progressively steeper soft knee. Figure text reports **approximately 1.5:1 at 2 dB of reduction, 2:1 at 5 dB, and 5:1 at 10 dB**; at extreme compression the effective ratio approaches approximately 8:1.
- LIMIT: nominal 4:1, moving toward approximately 20:1 at heavy GR (>12 dB), with threshold-position-dependent curvature. Attack 25–70 ms; five recovery positions 0.2/0.4/0.6/4/8 s.
- Differential push-pull variable-mu 5670 double triode rebias through rectified tube sidechain, balanced nickel-core transformers; linked stereo **control-voltage signals rather than summing audio**. Manufacturer specs quote 6 dB/oct SC HPF roughly −3 dB at 100 Hz, and `<0.1%` THD @ 1 kHz under typical conditions (not a full spectrum).
- Important variants: ordinary 5670 units, older 6386, 6BA6 T-Bar modification, mastering revision with attack 15–90 ms; **we model the ordinary 5670 edition**, not the T-Bar version. No variant should be silently substituted. https://www.manley.com/s/VARIABLE-MU-MANUAL-WEB-REV-521.pdf ; https://tapeop.com/reviews/gear/106/manley-variable-mu-plug-in-for-uad-2-apollo
- v1.0: progressively varying GR local ratio in a *precomputed* non-allocating, non-branchy 0.125-dB LUT. COMP and LIMIT have separate knee trajectories. Values are **engineering interpolation of sparse printed landmarks**, not a pointwise fit of an exhaustive hardware dataset. Six SteamMaster ratio positions remain (1.2/1.5/2/3/4/6); original hardware has only COMP and LIMIT. Tube transfer continues as conservative topology-based approximation and is **not** harmonic-matched. Existing 25–70 ms attack and five releases unchanged.

**Missing:** calibrated input and output levels, threshold-position curve mapping, tube transfer dependence on GR/control voltage, frequency/phase under gain reduction, harmonic spectra as function of drive, time-domain dynamic curves including channel link and input transformer hysteresis. The manual image cannot supply all of these.

## Furnace — original SSL G-series stereo bus compressor

**Primary documentation**:
- SSL: exceptionally broad **2:1 soft knee**, audible GR can occur with relatively low input level even at high threshold; ratio-dependent onset must differ from 4:1 and 10:1: https://support.solidstatelogic.com/hc/en-gb/articles/4408132343057-Why-does-the-Bus-Compressor-reduce-gain-when-the-threshold-is-fully-up
- SSL console description: true full-wave independent stereo detectors driving a **dominant** channel sidechain, shared linked gain; ratio-dependent soft knee, program-dependent AUTO release, attacks 0.1/0.3/1/3/10/30 ms. https://www.solidstatelogic.com/assets/uploads/downloads/six/SSL_SiX_User_Guide%20V1.0%20March_2019.pdf
- SSL's G-Master controls and ratios 2/4/10, releases 0.1/0.3/0.6/1.2/AUTO: https://www.bhphotovideo.com/lit_files/984286.pdf (Waves hardware-partnership reference).
- Modern SSL THE BUS+ **is not the 1989 console**, but provides analog-performance context: classic-stereo compressor THD+N typical around 0.004% at 1 kHz/0 dBu and published low-frequency modulation distortion under fast compression. No valid transfer of that full spectrum to original 4000G is implied. https://solidstatelogic.com/assets/uploads/downloads/The_Bus_User_Guide_1.1.pdf

v1.0: widen the 2:1 static knee from an unrealistically narrow 6 dB to a **provisional 24 dB**, with 4:1 at 6 dB and 10:1 at 2 dB. **The 24 dB value is not a published measured width**. Retain dominant stereo peak detector and two-time-constant AUTO release. The Furnace VCA signal path remains clean—no arbitrary invented harmonics added to reach a nonexistent “exact SSL saturation spectrum”.

**Missing:** actual dBu-to-DSP threshold control mapping, static I/O graphs for 2/4/10 at several threshold settings, stereo detector weighting and rectifier time constants, feedback network vs feed-forward topology verification by original board revision, and input-level-dependent THD+N/FFT through GR.

## Shadow — Shadow Hills Mastering Compressor

**Primary / partnered documentation**:
- Shadow Hills / Brainworx manual: independent **electroluminescent optical stage first** (fixed nominal 2:1), followed by feed-forward discrete VCA (ratios 1.2/2/3/4/6/20 Flood); stepped attack from 0.1–30 ms; recovery 0.1–1.2 s, **Dual** option. Transformer matrix changes frequency and distortion; no transistor/transformer lookup table is disclosed. https://files.plugin-alliance.com/products/shadow_hills_mastering_compressor/shadow_hills_mastering_compressor_manual.pdf
- Officially licensed plugin manual: optical gain reduction releases **~80% quickly**, remaining **~20% over more than 1 second**, level dependent. Same dual timing is used in discrete's DUAL position. https://manuals.plus/m/76eaaab16b2a1bacc1ea36963d7805acacfc2f4b231b2016fffcc690bfb09f96
- Hardware reverse-engineering/bench article (2026, secondary source, limited settings): optical THD+N ~0.0125% no compression vs ~0.11–0.13% with heavy compression, *but output levels also differ*, so these **must not** be treated as target harmonic fingerprints; transformer Nickel/Iron/Steel FFT plots and spectra are not provided as calibrated tabular samples. https://www.gainstaging.com/review/shadowhills-mastering-compressor

v1.0: the optical 80/20 dual memory was already present. **Corrected the discrete DUAL option** to actually employ independent 80% fast / 20% slow recovery states; the previous single time constant was not equivalent. This is still a topology-informed envelope model, not a hardware-matched optical cell or custom discrete VCA. The locked SteamMaster design has no transformer selector, so we **cannot claim a complete clone of the 3-transformer hardware** and do not invent a hidden selector.

**Missing:** illumination-to-photoresistance curves and aging/hysteresis, control response calibrations, full transfer/attack/recovery sweeps, stage loading, transformer nonlinearity vs level and frequency, FFT harmonics with matched input/output dBu and device revision.

## Pultec — Pulse Techniques EQP-1A

**Primary + academic sources:**
- Original manufacturer specifications: LF boost 0–13.5 dB, LF attenuation 0–17.5 dB, HF peaking boost 0–18 dB, HF attenuation 0–16 dB. Frequencies LF 20/30/60/100 Hz; HF peak 3/4/5/8/10/12/16 kHz; HF cut 5/10/20 kHz; ≤0.15% THD at +10dBm; 12AX7, 12AU7, 6X4 tube stage and transformers. https://pulsetechniques.com/products/tube-equalizers/eqp-1a/
- Academic virtual analog study (Barrera, Lizarraga-Seijas, Font, SMC 2024): WDF for original passive RLC network, LTspice component fitting using Audio Precision reissue measurement figures and original manual response curves; high sample rates needed to avoid frequency warping. Passive network ~16 dB insertion loss recovered by amplifier. https://repositori-api.upf.edu/api/core/bitstreams/a5b72470-a1f4-4020-a42e-670709310c85/content
- GPLv3 source code and supporting circuit simulations: https://github.com/ABSounds/EQP-WDF-1A ; **NOT copied or linked** into SteamMaster. SteamMaster may eventually have commercial distribution; GPL obligations must be evaluated before incorporating code.
- The Manley *Enhanced Pultec* manual confirms cutoff and boost are distinct, and boost + cut produces a low-mid dip, but it is a different product with different response and must not be used as a drop-in measurement source for Pulse Techniques originals: https://static1.squarespace.com/static/582f5ce4e4fcb562b921bdc3/t/590cf1e7893fc0f07b5f1d74/1494020584854/PULTEC%2Bmanual.pdf

v1.0: correct LF cut to a **true low shelf**, not an isolated peaking notch, plus a coupled mid-bass contour for simultaneous cut+boost; use manufacturer-quoted maximum gains, stronger resonant HF peaking, configurable bandwidth, 16 dB HF shelf cut. The coupled dip is an independently authored multi-TPT approximation; it is **not the research paper's complete passive-network WDF**. Existing locked UI only permits LF 30/60/100 and HF boost 8/12/16; original hardware has additional frequencies unavailable in SteamMaster.

**Missing:** measured frequency/phase traces of actual hardware across continuous pot travel, load-dependent passive network interactions, original 600-ohm path with transformer/nonlinear amp behavior, control pot tapers and accurate high-frequency prewarping. Do not label this measured/bit-exact.

## Test methodology and future true hardware fit

v1.0 adds `tests/TestHardwareReferences.cpp`: Pultec LF/HF response shape and maxima; nonzero **LF shelf** attenuation; simultaneous boost-and-cut contour; Manley progressive gain reduction with COMP vs LIMIT; SSL ratio-dependent knee; Shadow discrete DUAL fast/slow timing. Existing six test suites remain active. `tools/ReferenceCurveProbe.cpp` outputs **model-generated** CSV (`docs/V10_MODELED_CURVES.csv`) to line up with licensed physical-bench CSV later. The CSV cannot verify the real hardware on its own.

To claim *measurement-matched*, obtain at least these from each device with revision/serial-number recorded:

1. Audio Precision-style static sine sweeps from -30 to +24 dBu at 100/1000/10000 Hz, each hardware ratio, threshold, stage and pot position, capturing output level and GR meter position; calibrate dBu-to-dBFS explicitly (e.g., nominal +4 dBu equals -18 dBFS, if that is the selected reference).
2. Stepped/tone-burst attack/recovery measurements at varying duration, input amplitude and GR targets, several recovery histories; matched linked/asymmetrical stereo tests.
3. Level- and frequency-dependent THD/FFT with repeatable tone bandwidth, load, reference, and signal alignment, including compressor IN with 0 dB GR, operating GR 1/3/6 dB and heavy GR; isolate the dynamic modulation product from true gain-cell distortion.
4. Pultec complex transfer functions (phase and magnitude) for every discrete selector, multiple boost/cut/bandwidth positions and combined corners, then fit an independent circuit network or compatible-licensed WDF. Verify oversampling and CPU.
5. Held-out waveforms in true analog loopback with synchronized calibration, blind listening, null/level-matched comparisons; target tolerances established *before* fitting.

Current **no VST3 binary/no host validation**. CI checks compilation and standalone DSP safety/functional behavior, not subjective sound identity.
