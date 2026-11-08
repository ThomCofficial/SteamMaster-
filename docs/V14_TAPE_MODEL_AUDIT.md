# SteamMaster Tape v1.4 — original magnetic memory / repro EQ model

**Status:** DSP engineering update; **not** a hardware-measurement-matched Studer A800 or Ampex ATR-102 clone. No VST3 / GUI work. No third-party source code copied.

## Primary and technical references

1. Studer A800 original 1981 service information, *Tape Deck and Audio Adjustments* SI 66/81 E, including bias-setting target and recorder calibration: https://www.reeltoreel.nl/studer/Public/Products/Recording_Analog/A800/Technical_Info/SI_066-81_E_A800_TD_Audio_Adj.pdf. A particular bias adjustment is specified as -4 dB at 10 kHz for 15/30 IPS models, -8 dB for 7.5/15 IPS models; this is a hardware setup measurement, not a direct plugin control value.
2. Universal Audio's Studer A800 technical manual, for speed, tape formula, bias, HF calibration, and head bump behavior: https://help.uaudio.com/hc/en-us/articles/4419513099156-Studer-A800-Multichannel-Tape-Recorder-Manual. Gives nominal 250 +3 / 456 +6 / 900 and GP9 +9 fluxivity calibrations. These calibration numbers are *not* available as controls in SteamMaster.
3. Universal Audio's Ampex ATR-102 manual: https://help.uaudio.com/hc/en-us/articles/32491139365140-Ampex-ATR-102-Mastering-Tape-Recorder-Manual. Confirms speed-dependent bumps, distortion, tape formulation, 15 IPS NAB/CCIR and 30 IPS AES emphasis, bias/HF calibration, and distinct record/repro electronics.
4. *Real-time Physical Modelling for Analog Tape Machines*, DAFx-19: https://www.dafx.de/paper-archive/2019/DAFx2019_paper_3.pdf. Explains that magnetic recording requires stateful magnetisation/hysteresis and a nontrivial record/repro head response, rather than a memoryless static waveshaper.
5. Airwindows' ToTape6 technical commentary (conceptual inspiration; NO copied algorithms or source code): https://www.airwindows.com/totape6/. No Airwindows plugin binaries or third-party dependencies are linked.

## Implemented architecture

**Record/preparation:** The previous simplistic `atan`/`tanh` blend is replaced by a separate `TapeMagnetics` processor. Each personality has a *different* saturating transfer coefficient, bias/asymmetry term, nonlinear slew limiter, and causal short-term magnetic memory. Magnetisation state evolves per stereo channel, voice, and oversampling mode. The three voices remain stateful while the 20 ms personality crossfade runs; there is no reallocating on the audio thread. The 2x/4x/8x FIR-oversampling core is unchanged.

**Profiles:** Studio references the Studer A800's balanced, flux-dependent character. Vintage takes a more driven, colored ATR-102 style mastering tape approach. Modern is a high-headroom formulation with substantially less low-level distortion. **These are sonic design targets, not numerical component measurements or identifiable tape stocks.** All profiles use the previously approved `Studio/Vintage/Modern` selector; no extra GUI controls exist.

**Bias:** Negative bias increases nonlinear drive/asymmetry; positive bias reduces it and increases high-frequency attenuation. These are *qualitative* responses that follow how under-/overbias affect tape. The parameter `-5..+5` is *not* mapped to physical oscillator volts or Studer's -4/-8 dB setup readings.

**Playback:** Speed-dependent head-bump centre/level and upper-frequency loss are applied after the magnetic nonlinearity. An independent post-repro HF Trim stage remains `±6 dB`. Resonance and loss coefficients are smoothly changed; no instantaneous filter-state reset occurs. `7.5 IPS` is deliberately warmer/narrower-band than `30 IPS`; `15 IPS` is intermediate. The DC servo is 5 Hz, and its coefficient is precomputed off the sample loop.

**Level calibration:** A hidden smoothed compensating gain is based on *this model's* low-level, -10 dB Drive, 1 kHz transfer measurement at 48 kHz. This keeps all nine mode/speed combinations within 0.01 dB in the measured regression probe and prevents loudness from confounding A/B; it is **not** a calibration to the original deck's dBu or nWb/m reference. Drive intentionally changes program level and saturation.

**Safety:** No optional noise, hiss, wow/flutter, feedback delay, or extra controls. Bypass, dry/wet latency, fixed DSP delay and host-facing shape are unchanged. The saturator remains bounded with fixed state; the full limiter and nine-module rack tests still run.

## Measured *DSP outputs* (48 kHz, no external hardware involved)

| Test | Result |
| --- | --- |
| Tone H3 at Drive +9 dB, 1 kHz, input 0.65 | Studio 0.0473, Vintage 0.0656, Modern 0.0188 (absolute harmonic amplitudes) |
| H3 Vintage Bias -5 / +5 | 0.0596 / 0.0500 (positive overbias less distortion) |
| Fundamental 10 kHz, 7.5 vs 30 IPS | 0.0562 vs 0.0693, same input |
| Fundamental 46 Hz, 7.5 vs 30 IPS | 0.1161 vs 0.0883, same input |
| 1 kHz low-level gain, all 9 combinations | -0.0045 to -0.0003 dB |
| All 11 suites | Release passed, sanitizer pass recorded after final build |

`docs/V14_TAPE_MODELED_RESPONSE.csv` contains **model-generated**, not digitized hardware, low-level frequency responses from an independent C++ probe. It is generated from `tools/TapeResponseProbe.cpp` and should be regenerated after further model adjustments.

## What is NOT solved

- **No Studer or ATR physical machine was measured.** We have no level-swept harmonic spectra, coercivity/hysteresis-loop data, head frequency/phase traces, bias tuning measurements, or nonlinear multi-tone tests for specific machine/tape/calibration configurations. Cannot claim measurement matching.
- The magnetic solver is *phenomenological* (smooth saturating magnetisation and a short relaxation state), **not** a full Jiles-Atherton head-and-oxide differential model. No transformer or discrete record/repro amp circuit netlists are included.
- Playback EQ is an original fitted-voicing shelf and bump, **not** exact historical NAB/IEC/AES coefficient implementation or verified tape-stock curves.
- Calibration level, head width, tape formulation, wow/flutter and hiss are deliberately not exposed in the approved interface. Real hardware measurements and **level-matched blind listening** are the next milestones.
- CPU load should be profiled inside the eventual VST3/Cubase host; the processor computes three independent memory voices at high oversampling to keep switches smooth.

## Reproduce

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/steammaster_tape_response_probe > docs/V14_TAPE_MODELED_RESPONSE.csv
```
