#include "steammaster/Oversampling.hpp"
#include <algorithm>
#include <cmath>

namespace steammaster {
void StereoDelay::prepare(std::size_t delay) noexcept {
    samples_=std::min(delay,kCapacity-1);
    reset();
}
void StereoDelay::reset() noexcept {
    buffer_.fill({}); cursor_=0;
}
Stereo StereoDelay::process(Stereo input) noexcept {
    if(samples_==0) return input;
    const Stereo out=buffer_[cursor_];
    buffer_[cursor_]=input;
    if(++cursor_==samples_) cursor_=0;
    return out;
}

void FIRHalfBand2x::prepare() noexcept {
    // 4-term Blackman-Harris window: low sidelobes, exact halfband zero taps.
    constexpr double a0=.35875,a1=.48829,a2=.14128,a3=.01168;
    constexpr double center=static_cast<double>(kCenter);
    double dc=0.;
    for(std::size_t i=0;i<kTaps;++i) {
        const auto offset=static_cast<double>(i)-center;
        if(i!=kCenter && i%2==0) {h_[i]=0.;continue;}
        const double sinc=(i==kCenter) ? .5 : std::sin(kPi*.5*offset)/(kPi*offset);
        const double phase=2.*kPi*static_cast<double>(i)/static_cast<double>(kTaps-1);
        const double window=a0-a1*std::cos(phase)+a2*std::cos(2.*phase)-a3*std::cos(3.*phase);
        h_[i]=sinc*window;
        dc+=h_[i];
    }
    for(auto& h:h_) h/=dc;
    reset();
}
void FIRHalfBand2x::reset() noexcept {history_.fill({});cursor_=0;}
void FIRHalfBand2x::push(Stereo input) noexcept {
    if(++cursor_==kTaps)cursor_=0;
    history_[cursor_]=input;
    history_[cursor_+kTaps]=input;
}
Stereo FIRHalfBand2x::delayed(std::size_t n) const noexcept {
    return history_[cursor_+kTaps-n];
}
void FIRHalfBand2x::up(Stereo input,Stereo& even,Stereo& odd) noexcept {
    push(input);
    // 2*h[center] is the only nonzero even coefficient.
    const auto middle=delayed(kCenter/2);
    even={2.*h_[kCenter]*middle.l,2.*h_[kCenter]*middle.r};
    odd={};
    for(std::size_t i=1;i<kTaps;i+=2) {
        const auto v=delayed(i/2);
        odd.l+=2.*h_[i]*v.l;
        odd.r+=2.*h_[i]*v.r;
    }
}
Stereo FIRHalfBand2x::down(Stereo even,Stereo odd) noexcept {
    // Emit on the even 2x phase, then retain the odd phase for next input.
    push(even);
    Stereo out{};
    const auto middle=delayed(kCenter);
    out.l=h_[kCenter]*middle.l;
    out.r=h_[kCenter]*middle.r;
    for(std::size_t i=1;i<kTaps;i+=2) {
        const auto v=delayed(i);
        out.l+=h_[i]*v.l;
        out.r+=h_[i]*v.r;
    }
    push(odd);
    return out;
}

void OversamplingPath::prepare(unsigned levels) noexcept {
    levels_=std::min(levels,kMaxLevels);
    for(auto& f:up_)f.prepare();
    for(auto& f:down_)f.prepare();
    const std::size_t nativeDelay=(levels_>=1?40u:0u)+(levels_>=2?20u:0u)+(levels_>=3?10u:0u);
    compensation_.prepare(kLatency-nativeDelay);
}
void OversamplingPath::reset() noexcept {
    for(auto& f:up_)f.reset();
    for(auto& f:down_)f.reset();
    compensation_.reset();
}
Stereo OversamplingPath::process(Stereo input,NonlinearCallback nonlinear,void* ctx) noexcept {
    std::array<Stereo,8> samples{}, expanded{};
    samples[0]=input;
    unsigned count=1;
    for(unsigned level=0;level<levels_;++level) {
        // FIR state is chronological: never iterate the stream backwards to
        // perform an in-place expansion (doing so breaks 4x/8x processing).
        for(unsigned i=0;i<count;++i)
            up_[level].up(samples[i],expanded[2*i],expanded[2*i+1]);
        count*=2;
        for(unsigned i=0;i<count;++i) samples[i]=expanded[i];
    }
    for(unsigned i=0;i<count;++i)
        samples[i]=nonlinear(samples[i],levels_,ctx);
    for(unsigned level=levels_;level>0;--level) {
        for(unsigned i=0;i<count/2;++i)
            samples[i]=down_[level-1].down(samples[2*i],samples[2*i+1]);
        count/=2;
    }
    return compensation_.process(samples[0]);
}

void OversampledProcessor::prepare(double sampleRate) noexcept {
    for(unsigned i=0;i<paths_.size();++i)paths_[i].prepare(i);
    const double fs=clamp(sampleRate,8000.,384000.);
    fadeSamples_=static_cast<std::uint32_t>(clamp(std::round(fs*.012),128.,8192.));
    reset();
}
void OversampledProcessor::reset() noexcept {
    for(auto& p:paths_)p.reset();
    active_=incoming_=requested_;
    warmup_=fadePos_=0;
}
void OversampledProcessor::setMode(OversamplingMode mode) noexcept {
    unsigned idx=static_cast<unsigned>(mode);
    requested_=static_cast<OversamplingMode>(idx<=3?idx:0);
}
void OversampledProcessor::beginTransition(OversamplingMode target) noexcept {
    incoming_=target;
    paths_[static_cast<unsigned>(incoming_)].reset();
    // FIR pipeline delay plus extra settle time before starting the fade.
    warmup_=static_cast<std::uint32_t>(kLatency+64);
    fadePos_=0;
}
Stereo OversampledProcessor::process(Stereo input,NonlinearCallback callback,void* ctx) noexcept {
    if(active_==incoming_ && requested_!=active_)beginTransition(requested_);
    const Stereo outgoing=paths_[static_cast<unsigned>(active_)].process(input,callback,ctx);
    if(incoming_==active_)return outgoing;
    const Stereo entering=paths_[static_cast<unsigned>(incoming_)].process(input,callback,ctx);
    if(warmup_>0) {--warmup_;return outgoing;}
    const double mix=clamp(static_cast<double>(++fadePos_)/static_cast<double>(fadeSamples_),0.,1.);
    const Stereo out{outgoing.l+(entering.l-outgoing.l)*mix,outgoing.r+(entering.r-outgoing.r)*mix};
    if(fadePos_>=fadeSamples_) {active_=incoming_;fadePos_=0;}
    return out;
}
} // namespace steammaster
