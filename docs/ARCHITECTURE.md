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

## Nonlinear DSP & oversampling — v0.2 implemented baseline

The stand-alone DSP now contains `FIRHalfBand2x`, `OversamplingPath`, and
`OversampledProcessor` (`include/steammaster/Oversampling.hpp`,
`src/Oversampling.cpp`):

- Three cascaded **81-tap linear-phase Blackman-Harris halfband** interpolation
  and decimation stages, with polyphase zero-tap elimination.
- OFF, 2x, 4x, 8x, and a **70-sample constant native-rate latency** per
  oversampled module. Natural group delay for paths is 0/40/60/70 samples;
  their compensators supply 70/30/10/0 samples respectively.
- Ocean and Exciter each return 70-sample-delayed wet processing and align dry,
  original-band and bypass paths. The six-module rack's current DSP latency is
  **140 host samples** (Ocean + Exciter), invariant with global quality. That is
  only a DSP-core measurement until a VST3 host adapter reports it to Cubase.
- Prepared quality paths and real-time-safe warmup/crossfades; the next
  requested quality is coalesced while one transition is running. All FIR
  buffers and coefficient storage are fixed-size and initialized off the audio
  thread. The active CPU cost grows with oversampling factor.
- Exciter Tape-like personality maintains per-quality nonlinear memory and
  only calculates transfer functions with non-negligible interpolation weight.
  Tape module itself has **not** been implemented.
- Do **not** oversample linear filters or detector dynamics for no reason.
  No global VST3 oversampling selector exists yet; `SixModuleRack::setOversamplingMode`
  is an audio-thread-side interface.

Remaining oversampling work includes full-spectrum alias sweeps at different
sample rates, subjective/level-matched listening, proper Tape-module nonlinear
processing, and host-backed latency validation. At 192 kHz and 8x, internal
processing reaches 1.536 MHz and requires explicit Windows CPU profiling.

## Host boundary still missing

When constructing a VST3 wrapper, return the constant DSP latency via the host
latency interface, preallocate block/scratch storage, and transfer parameters
through a lock-free snapshot or queue. No JUCE or VST3 wrapper has been
compiled or tested in this checkpoint. The host must ensure parameter-setters
run on the audio thread at appropriate boundaries, and GUI metering must use
rate-limited atomics/snapshots rather than reading live DSP state across threads.

## Calibration boundary

Known UI control ranges and stepped positions are immutable. Direct DSP fields fill gaps in calculations **only as engineering values**; they are not new approved labels or hidden GUI widgets. Pultec's 0–10 amplitude -> gain curves, Shadow's 1–24 -> dB calibration and Exciter's precise saturation tone must be listening-tested and approved. Limiter artwork and global panel are entirely open.

## v0.3 actual implementation (8 November 2026)

The `MasteringRack` in `include/steammaster/Mastering.hpp` and `src/Mastering.cpp` adds the final three DSP stages and global stereo/gain handling to the previous `SixModuleRack`. The earlier milestone-only notes above remain as historical design decisions. The new whole-rack path is available in the library, **not as a DAW plug-in**.

### Routing details

The new Baxandall runs the two row processors for both M/S and L/R concurrently, smoothing transitions between modes. It uses the approved discrete frequency arrays with exact stored enum values. Frequencies unrepresentable at the current Nyquist are effectively bypassed rather than silently changed to different labeled values. No saturation or independent LINK audio processor is added.

Tape uses high-frequency trim, speed-dependent bump voicing, linked program-dependent transient control, oversampled memory-assisted nonlinear transfers and a slow DC servo. All voicing coefficients are preliminary engineering decisions rather than approved hardware calibrations.

Mono Maker applies an HPF to the Side channel after upper-band-only widening; the filter progressively attenuates low Side content. Stereo link is forcibly 100% while Mono Maker is enabled, conservatively preserving centered bass through the limiter. An independently linked low-frequency limiter envelope would improve flexibility and needs future design approval.

`MasterLimiter` buffers input and raw bypass paths separately so threshold automation at the detector cannot be mismatched with the pre-gain applied to delayed output. Its lookahead uses the fixed maximum lookahead plus a 30-sample reconstruction-detector delay. A four-times FIR peak detector and a corresponding output meter are in place. **True-peak compliance and ceiling overshoot under worst-case ISP sequences are not certified.** Actual limiter modes, controls and release behaviors remain subject to final design approval.

Latency is always `140` first-six + `70` Tape + `30 + ceil(0.005*fs)` limiter samples; native-rate EQ and global stereo do not add explicit processing delay. Full-rack bypass uses an equivalently delayed original input, with smoothing. This is a DSP-core contract only until the VST3 host adapter reports it.

### CPU and allocation

All ring buffers and FIR coefficient arrays have fixed size and are prepared ahead of processing, with no allocations inside the sample callbacks. High sample rates and 8x nonlinear mode plus the 4x limiter TP meter remain CPU-intensive. Windows machine profiling, block-size performance and real DAW lifecycle tests are needed.
