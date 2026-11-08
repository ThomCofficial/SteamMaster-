#include "steammaster/DSP.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace steammaster;
namespace {
int checks = 0;
void check(bool ok,const std::string& label) {
    ++checks;
    if (!ok) throw std::runtime_error(label);
}
void finite(Stereo x,const std::string& name) {
    check(std::isfinite(x.l)&&std::isfinite(x.r),name);
}
double measureGain(TptEQ::Mode mode,double hz,double gainDb,double q,double frequency,double fs) {
    TptEQ filter;filter.prepare(fs);filter.configure(mode,hz,q,gainDb);
    double totalInput=0.,totalOut=0.;const int n=static_cast<int>(fs*.4);
    for(int i=0;i<n;++i) {
        double v=.05*std::sin(2.*kPi*frequency*static_cast<double>(i)/fs);
        double o=filter.process({v,v}).l;
        if(i>n/2){totalInput+=v*v;totalOut+=o*o;}
    }
    return std::sqrt(totalOut/totalInput);
}
void checkEQ() {
    const double fs=48000.;
    const double g0=measureGain(TptEQ::Mode::Peak,1000.,0.,1.,1000.,fs);
    check(std::abs(g0-1.)<.003,"neutral parametric EQ must null");
    const double plus6=measureGain(TptEQ::Mode::Peak,1000.,6.,1.,1000.,fs);
    check(plus6>1.85 && plus6<2.15,"+6dB bell calibration");
    const double minus6=measureGain(TptEQ::Mode::Peak,1000.,-6.,1.,1000.,fs);
    check(minus6>.45 && minus6<.58,"-6dB bell calibration");
    const double low=measureGain(TptEQ::Mode::LowShelf,200.,5.,.7,45.,fs);
    check(low>1.60 && low<2.0,"+5dB low-shelf calibration");
    const double high=measureGain(TptEQ::Mode::HighShelf,4000.,5.,.7,12000.,fs);
    check(high>1.5 && high<2.,"+5dB high-shelf calibration");
}
void checkCompressor() {
    Furnace f;f.prepare(48000.);
    FurnaceParameters p; p.thresholdDb=-15.;p.ratio=FurnaceRatio::R4;
    p.attack=FurnaceAttack::Ms10;p.release=FurnaceRelease::S0_3;
    f.set(p);
    Stereo last{};
    for (int i=0;i<96000;++i)last=f.process({.7,.35});
    check(f.gainReductionDb()>4.,"Furnace actually reduces gain");
    check(std::abs(last.l-2.*last.r)<1e-5,"Furnace stereo link preserves channel ratio");
    f.reset();p.thresholdDb=15.;f.set(p);
    for(int i=0;i<96000;++i) last=f.process({.3,.15});
    check(f.gainReductionDb()<.3,"Furnace positive threshold no excessive compression");
    Ocean o;o.prepare(48000.);
    OceanParameters op;op.thresholdDb=-25.;op.ratio=OceanRatio::R6;
    o.set(op);
    for(int i=0;i<96000;++i) last=o.process({.8,.4});
    check(o.gainReductionDb()>3.,"Ocean actually reduces gain");
    Shadow s;s.prepare(48000.);
    ShadowParameters sp;sp.opticalThresholdDb=-25.;sp.discreteThresholdDb=-25.;
    s.set(sp);
    for(int i=0;i<96000;++i) last=s.process({.65,.325});
    check(s.opticalReductionDb()>1.,"Shadow optical stage active");
    check(s.discreteReductionDb()>1.,"Shadow discrete stage active");
    check(std::abs(last.l-2.*last.r)<1e-4,"Shadow stereo link preserves channel ratio");
    sp.opticalIn=false;sp.discreteIn=false;s.set(sp);
    for(int i=0;i<4800;++i) last=s.process({.35,.175});
    check(std::abs(last.l-.35)<1e-5,"Shadow independent stages fully bypass");
}
void checkPultecAndExciter() {
    Pultec p;p.prepare(48000.);PultecParameters pp;
    pp.lowBoost=7.;pp.lowAttenuation=5.;pp.highBoost=4.;pp.highAttenuation=3.;p.set(pp);
    Stereo last{};
    for(int i=0;i<48000;++i){const double v=.1*std::sin(2.*kPi*80.*i/48000.);last=p.process({v,v});finite(last,"Pultec finite");}
    Exciter e;e.prepare(48000.);ExciterParameters ep;
    ep.lowDrive=8.;ep.highDrive=8.;e.set(ep);
    double dry=0.,wet=0.;
    for(int i=0;i<48000;++i){
        const double x=.22*std::sin(2.*kPi*180.*i/48000.)+.14*std::sin(2.*kPi*7500.*i/48000.);
        last=e.process({x,x});
        finite(last,"Exciter finite");
        if(i>24000){dry+=x*x;wet+=(last.l-x)*(last.l-x);}
    }
    check(wet > dry*.00001,"Exciter drive produces real harmonic difference");
    ep.lowDrive=0.;ep.highDrive=0.;e.set(ep);e.reset();
    for(int i=0;i<8000;++i){auto x=.3*std::sin(2.*kPi*200.*i/48000.);last=e.process({x,x});}
    check(std::abs(last.l-last.r)<1.e-8,"Exciter stereo stable");
}
void checkBypassClick() {
    Furnace f;f.prepare(48000.);FurnaceParameters p;
    p.thresholdDb=-15.;f.set(p);
    for (int i=0;i<48000;++i) f.process({.25,.25});
    p.common.in=false;f.set(p);
    double prev=f.process({.25,.25}).l,maxJump=0.;
    for(int i=0;i<1600;++i){const auto v=f.process({.25,.25}).l;maxJump=std::max(maxJump,std::abs(v-prev));prev=v;}
    check(maxJump<.03,"Furnace bypass crossfade discontinuity");
}
void checkRackSafety() {
    for(double fs: {44100.,48000.,88200.,96000.,192000.}) {
        SixModuleRack rack;rack.prepare(fs);
        // Full-scale, stereo phase inversion, silence and impulse patterns.
        for(int i=0;i<static_cast<int>(fs*.12);++i) {
            const double x=(i%31==0) ? 1. : (i%2==0 ? .7 : -.7);
            const auto y=rack.process({x,-x});
            check(std::isfinite(y.l)&&std::isfinite(y.r),"Rack sample-rate stability");
            check(std::abs(y.l)<=4.00001 && std::abs(y.r)<=4.00001,"Emergency containment");
        }
        rack.reset();
        for(int i=0;i<256;++i){auto y=rack.process({0.,0.});finite(y,"Rack silence");}
        auto y=rack.process({std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()});
        finite(y,"Rack NaN and Inf containment");
        check(rack.nonFiniteEvents()>=2,"Nonfinite event telemetry");
    }
}
void checkAutomation() {
    SixModuleRack rack;rack.prepare(48000.);
    FurnaceParameters fp;OceanParameters op;ExciterParameters ep;ShadowParameters sp;
    ParametricParameters eq;
    for(auto& b:eq.bands){b.frequencyHz=1300.;b.gainDb=0.;b.q=.7;}
    PultecParameters pp;
    Stereo prev{};double maxJump=0.;
    for(int i=0;i<96000;++i) {
        if(i%97==0) {
            fp.common.in=(i%2==0);fp.thresholdDb=i%3==0?-15.:15.;
            fp.attack=i%5==0?FurnaceAttack::Ms0_1:FurnaceAttack::Ms30;
            fp.release=i%7==0?FurnaceRelease::Auto:FurnaceRelease::S0_1;
            rack.furnace.set(fp);
            op.thresholdDb=i%5==0?-30.:-6.;op.ratio=i%2?OceanRatio::R1_2:OceanRatio::R6;
            rack.ocean.set(op);
            ep.lowDrive=(i%3==0)?10.:0.;ep.highDrive=(i%5==0)?10.:0.;
            ep.mode=static_cast<ExciterMode>(i%3);ep.common.in=(i%11!=0);
            rack.exciter.set(ep);
            sp.opticalIn=i%3!=0;sp.discreteIn=i%5!=0;
            sp.discreteRatio=i%7==0?DiscreteRatio::Flood20:DiscreteRatio::R1_2;
            rack.shadow.set(sp);
            eq.bands[0].gainDb=(i%5==0)?12.:-12.;
            eq.bands[0].frequencyHz=(i%4==0)?20.:10000.;
            eq.bands[0].q=(i%3==0)?5.:.3;
            rack.parametric.set(eq);
            pp.lowBoost=static_cast<double>(i%11);pp.highBoost=static_cast<double>(i%11);
            pp.lowAttenuation=static_cast<double>((i+5)%11);
            rack.pultec.set(pp);
        }
        const auto x=.12*std::sin(2.*kPi*400.*i/48000.);
        auto y=rack.process({x,-x*.7});
        finite(y,"Fast automation finite");
        check(std::abs(y.l)<=4.0 && std::abs(y.r)<=4.0,"Rapid automation guard");
        if(i>200) maxJump=std::max(maxJump,std::abs(y.l-prev.l));
        prev=y;
    }
    check(maxJump<.8,"Fast automation no catastrophic discrete jumps");
}
void benchmark() {
    SixModuleRack rack;rack.prepare(48000.);
    const int samples=48000*2;
    volatile double sink=0.;
    const auto start=std::chrono::steady_clock::now();
    for(int i=0;i<samples;++i) {
        double t=static_cast<double>(i)/48000.;
        Stereo x{.12*std::sin(2.*kPi*200.*t),.13*std::sin(2.*kPi*1700.*t)};
        auto y=rack.process(x);sink+=y.l*.000001;
    }
    const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::cout << "Benchmark: 2s stereo at 48k rendered in " << elapsed << "s; RTF="<<elapsed/2.<<" (machine-specific; not Cubase)\n";
    (void)sink;
}
}
int main() {
    try {
        checkEQ();std::cout<<"[PASS] EQ transfer/calibration\n";
        checkCompressor();std::cout<<"[PASS] Compressor stages and stereo link\n";
        checkPultecAndExciter();std::cout<<"[PASS] Pultec and exciter processing\n";
        checkBypassClick();std::cout<<"[PASS] Bypass smoothing\n";
        checkRackSafety();std::cout<<"[PASS] Sample-rate safety and non-finite input\n";
        checkAutomation();std::cout<<"[PASS] Fast automation stress\n";
        benchmark();
        std::cout << "ALL PASS: " << checks << " assertions\n";
        return 0;
    } catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<" at assertion "<<checks<<"\n";return 1;}
}
