# SteamMaster v1.1 — Original, circuit-derived Pultec EQP-1A work

## Licensing provenance (important)

- **SteamMaster implementation:** new C++ written independently from component values and published transfer-function principles. No files or code were copied from the GPL-3.0 `ABSounds/EQP-WDF-1A` implementation, no dependency on that project or its licensed DSP library. This is not legal advice; a later commercial release needs a proper IP/trademark review.
- **Primary electronic diagram:** Sowter's independently available `Pultec EQP 1A.pdf`, final page, passive filter section. https://www.sowter.co.uk/schematics/Pultec%20EQP%201A.pdf
- **Manufacturer specifications:** Pulse Techniques EQP-1A, https://pulsetechniques.com/products/eqp-1a
- **Background paper (research, not code):** Barrera, Lizarraga-Seijas, Font, *Modeling the Pultec EQP-1A with Wave Digital Filters*, SMC 2024. https://repositori-api.upf.edu/api/core/bitstreams/a5b72470-a1f4-4020-a42e-670709310c85/content
- **Original operation manual:** https://www.thehistoryofrecording.com/Manuals/Pultec/Pultec_EQP-1A_Manual.pdf

## What's now actually modeled

1. An independently authored **passive low-frequency RC network**. Two input/output electrical nodes solve a conductance matrix per sample; capacitor branches use bilinear/trapezoidal state equations for **branch current and capacitor voltage**, rather than 2–3 arbitrary low-shelf/peaking digital EQs. A switched capacitor selection informed by the published diagram: low atten 30/60/100 Hz → 27/8.2/12 nF; low boost 30/60/100 Hz → 0.82/0.22/0.33 µF. Continuously varying control resistance alters each branch loading. Boost and attenuation **interact in the same solved network**, producing the bass boost + low-mid dip without a separately inserted peaking-notch filter.
2. A **series RLC-derived HF boost** with the original diagram's component pairings at 8 kHz (8.2 nF/47 mH), 12 kHz (5.6 nF/27 mH), 16 kHz (3.3 nF/27 mH). The response is computed by bilinear-transforming a resonant current/voltage transfer; effective damping follows the selected band-width pot position. This is not a physical, fully loaded inductor model.
3. An **RC-derived HF attenuation shelf** with selected 5/10/20 kHz capacitor options (.22/.068/.068 µF) and effective terminal resistances. No generic high-shelf TPT filter remains inside this Pultec module; other clean Parametric EQ filtering is unchanged.
4. A calibrated **4 kHz passband reference gain restoration**, modeling the idea of recovery after a lossy passive section. This adjustment varies with boost/cut settings; it is an intentional calibration approximation, not a separate tube amplifier simulation.
5. **20 ms crossfade between individually stateful network configurations** to reduce discontinuities on discrete selector changes; fixed-size storage and no runtime allocation in the audio callback.

## Crucial distinctions: this is NOT an exact original netlist

- The original passive filter has interconnected branches, multiple potentiometer taps, input/output transformer loading, and control-dependent insertion losses. The low-frequency two-node ladder, effective **620 Ω / 80 kΩ / 10 kΩ** termination and control resistor maps are **reduced equivalent networks**, not original exact trace values; they were chosen to approximate published response shapes. The historical schematic does **not** authorize calling these loads genuine hardware-component values.
- The RLC resonance uses an effective damping parameter and a peak-gain branch; the original circuit does not supply that independent peak-gain source. The HF attenuation is also a reduced network, not a full switch/pot wiring transcription.
- There is **no physical 12AX7/12AU7 valve circuit, tube bias, input/output transformer inductance, magnetic hysteresis, hardware noise, or nonlinearity** in this revision. The passive network is deliberately linear and does **not** invent tube harmonics.
- Not a measurement-matched clone; no frequency-by-frequency error against independently measured hardware curves has been established. Comparisons currently assert functional response shapes and manufacturer maxima, **not** accuracy to within a specified tolerance.
- GPU/UI changes are out of scope. Original SteamMaster Pultec controls (LF 30/60/100; HF boost 8/12/16, HF attenuation 5/10/20) are preserved. Wider historical 20 Hz, 3/4/5/10 kHz selectors are not exposed.

## Engineering checks

Added `TestPultecCircuit.cpp` with eight-suite CI integration: neutral response, LF boost/cut and simultaneous interaction, RLC bandwidth/peak, RC high-cut, sample rates 44.1/48/96/192 kHz, reset during a crossfade, stereo isolation, rapid automation boundedness, and exact amplitude linearity (passive network should not add spurious harmonics). Legacy tests continue to run. All audio-thread paths use precomputed coefficients, 2×2 algebra and fixed-size history arrays.

## Next stage for an accurate clone

- Translate every node and potentiometer tap of the complete independent schematic into a nodal netlist; compare controlled circuit simulations with the original PDF's plotted responses at low/high boost/cut combinations.
- Independently digitize manufacturer and laboratory magnitude **and phase** plots under explicit control settings; establish a predeclared accuracy tolerance against held-out cases.
- Model valve stages/transformers separately only with measured level-dependent harmonic spectra and dynamic response; no fake `tanh` makes the Pultec a tube clone.
- Profile CPU cost and verify host/transport automation in Windows/Cubase after the DSP becomes a VST3 (not yet present).
