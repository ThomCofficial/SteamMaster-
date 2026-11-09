# SteamMaster v1.6 VST3 test bench — engineering validation

Temporary digital interface, no work on the locked visual design. JUCE 8 fetched at build time. Builds as Windows VST3 with an automatable, persistent 107-parameter tabbed editor.

## Scope
- All 9 DSP modules in the processing chain; global gain, oversampling, stereo imaging, and true-peak limiter.
- All available DSP controls mapped to JUCE APVTS; full-chain random stress at 44.1/48/96/192 kHz, 75 blocks per rate with 256 samples per block.
- Full-chain test checks finite, bounded output and reset recovery. Extreme parameter settings may trigger emergency clipping and are not considered sonically acceptable.
- Host smoke test constructs processor, editor, serializes state, changes every parameter across extremes and random settings, processes blocks and destroys processor at 44.1/48/96 kHz.
- DSP-only build stays JUCE-free. Plugin build is an optional CMake target, isolated in its own CI workflow.

## Outstanding gates
- Windows VST3 compilation and smoke test must pass GitHub CI before the artifact is considered downloadable.
- Actual Cubase 13 scan, editor open/close, insert removal and reload cannot be validated in GitHub CI.
- CPU benchmark against real Cubase projects, DAW automation playback, multi-instance tests and latency-change handling remain required.
- Some extreme knob settings may sound unpleasant or trigger output emergency clamps; never test at loud monitoring levels.
- This is not a calibrated hardware clone or mastering release.
