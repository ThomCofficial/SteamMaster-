#pragma once
#include "steammaster/DSP.hpp"

namespace steammaster {
// Independently authored, first-order analog-prototype shelving network.
// H_low(s)=(s+G*w0)/(s+w0), H_high(s)=(G*s+w0)/(s+w0).
// Implemented as a trapezoidal (TPT) integrator, with smoothed physical pole
// and linear gain. No circuit schematic or proprietary calibration is claimed.
// Unity-gain shelves are EXACTLY transparent (not just approximately flat).
class BaxShelf final {
public:
    enum class Band { Low, High };
    void prepare(double sampleRate, Band band) noexcept;
    void set(double faceplateHz, double gainDb) noexcept;
    double process(double input) noexcept;
    void clear() noexcept { state_ = 0.0; }
private:
    Smoother integration_{}, slope_{};
    double fs_=48000.0, state_=0.0;
    Band band_=Band::Low;
};
} // namespace steammaster
