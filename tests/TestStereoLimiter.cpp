#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace steammaster;
namespace {
std::uint64_t assertions=0;
void expect(bool good,const char* why) { ++assertions; if(!good)throw std::runtime_error(why); }
struct Ref8x {
    FIRHalfBand2x a{},b{},c{};
    Ref8x(){a.prepare();b.prepare();c.prepare();}
    double process(Stereo x) noexcept {
        Stereo v[2]{},w[4]{},u[2]{};
        a.up(x,v[0],v[1]);
        for(int i=0;i<2;++i)b.up(v[i],w[i*2],w[i*2+1]);
        double peak=std::max(std::abs(x.l),std::abs(x.r));
        for(auto q:w){c.up(q,u[0],u[1]);for(auto z:u)peak=std::max({peak,std::abs(z.l),std::abs(z.r)});}
        return peak;
    }
};
// Coherent side-signal gain, measured after all smoothing, excluding startup.
double sideGain(GlobalStereo& processor,double fs,double hz) {
    const int n=static_cast<int>(fs*.20);
    double num=0.,den=0.;
    for(int i=0;i<n;++i){
        const double s=.12*std::sin(2*kPi*hz*i/fs);
        const auto y=processor.process({s,-s});
        expect(std::isfinite(y.l)&&std::isfinite(y.r),"Nonfinite stereo width response");
        if(i>n/2){num+=((y.l-y.r)*.5)*s;den+=s*s;}
    }
    return num/den;
}
void stereoTests(double fs){
    GlobalStereo processor;processor.prepare(fs);
    GlobalParameters p;processor.set(p);
    for(int i=0;i<2200;++i){
        const Stereo s{.3*std::sin(2*kPi*71*i/fs),.1*std::cos(2*kPi*2007*i/fs)};
        auto y=processor.process(s);
        expect(std::abs(y.l-s.l)<1e-12 && std::abs(y.r-s.r)<1e-12,"Neutral stereo is not sample exact");
    }
    p.stereoWidth=2.;p.widthFrequencyHz=800.;processor.set(p);
    const double bass=sideGain(processor,fs,40.);
    const double high=sideGain(processor,fs,4000.);
    std::cout<<fs<<" Hz widener 40Hz="<<bass<<", 4kHz="<<high<<"\n";
    expect(bass>.96&&bass<1.03,"High-band widener changed subbass width");
    expect(high>1.75&&high<2.08,"High-band widening not effective");
    p.monoMakerHz=200.;processor.set(p);
    const double monoBass=std::abs(sideGain(processor,fs,40.));
    const double monoHigh=sideGain(processor,fs,4000.);
    expect(monoBass<.065,"Mono Maker failed to remove subbass side");
    expect(monoHigh>1.70,"Mono Maker damaged treble side");
    // Switching Width cannot reintroduce side bass that Mono Maker removed.
    p.stereoWidth=1.;processor.set(p);
    const double w100=std::abs(sideGain(processor,fs,40.));
    p.stereoWidth=2.;processor.set(p);
    const double w200=std::abs(sideGain(processor,fs,40.));
    expect(w200 < w100*1.08+1e-6,"Widening reintroduced LF side into Mono Maker");
    // Mid-sum remains exact at all filter states (and correlation stays valid).
    for(int i=0;i<6000;++i){
        if(i%257==0){p.stereoWidth=(i%3==0)?0.:(i%2==0?2.:1.);
            p.widthFrequencyHz=(i%2==0)?5000.:800.;p.monoMakerHz=(i%3==0)?0.:300.;processor.set(p);}
        const Stereo x{.23*std::sin(2*kPi*1337*i/fs)+.12,.2*std::cos(2*kPi*337*i/fs)-.11};
        const auto y=processor.process(x);
        expect(std::isfinite(y.l)&&std::isfinite(y.r),"Stereo automation generated nonfinite values");
        expect(std::abs((y.l+y.r)-(x.l+x.r))<1.e-12,"Stereo image altered mid/sum signal");
        expect(std::abs(y.l)<2.&&std::abs(y.r)<2.,"Stereo automation created peak artifact");
    }
    processor.reset();p.stereoWidth=1.7;p.monoMakerHz=250.;processor.set(p);
    for(int i=0;i<static_cast<int>(fs*.08);++i){
        auto y=processor.process({.23,.23});
        expect(std::abs(y.l-y.r)<1e-12,"Mono input creates artificial side energy");
    }
    expect(processor.correlation()>.999,"Correlation diagnostic did not recognize mono input");
}
void limiterModeTests(double fs){
    std::array<double,3> releases{};
    for(int mode=0;mode<3;++mode){
        MasterLimiter lim;lim.prepare(fs);
        LimiterParameters p;p.autoRelease=true;p.thresholdDb=-15.;p.ceilingDbTP=-1.;
        p.mode=static_cast<LimiterMode>(mode);p.lookaheadMs=5.;lim.set(p);
        // A brief peak should release faster than sustained GR in AUTO.
        for(int i=0;i<static_cast<int>(fs*.006);++i)lim.process({1.3,1.3});
        const double brief=lim.effectiveReleaseMs().l;
        for(int i=0;i<static_cast<int>(fs*.19);++i)lim.process({1.3,1.3});
        const double sustained=lim.effectiveReleaseMs().l;
        releases[static_cast<std::size_t>(mode)]=sustained;
        std::cout<<fs<<" Hz limiter mode="<<mode<<" AUTO "<<brief<<"ms transient, "<<sustained<<"ms sustained\n";
        expect(sustained>brief+70.,"AUTO release not program dependent");
        expect(sustained>100.&&sustained<1800.,"AUTO release out of bounds");
        const auto gr=lim.gainReductionDb();
        expect(gr.l>1.&&std::abs(gr.l-gr.r)<.002,"Linked limiter gain reduction invalid");
    }
    expect(releases[1]<releases[0]*.8,"Punch voice not distinct from Clean");
    expect(releases[2]>releases[1]*1.08,"Modern voice not distinct from Punch");
}
void modeAudioDistinction() {
    constexpr double fs=48000.;
    // The limiter owns multiple lookahead/detector rings; avoid exceeding
    // the smaller default Windows test-executable thread stack.
    std::array<std::unique_ptr<MasterLimiter>,3> engines{};
    for(int mode=0;mode<3;++mode) {
        engines[static_cast<std::size_t>(mode)]=std::make_unique<MasterLimiter>();
        auto& lim=*engines[static_cast<std::size_t>(mode)];
        lim.prepare(fs);
        LimiterParameters p;p.autoRelease=true;p.thresholdDb=-12.;p.ceilingDbTP=-1.;
        p.lookaheadMs=5.;p.mode=static_cast<LimiterMode>(mode);lim.set(p);
    }
    double cleanVsPunch=0.,cleanVsModern=0.;
    for(int i=0;i<48000;++i) {
        const double volume=i%6000<1000?1.25:.25;
        const double wave=volume*std::sin(2*kPi*997.*i/fs);
        std::array<double,3> y{};
        for(int mode=0;mode<3;++mode)
            y[static_cast<std::size_t>(mode)]=engines[static_cast<std::size_t>(mode)]->process({wave,wave}).l;
        if(i>=8000) {
            cleanVsPunch+=std::abs(y[0]-y[1]);
            cleanVsModern+=std::abs(y[0]-y[2]);
        }
    }
    cleanVsPunch/=40000.;cleanVsModern/=40000.;
    expect(cleanVsPunch>.005,"Punch sonic envelope identical to Clean");
    expect(cleanVsModern>.0005,"Modern sonic envelope identical to Clean");
    std::cout<<"Mode waveform mean absolute deltas Clean/Punch="<<cleanVsPunch
             <<", Clean/Modern="<<cleanVsModern<<"\n";
}
void truePeakTests(double fs){
    for(int mode=0;mode<3;++mode){
        MasterLimiter lim;lim.prepare(fs);
        LimiterParameters p;p.mode=static_cast<LimiterMode>(mode);p.autoRelease=true;
        p.truePeak=true;p.lookaheadMs=.1;p.stereoLink=0.;p.thresholdDb=-15.;p.ceilingDbTP=-1.;lim.set(p);
        Ref8x ref;
        double peak=0.;
        const int n=static_cast<int>(fs*.032);
        for(int i=0;i<n;++i){
            const double impulse=(i%173==7)?3.5:0.;
            const double a=1.5*std::sin(2.*kPi*10777.*i/fs)+impulse;
            const double b=.8*std::sin(2.*kPi*6611.*i/fs+.3)-impulse*.7;
            const auto y=lim.process({a,b});
            expect(std::isfinite(y.l)&&std::isfinite(y.r),"Limiter emitted nonfinite output");
            peak=std::max(peak,ref.process(y));
        }
        const double db=gainToDb(peak);
        std::cout<<fs<<" Hz mode="<<mode<<" measured 8x="<<db<<" dBTP, emergency="<<lim.safetyClipEvents()<<"\n";
        expect(db<=-.93,"Limiter AUTO mode true peak ceiling regression");
        expect(lim.safetyClipEvents()<static_cast<std::uint64_t>(n/5),"Limiter relying excessively on hard clipping");
    }
}
} // namespace
int main(){try{
    for(double fs:{44100.,48000.,96000.,192000.})stereoTests(fs);
    modeAudioDistinction();
    for(double fs:{48000.,96000.}){limiterModeTests(fs);truePeakTests(fs);}
    std::cout<<"PASS Stereo/Limiter "<<assertions<<" assertions\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL after "<<assertions<<": "<<e.what()<<"\n";return 1;}}
