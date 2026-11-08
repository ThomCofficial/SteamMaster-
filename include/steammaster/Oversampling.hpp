#pragma once
#include "steammaster/Core.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace steammaster {

// Fixed-latency, sample-at-a-time, FIR 2x/4x/8x resampling for nonlinear
// processors. All storage is embedded, with no allocations in process().
enum class OversamplingMode : std::uint8_t { Off=0, X2=1, X4=2, X8=3 };

class StereoDelay {
public:
    static constexpr std::size_t kCapacity = 256;
    void prepare(std::size_t samples) noexcept;
    void reset() noexcept;
    Stereo process(Stereo in) noexcept;
    std::size_t latencySamples() const noexcept { return samples_; }
private:
    std::array<Stereo,kCapacity> buffer_{};
    std::size_t samples_ = 0, cursor_ = 0;
};

// Pure callback: only audio-DSP state in ctx may be mutated. qualityIndex is
// 0,1,2,3 for OFF,2x,4x,8x. No std::function/dynamic dispatch allocations.
using NonlinearCallback = Stereo (*)(Stereo in, unsigned qualityIndex, void* ctx) noexcept;

class FIRHalfBand2x {
public:
    static constexpr std::size_t kTaps=81;
    static constexpr std::size_t kCenter=(kTaps-1)/2;
    void prepare() noexcept;
    void reset() noexcept;
    void up(Stereo input, Stereo& even, Stereo& odd) noexcept;
    Stereo down(Stereo even, Stereo odd) noexcept;
private:
    Stereo delayed(std::size_t n) const noexcept;
    void push(Stereo sample) noexcept;
    std::array<double,kTaps> h_{};
    std::array<Stereo,2*kTaps> history_{};
    std::size_t cursor_=0;
};

class OversamplingPath {
public:
    static constexpr unsigned kMaxLevels=3;
    static constexpr std::size_t kLatency=70; // (40 + 20 + 10) host samples
    void prepare(unsigned levels) noexcept;
    void reset() noexcept;
    Stereo process(Stereo input, NonlinearCallback callback, void* ctx) noexcept;
private:
    unsigned levels_ = 0;
    std::array<FIRHalfBand2x,kMaxLevels> up_{};
    std::array<FIRHalfBand2x,kMaxLevels> down_{};
    StereoDelay compensation_{};
};

class OversampledProcessor {
public:
    static constexpr std::size_t kLatency=OversamplingPath::kLatency;
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    // Called by audio thread at a block boundary. No allocations/reinitialization.
    void setMode(OversamplingMode quality) noexcept;
    OversamplingMode requestedMode() const noexcept {return requested_;}
    OversamplingMode activeMode() const noexcept {return active_;}
    bool transitioning() const noexcept {return active_ != incoming_;}
    Stereo process(Stereo input, NonlinearCallback callback, void* ctx) noexcept;
private:
    void beginTransition(OversamplingMode next) noexcept;
    std::array<OversamplingPath,4> paths_{};
    OversamplingMode active_=OversamplingMode::Off;
    OversamplingMode incoming_=OversamplingMode::Off;
    OversamplingMode requested_=OversamplingMode::Off;
    std::uint32_t warmup_=0, fadePos_=0, fadeSamples_=576;
};

} // namespace steammaster
