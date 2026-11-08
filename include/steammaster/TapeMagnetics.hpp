#pragma once
#include "steammaster/Core.hpp"
#include <array>

namespace steammaster {
// ORIGINAL, phenomenological nonlinear tape model based on magnetic memory,
// slew-dependent losses and saturated magnetisation. It is NOT an implementation
// of any third-party plugin or a hardware-identified Jiles-Atherton model.
// Real-time safe: fixed storage, no allocation, and no parameter-dependent reset.
class TapeMagnetics final {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    // quality: 0/1/2/3 for 1x/2x/4x/8x. weights are crossfading mode weights.
    Stereo process(Stereo input, unsigned quality, double drive,
                   double bias, double speedFactor,
                   const std::array<double,3>& weights) noexcept;
private:
    struct State {double previous=0.0, magnetisation=0.0;};
    struct Voice {double drive, hysteresis, slew, asymmetry;};
    static constexpr std::array<Voice,3> voices_{{
        {1.00,.095,.18,.035}, // Studio: Studer A800-inspired balanced compression
        {2.02,.135,.20,.125}, // Vintage: ATR-style more saturated mastering tape
        { .46,.025,.05,.012}  // Modern: high-headroom, low distortion formulation
    }};
    // All quality paths are kept warm in the oversampling processor's crossfade.
    std::array<std::array<std::array<State,2>,3>,4> states_{};
    std::array<double,4> memoryAlpha_{};
    static double run(double input, State& s, const Voice& voice,
                      double alpha, double drive, double bias,
                      double speedFactor, double oversample) noexcept;
};
} // namespace steammaster
