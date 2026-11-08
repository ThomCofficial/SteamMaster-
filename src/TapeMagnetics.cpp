#include "steammaster/TapeMagnetics.hpp"
#include <algorithm>
#include <cmath>

namespace steammaster {
void TapeMagnetics::prepare(double sampleRate) noexcept {
    const double fs=clamp(sampleRate,8000.,384000.);
    for(unsigned q=0;q<4;++q) {
        const double rate=fs*static_cast<double>(1u<<q);
        // Short magnetic relaxation. This is a calibrated-by-ear *hypothesis*,
        // not a claimed physical remanence constant for 456/GP9 tape.
        memoryAlpha_[q]=1.-std::exp(-1./(rate*.00045));
    }
    reset();
}
void TapeMagnetics::reset() noexcept { states_={}; }

double TapeMagnetics::run(double x,State& s,const Voice& voice,
                         double alpha,double drive,double bias,
                         double speedFactor,double os) noexcept {
    x=clamp(x,-16.,16.);
    // Slew limiter is normalised to approximately the same *physical* change
    // per second irrespective of oversampling selection.
    const double derivative=(x-s.previous)*os;
    s.previous=x;
    const double softenedDerivative=std::tanh(1.4*derivative)/1.4;
    const double slewLoss=voice.slew*speedFactor*(derivative-softenedDerivative)/os;
    const double history=x-s.magnetisation;
    const double field=x-slewLoss-voice.hysteresis*speedFactor*history;
    // Overbias: less distortion and lower HF output; underbias: more distortion.
    // The latter's HF loss is independently handled by the replay EQ.
    const double k=clamp(drive*voice.drive*(1.-.042*bias),.12,5.);
    const double b=voice.asymmetry+.021*bias;
    const double offset=std::tanh(k*b);
    const double denominator=std::max(.12,k*(1.-offset*offset));
    const double magnetic=(std::tanh(k*(field+b))-offset)/denominator;
    // A bounded, causal magnetic memory state: no hidden delays/noise/wow.
    s.magnetisation=clamp(s.magnetisation+alpha*(magnetic-s.magnetisation),-16.,16.);
    return sanitized(magnetic);
}
Stereo TapeMagnetics::process(Stereo x,unsigned quality,double drive,
                              double bias,double speedFactor,
                              const std::array<double,3>& weights) noexcept {
    const unsigned q=std::min(quality,3u);
    const double os=static_cast<double>(1u<<q);
    Stereo result{};
    // Every personality stays stateful during crossfade, avoiding state jumps
    // when toggling Studio/Vintage/Modern under sustained program material.
    for(unsigned i=0;i<3;++i) {
        auto& left=states_[q][i][0];auto& right=states_[q][i][1];
        const double l=run(x.l,left,voices_[i],memoryAlpha_[q],drive,bias,speedFactor,os);
        const double r=run(x.r,right,voices_[i],memoryAlpha_[q],drive,bias,speedFactor,os);
        result.l+=weights[i]*l;result.r+=weights[i]*r;
    }
    return {sanitized(result.l),sanitized(result.r)};
}
} // namespace steammaster
