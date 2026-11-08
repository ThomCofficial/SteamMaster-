# v0.8 compressor research and implementation boundary

This is a topology-informed approximation, **not** a measurement-matched hardware emulation. No hardware impulse, gain-transfer, distortion or burst-response measurements were acquired. Do not advertise exact SSL, Shadow Hills or vari-mu curves/harmonics.

## Reference families and open measurement work

- **Furnace (SSL 4000 G bus compressor family):** stereo-linked VCA topology; 2:1, 4:1, 10:1 ratio choices; stepped attack and release, program-dependent AUTO. See Solid State Logic bus compressor product documentation and operating manuals. v0.8 implements ratio-dependent soft-knee widths and parallel fast/slow AUTO gain-reduction recovery. Knee widths and release constants are engineering hypotheses, not measured SSL coefficients. No claimed SSL VCA harmonic match.
- **Shadow (Shadow Hills Mastering Compressor family):** serial optical then discrete compression. Optical nominal 2:1 and dual-speed recovery; discrete Class-A VCA with stepped ratios and time constants. See Shadow Hills Industries Mastering Compressor manual and Universal Audio Shadow Hills Mastering Compressor documentation. v0.8 changes optical stage to nominal 2:1 with 80/20 fast/slow gain-reduction recovery. Actual recovery curve and harmonic content still require measured reference. Transformer colors are NOT added to the locked Shadow UI or audio chain.
- **Ocean (vari-mu family):** variable-mu transfer and tube behavior depend on device and bias. Manley Variable Mu, Fairchild 660/670 and other vari-mu units are *different circuits*. The Ocean hardware reference is not specified; therefore no exact hardware calibration is possible. Existing soft-knee and tube approximation remains and must not be called a measured Fairchild/Manley reproduction.

## Required to achieve hardware matching

1. Identify exact hardware revisions and operating levels, including transformer selections and calibration.
2. Capture stepped steady-state input/output curves for every ratio/threshold and at several frequencies.
3. Measure attack and multi-stage release with controlled tone bursts at different GR depths, sample rates and stereo imbalance.
4. Measure THD harmonic spectra, IMD and frequency response versus drive/GR, with level-matched bypass.
5. Fit models to measurements; blind A/B and quantify errors; preserve existing realtime safety and regression tests.

Current tests demonstrate software stability and selected behavioral characteristics, **not** exact matching.
