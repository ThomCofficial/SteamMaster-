#include "steammaster/DSP.hpp"
#include "steammaster/Oversampling.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace steammaster;
namespace {
std::uint64_t assertions=0;
void require(bool x,const char* message){++assertions;if(!x)throw std::runtime_error(message);}
Stereo linear(Stereo v,unsigned,void*) noexcept {return v;}
Stereo tanhClip(Stereo v,unsigned,void*) noexcept {
    return {std::tanh(4.*v.l)/std::tanh(4.),std::tanh(4.*v.r)/std::tanh(4.)};
}
Stereo unsafe(Stereo v,unsigned,void*) noexcept {
    // Max output bounded by input, useful to stress FIR/interpolator gain.
    return {std::atan(6.*v.l)/std::atan(6.),std::atan(6.*v.r)/std::atan(6.)};
}
void latencyAndDC() {
    constexpr int size=2048;
    for(unsigned factor=0;factor<4;++factor) {
        OversamplingPath p;p.prepare(factor);
        int best=-1;
        double maxAmp=0.,sum=0.;
        for(int i=0;i<size;++i) {
            const auto out=p.process({i==0?1.:0.,i==0?-.35:0.},&linear,nullptr);
            sum+=out.l;
            require(std::isfinite(out.l) && std::isfinite(out.r),"FIR impulse finite");
            require(std::abs(out.l+out.r/.35)<1e-10,"Stereo independent linear gain");
            if(std::abs(out.l)>maxAmp){maxAmp=std::abs(out.l);best=i;}
        }
        require(best==static_cast<int>(OversamplingPath::kLatency),"All FIR rates share fixed peak latency");
        require(std::abs(sum-1.)<.0005,"All FIR rates preserve unit DC gain");
        p.reset();
        double avg=0.;
        for(int i=0;i<4000;++i){const double v=p.process({.2,.1},&linear,nullptr).l;if(i>512)avg+=v;}
        avg/=static_cast<double>(4000-513);
        require(std::abs(avg-.2)<.0002,"FIR steady-state passband gain");
        std::cout<<"FIR "<<(1u<<factor)<<"x: impulse peak at "<<best<<", DC="<<sum<<"\n";
    }
}
// Coherent tone magnitude after discarding pipeline startup.
double toneMagnitude(const std::vector<double>& input,double hz,double fs,std::size_t skip){
    double re=0.,im=0.;
    for(std::size_t i=skip;i<input.size();++i){
        const double phase=2.*kPi*hz*static_cast<double>(i)/fs;
        re+=input[i]*std::cos(phase);im-=input[i]*std::sin(phase);
    }
    const auto n=static_cast<double>(input.size()-skip);
    return 2.*std::sqrt(re*re+im*im)/n;
}
void checkAlias() {
    constexpr double fs=48000.,signal=9000.,alias=21000.;
    constexpr std::size_t n=48000;
    std::vector<double> base(n),high(n);
    for(unsigned quality:{0u,1u,2u,3u}) {
        OversamplingPath p;p.prepare(quality);
        for(std::size_t i=0;i<n;++i){
            const double value=.75*std::sin(2.*kPi*signal*static_cast<double>(i)/fs);
            const double y=p.process({value,value},&tanhClip,nullptr).l;
            require(std::isfinite(y) && std::abs(y)<3.,"Nonlinear stage output bounded");
            if(quality==0)base[i]=y;
            else high[i]=y;
        }
        if(quality>0)
            require(toneMagnitude(high,signal,fs,4096)>.95*toneMagnitude(base,signal,fs,4096),
                    "No unexpected nonlinear fundamental loss at 2x/4x/8x");
    }
    const double fundamentalBase=toneMagnitude(base,signal,fs,4096);
    const double fundamentalHigh=toneMagnitude(high,signal,fs,4096);
    const double aliasBase=toneMagnitude(base,alias,fs,4096);
    const double aliasHigh=toneMagnitude(high,alias,fs,4096);
    const double rejection=20.*std::log10(aliasBase/std::max(1e-12,aliasHigh));
    std::cout<<"Alias 9 kHz nonlinear at 48 kHz: OFF "<<aliasBase
             <<" 8x "<<aliasHigh<<" attenuation "<<rejection<<" dB\n";
    require(fundamentalBase>.15 && fundamentalHigh>.15,"Oversampling fundamental preserved");
    require(rejection>20.,"8x bandlimiting reduces 21k alias by >20dB");
}
void checkSafeTransitions() {
    OversampledProcessor p;p.prepare(48000.);
    constexpr int n=96000;
    double maxDelta=0.,prev=0.,peak=0.;
    constexpr std::array<OversamplingMode,4> modes{OversamplingMode::Off,OversamplingMode::X2,OversamplingMode::X4,OversamplingMode::X8};
    for(int i=0;i<n;++i) {
        if(i%133==0) p.setMode(modes[(i/133)%4]);
        // Fixed nonzero DC is a worst case for control-switch discontinuities.
        const double v=p.process({.37,-.24},&unsafe,nullptr).l;
        require(std::isfinite(v),"Rapid crossfade finite");
        peak=std::max(peak,std::abs(v));
        if(i>800)maxDelta=std::max(maxDelta,std::abs(v-prev));
        prev=v;
    }
    std::cout<<"Rapid mode changes: peak "<<peak<<", maximum step "<<maxDelta<<"\n";
    require(maxDelta<.025,"Continuous oversampling-rate automation without clicks");
    require(peak<1.,"Crossfade does not create dangerous amplitudes");
    p.setMode(OversamplingMode::X8);
    for(int i=0;i<8000;++i)p.process({.37,-.24},&unsafe,nullptr);
    require(p.activeMode()==OversamplingMode::X8,"Latest requested oversampling eventually applied");
    p.reset();
    require(p.activeMode()==OversamplingMode::X8,"Reset preserves user-selected quality");
}
void checkRackLatencyAndSafety() {
    for(const double fs:{44100.,48000.,88200.,96000.,192000.}) {
        auto rack=std::make_unique<SixModuleRack>();rack->prepare(fs);
        OceanParameters ocean{};ocean.common.in=false;rack->ocean.set(ocean);
        FurnaceParameters furnace{};furnace.common.in=false;rack->furnace.set(furnace);
        ParametricParameters eq{};eq.common.in=false;rack->parametric.set(eq);
        PultecParameters pultec{};pultec.common.in=false;rack->pultec.set(pultec);
        ShadowParameters shadow{};shadow.common.in=false;rack->shadow.set(shadow);
        ExciterParameters exciter{};exciter.common.in=false;rack->exciter.set(exciter);
        for(auto mode:{OversamplingMode::Off,OversamplingMode::X2,OversamplingMode::X4,OversamplingMode::X8}) {
            rack->setOversamplingMode(mode);
            for(int i=0;i<12000;++i)rack->process({0.,0.}); // settle bypass ramps
            rack->reset();
            const int n=800;
            int peakSample=-1;double peak=0.,atPeakRight=0.;
            for(int i=0;i<n;++i){
                const auto y=rack->process({i==0?.45:0.,i==0?-.2:0.});
                require(std::isfinite(y.l)&&std::isfinite(y.r),"Rack bypass finite");
                if(std::abs(y.l)>peak){peak=std::abs(y.l);peakSample=i;atPeakRight=y.r;}
            }
            require(peakSample==static_cast<int>(SixModuleRack::latencySamples()),"Rack bypass impulse has exact reported latency");
            require(std::abs(peak-.45)<.0001,"Rack bypass preserves input level");
            require(std::abs(atPeakRight+.2)<.0001,"Rack bypass preserves independent stereo level");
        }
        rack->reset();rack->setOversamplingMode(OversamplingMode::X8);
        for(int i=0;i<static_cast<int>(fs*.11);++i){
            const double t=i/fs;
            const auto y=rack->process({.8*std::sin(2.*kPi*13000.*t),-.85*std::sin(2.*kPi*9000.*t)});
            require(std::isfinite(y.l)&&std::isfinite(y.r),"Rack 8x finite at all rates");
            require(std::abs(y.l)<=4. && std::abs(y.r)<=4.,"Rack guard preserved at all rates");
        }
    }
    require(SixModuleRack::latencySamples()==140,"Shared nonlinear path latency reported as 140 samples");
}
} // namespace
int main() {
    try {
        latencyAndDC();
        checkAlias();
        checkSafeTransitions();
        checkRackLatencyAndSafety();
        std::cout<<"PASS: "<<assertions<<" oversampling and safety assertions\n";
        return 0;
    } catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<" at assertion "<<assertions<<"\n";return 1;}
}
