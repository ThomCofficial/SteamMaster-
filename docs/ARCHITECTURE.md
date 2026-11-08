# SteamMaster DSP Architecture — decision log (work in progress)

## Release-target routing

Global Input -> Ocean -> Furnace -> Parametric EQ -> Pultec -> Shadow -> Exciter -> Baxandall -> Tape -> Global Stereo (Mono Maker and high-band width) -> Global Output -> Master Limiter -> final TP/LUFS meters -> host.

The limiter **must** follow all potentially gain-raising output and stereo transformations. Its bypass/True Peak OFF behaviors cannot claim a true-peak ceiling. Independent left/right limiting may undo mono bass; low-end stereo consistency must be addressed in the limiter architecture, without modifying user-approved global controls.

## Current milestone routing

`SixModuleRack::process()` runs only Ocean -> Furnace -> Parametric -> Pultec -> Shadow -> Exciter. There is NO limiter or host-side output control. The rack includes last-resort sample-domain containment at ±4.0 to prevent nonfinite/absurd bursts. This DOES NOT enforce 0 dBFS, true peak, or a safe consumer monitoring level.

## Real-time contract

- Instantiate/preallocate engines at `prepare(sampleRate)` off the audio thread.
- Call typed `set()` functions on **the audio thread only**, at sample/block boundaries. Never mutate parameter structs on the UI thread while the DSP reads them.
- Call `process(Stereo)` for each sample; no heap allocation, OS calls, file access, or mutex acquisition occurs in the processing implementation.
- `reset` and `prepare` are not called from the sample callback.
- Gain-reduction telemetry is currently DSP-side only and must be mirrored into a host-safe atomic/UI snapshot by the future adapter. Nonfinite and emergency event counters are atomic.
- Future host adapters must handle mono and stereo buffers, variable block sizes, transport changes, editor open/close, sample-rate changes and VST3 state lifecycle.

## Filter topology

`TptEQ` is a topology-preserving state-variable filter. Smooth positive internal `g` and `k` plus response weights rather than linearly sweeping arbitrary biquad numerator and denominator coefficients. Its frequency is clamped **internally** below Nyquist to preserve digital stability; this is not a GUI range decision.

## Nonlinear DSP & oversampling — pending production work

The Exciter's three paths already produce distinct nonlinear responses, with 15 ms transfer-function crossfades, but are currently evaluated at native rate. Ocean's mild `tanh` color is also native-rate. This **fails the final anti-aliasing requirement** for high drive until local linear-phase/FIR oversampling is implemented around nonlinear blocks.

The recommended JUCE host adapter should initialize `juce::dsp::Oversampling<double>` (or a purpose-built equivalent) in `prepareToPlay`, query `getLatencyInSamples`, use integer-latency mode if appropriate, preallocate worst-case scratch buffers and compensate dry paths. OFF/2×/4×/8× transitions should crossfade prepared paths with matched latency, and the reported host latency should stay fixed where feasible. See https://docs.juce.com/master/classjuce_1_1dsp_1_1Oversampling.html .

## Calibration boundary

Known UI control ranges and stepped positions are immutable. Direct DSP fields fill gaps in calculations **only as engineering values**; they are not new approved labels or hidden GUI widgets. Pultec's 0–10 amplitude -> gain curves, Shadow's 1–24 -> dB calibration and Exciter's precise saturation tone must be listening-tested and approved. Limiter artwork and global panel are entirely open.
