#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace steammaster;
namespace {
std::uint64_t nTests=0;
void check(bool ok,const char* message){++nTests;if(!ok)throw std::runtime_error(message);}
bool finite(Stereo s){return std::isfinite(s.l)&&std::isfinite(s.r);}
void baxTests(double fs) {
    Baxandall eq;eq.prepare(fs);
    for(int i=0;i<600;++i){const auto x=Stereo{.2*std::sin(2*kPi*i*110./fs),.1*std::sin(2*kPi*i*234./fs)};const auto y=eq.process(x);check(finite(y),"bax neutral finite");if(i>500)check(std::abs(y.l-x.l)<1.e-6&&std::abs(y.r-x.r)<1.e-6,"bax neutral unity");}
    BaxandallParameters p;
    p.rows[0].lowGainDb=5.;p.rows[0].outputDb=2.;p.rows[1].lowGainDb=-5.;
    eq.set(p);
    double left=0.,right=0.;
    for(int i=0;i<static_cast<int>(fs*.12);++i){auto x=eq.process({.1,.1});check(finite(x),"bax flat finite");left=x.l;right=x.r;}
    check(std::abs(left-right)<.01,"bax mono MID row remains mono");
    p.routing=BaxRouting::LeftRight;eq.set(p);
    for(int i=0;i<static_cast<int>(fs*.12);++i){auto x=eq.process({.1,.1});left=x.l;right=x.r;}
    check(left>right*1.3,"bax LR independent row processing");
    for(unsigned h=0;h<8;++h)for(unsigned l=0;l<8;++l){
        p.rows[0].hpf=static_cast<BaxHPF>(h);p.rows[0].lpf=static_cast<BaxLPF>(l);
        p.rows[0].highFrequency=static_cast<BaxHighShelf>(l);p.rows[0].lowFrequency=static_cast<BaxLowShelf>(h);
        p.rows[0].lowGainDb=h%2==0?5.:-5.;p.rows[0].highGainDb=l%2==0?5.:-5.;
        eq.set(p);
        for(int i=0;i<80;++i){auto x=eq.process({i%2==0?.7:-.7,.7});check(finite(x)&&std::abs(x.l)<20.,"bax all switches stable");}
    }
    p.rows[0].in=false;p.rows[1].in=false;eq.set(p);
    for(int i=0;i<static_cast<int>(fs*.16);++i){auto x=eq.process({.1,-.3});if(i>static_cast<int>(fs*.14))check(std::abs(x.l-.1)<1e-5&&std::abs(x.r+.3)<1e-5,"bax bypass independent unity");}
}
void tapeTests(double fs){
    Tape t;t.prepare(fs);TapeParameters p;
    p.common.in=false;t.set(p);
    for(int i=0;i<static_cast<int>(fs*.16);++i){const auto x=Stereo{.15,-.2};const auto y=t.process(x);check(finite(y),"tape bypass finite");if(i>static_cast<int>(fs*.14))check(std::abs(y.l-.15)<.00001,"tape bypass wet latency aligned");}
    t.reset(); // prior bypass input was intentionally asymmetric
    p.common.in=true;p.common.mix=1.;p.driveDb=8.;p.bias=4.;p.hfTrimDb=6.;
    for(unsigned type=0;type<3;++type){p.type=static_cast<TapeType>(type);for(unsigned speed=0;speed<3;++speed){p.speed=static_cast<TapeSpeed>(speed);t.set(p);for(unsigned os=0;os<4;++os){t.setOversamplingMode(static_cast<OversamplingMode>(os));for(int i=0;i<1000;++i){const double x=.75*std::sin(2*kPi*997.*i/fs);auto y=t.process({x,x});check(finite(y)&&std::abs(y.l)<3.5,"tape worst-case finite");check(std::abs(y.l-y.r)<1e-10,"tape preserves symmetric stereo");}}}}
}
void globalStereoTests(double fs){
    GlobalStereo s;s.prepare(fs);GlobalParameters p;
    p.stereoWidth=2.;p.widthFrequencyHz=800.;p.monoMakerHz=200.;s.set(p);
    double bassSide=0.,trebleSide=0.;
    const int n=static_cast<int>(fs*.5);
    for(int i=0;i<n;++i){const double low=.2*std::sin(2*kPi*45.*i/fs);const double high=.2*std::sin(2*kPi*4000.*i/fs);
        auto y=s.process({low+high,-low-high});check(finite(y),"global stereo finite");if(i>n/2){bassSide+=(y.l-y.r)*std::sin(2*kPi*45.*i/fs);trebleSide+=(y.l-y.r)*std::sin(2*kPi*4000.*i/fs);}}
    bassSide=std::abs(bassSide)/(n/2);trebleSide=std::abs(trebleSide)/(n/2);
    check(bassSide<.015,"mono maker removes deep side bass at width 200%");
    check(trebleSide>.28,"stereo widener retains wide high side");
    p.stereoWidth=1.;p.monoMakerHz=0.;s.set(p);
    for(int i=0;i<static_cast<int>(fs*.25);++i){auto x=s.process({.2,-.1});if(i>static_cast<int>(fs*.19))check(std::abs(x.l-.2)<.0001&&std::abs(x.r+.1)<.0001,"neutral stereo exact");}
}
void limiterTests(double fs){
    MasterLimiter lim;lim.prepare(fs);LimiterParameters p;
    p.thresholdDb=-12.;p.ceilingDbTP=-1.;p.lookaheadMs=5.;p.releaseMs=30.;p.truePeak=true;lim.set(p);
    check(lim.latencySamples()==35u+static_cast<size_t>(std::ceil(fs*.005)),"limiter fixed maximum latency");
    double maxSignal=0.,gr=0.,tp=0.;
    for(int i=0;i<static_cast<int>(fs*.18);++i){const double wave=.9*std::sin(2*kPi*1700.*i/fs);auto x=lim.process({wave,wave});check(finite(x),"limiter signal finite");maxSignal=std::max(maxSignal,std::abs(x.l));gr=std::max(gr,lim.gainReductionDb().l);tp=std::max(tp,lim.measuredOutputTruePeak());}
    check(maxSignal<=dbToGain(-1.)+1e-9,"limiter enforces sample ceiling on steady tone");
    check(gr>3.,"limiter actually applies gain reduction");
    check(tp<1.1,"limiter true-peak telemetry finite and bounded");
    // The FIR reconstruction true-peak limit is under development, deliberately
    // NOT certified as a hard ceiling under worst-case intersample transients.
    p.truePeak=false;p.stereoLink=0.;p.lookaheadMs=.1;p.autoRelease=true;p.mode=LimiterMode::Punch;lim.set(p);
    for(int i=0;i<static_cast<int>(fs*.05);++i){auto x=lim.process({2.0,0.1});check(finite(x)&&std::abs(x.l)<1.1,"limiter extreme toggle safe");}
    p.in=false;lim.set(p);
    for(int i=0;i<static_cast<int>(fs*.15);++i){auto y=lim.process({.25,-.12});check(finite(y),"limiter bypass stable");if(i>static_cast<int>(fs*.13))check(std::abs(y.l-.25)<.0001&&std::abs(y.r+.12)<.0001,"limiter bypass delayed dry");}
}
void masterRackTests(double fs){
    MasteringRack rack;rack.prepare(fs);
    GlobalParameters global;global.in=false;global.inputDb=18.;global.outputDb=12.;global.oversampling=OversamplingMode::X8;rack.setGlobal(global);
    const size_t latency=rack.latencySamples();check(latency<4096,"rack total latency representable");
    for(int i=0;i<static_cast<int>(fs*.10);++i) rack.process({}); // let bypass ramp settle before impulse
    double highest=0.;int peakAt=-1;
    for(size_t i=0;i<latency+500;++i){auto y=rack.process({i==0?.4:0.,i==0?-.4:0.});check(finite(y),"mastering rack bypass finite");if(std::abs(y.l)>highest){highest=std::abs(y.l);peakAt=static_cast<int>(i);}}
    check(peakAt==static_cast<int>(latency),"global bypass equals fixed actual chain latency");
    std::cout<<"  bypass peak "<<highest<<" at "<<peakAt<<" (latency "<<latency<<")\n";
    check(std::abs(highest-.4)<1.e-7,"global bypass does not change level");
    global.in=true;global.monoMakerHz=150.;global.stereoWidth=1.8;rack.setGlobal(global);
    for(int i=0;i<8000;++i){
        if(i%337==0){global.oversampling=static_cast<OversamplingMode>((i/337)%4);global.outputDb=(i/337)%2==0?12.:-24.;rack.setGlobal(global);}
        const double l=i%199==0?2.:-.5,r=i%29==0?-2.:.6;
        auto y=rack.process({l,r});check(finite(y)&&std::abs(y.l)<=4.&&std::abs(y.r)<=4.,"mastering rack rapid parameter changes contained");
    }
    rack.reset();for(int i=0;i<100;++i){auto y=rack.process({std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()});check(finite(y),"rack nonfinite containment");}
    check(rack.invalidEvents()>0,"rack nonfinite telemetry works");
}
}
int main(){try{
    for(double fs:{44100.,48000.,88200.,96000.,192000.}){
        std::cout<<"Full mastering regression @ "<<fs<<" Hz\n";
        baxTests(fs);tapeTests(fs);globalStereoTests(fs);limiterTests(fs);masterRackTests(fs);
    }
    std::cout<<"PASS "<<nTests<<" full-chain assertions\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<" after "<<nTests<<" checks\n";return 1;}}
