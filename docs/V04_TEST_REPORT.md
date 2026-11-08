# SteamMaster DSP v0.4 — limiter hardening / spectral characterization

**Scope:** DSP only. No VST3, JUCE adapter, UI, images, or controls modified.

## Implementation changes

- Limiter true-peak detector and output measurement upgraded from **4x to 8x** FIR reconstruction, increasing the intrinsic detector delay from 30 to **35 host samples**.
- Full limiter lookahead latency becomes **35 + ceil(0.005 * fs)** samples. The full nine-module rack remains internally fixed-latency across lookahead selection and global oversampling modes: **210 + 35 + ceil(0.005 * fs)** samples. No VST3 latency reporting is implemented yet.
- A per-channel gain-release hold prevents the gain envelope from recovering during the selected lookahead interval after detecting a transient. This remains an approximate limiter and not a verified reference-quality true-peak algorithm.
- Added an observable sample-domain safety-clip event counter to expose, rather than hide, residual limiter protection interventions. This is a diagnostic for audio-thread-only reading, not a GUI meter binding.
- Added a separate **8x output reconstruction stress test** that detects ISP overshoot which sample-only ceiling tests cannot find.
- Added EQ plateau/peak response checks across 44.1, 48, 88.2, 96 and 192 kHz; repetitive Tape mode/drive/bias/speed/oversampling automation stress tests; and a 48 kHz 11 kHz/15 kHz third-harmonic alias test for all three tape types.

## Outcome and important limitations

- Local CMake Release: **4/4 CTest suites pass**.
- Test material includes deterministic wideband random transients with signal bursts, unlinked stereo, 0.1/1/5 ms lookahead and true-peak ON/OFF, across five sample rates.
- Under the hardest artificial burst tests, TP ON still permits up to approximately **0.63 dB above the selected ceiling** (observed against an additional 8x output reconstruction check); sample-domain safety clipping may occur. This is **a measured defect, not passing TP ceiling compliance**. The characterisation test bounds known overshoot at ceiling +0.75 dB only to catch larger regressions and does **not** certify any TP ceiling at that tolerance or other input signal. If true-peak limiting must be reliable, the algorithm must be replaced/refined and tested against independent BS.1770-conformant methods and large adversarial input sets before release.
- On the specific high-drive 11 kHz sinusoidal stimulus at 48 kHz, 8x Tape mode attenuates a measured third-harmonic alias at 15 kHz by approximately **113 dB** relative to OFF. This is **one test tone**, not a global alias-free certification or an audio quality claim.
- Analog hardware character, compressor envelope calibration, Pultec interactions, all unresolved parameter mappings, LUFS reporting, Windows/Cubase DAW lifecycle, host-side preset/state/thread safety, latency reporting, and CPU at 192 kHz / 8x are still unvalidated.

## Verification commands

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/steammaster_characterization_tests
```

CI must pass Windows/Linux Debug/Release plus ASan/UBSan before merging this milestone. Preserving reference results will allow the eventual true-peak algorithm replacement to be compared against the failing overshoot profile documented here.
