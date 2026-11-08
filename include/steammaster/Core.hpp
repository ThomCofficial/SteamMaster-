#pragma once

namespace steammaster {
struct Stereo { double l = 0.0, r = 0.0; };
constexpr double kPi = 3.1415926535897932384626433832795;
double dbToGain(double db) noexcept;
double gainToDb(double gain) noexcept;
double sanitized(double value) noexcept;
double clamp(double value, double lo, double hi) noexcept;
} // namespace steammaster
