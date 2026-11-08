#include "steammaster/BaxShelf.hpp"
#include <cmath>

namespace steammaster {
void BaxShelf::prepare(double sampleRate, Band band) noexcept {
    fs_=clamp(sampleRate,8000.0,384000.0);
    band_=band;
    integration_.prepare(fs_,.012);
    slope_.prepare(fs_,.012);
    integration_.reset(.01);
    slope_.reset(0.0);
    clear();
}
void BaxShelf::set(double faceplateHz, double gainDb) noexcept {
    // The Dangerous manual describes the LOW frequency marking as the
    // *bottom* shelf shoulder; HIGH is toward the center of the slope.
    // These are documented calibration estimates, not digitized hardware data.
    const double factor=(band_==Band::Low)?2.0:0.85;
    const double dcGain=dbToGain(clamp(gainDb,-5.0,5.0));
    const double center=faceplateHz*factor;
    // Symmetric pole/zero placement makes +N / -N dB mirror each other,
    // unlike a fixed-pole lowpass blend. In analog form:
    // LOW: (s + center*sqrt(G))/(s + center/sqrt(G))
    // HIGH: (G*s + center*sqrt(G))/(s + center*sqrt(G))
    // for HIGH the zero is at center/sqrt(G), so the boost and cut curves
    // are reciprocal with the same center frequency.
    const double pole=center*(band_==Band::Low?1.0/std::sqrt(dcGain):std::sqrt(dcGain));
    const double fc=clamp(pole,5.0,.45*fs_);
    const double g=std::tan(kPi*fc/fs_);
    integration_.set(g/(1.0+g));
    slope_.set(dcGain-1.0);
}
double BaxShelf::process(double input) noexcept {
    const double a=integration_.next();
    const double v=(input-state_)*a;
    const double low=state_+v;
    state_=sanitized(low+v);
    const double shaped=band_==Band::Low?low:(input-low);
    return sanitized(input+slope_.next()*shaped);
}
} // namespace steammaster
