#include "steammaster/Mastering.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <algorithm>
using namespace steammaster;
namespace {
int assertions=0;
void check(bool cond,const char* what) {
    ++assertions;
    if(!cond){std::fprintf(stderr,"BAX FAIL: %s\n",what);std::exit(1);}
}
// Synchronous complex tone measurement; checks the *actual running DSP*.
// Reference input and output use identical windows to suppress leakage bias.
double gainAt(double fs,double freq,BaxandallParameters p) {
    Baxandall bax;bax.prepare(fs);bax.set(p);
    const int nWarm=static_cast<int>(fs*.35);
    const int nMeasure=static_cast<int>(fs*.20);
    double iy=0.,qy=0.,ix=0.,qx=0.;
    const double step=2.*kPi*freq/fs;
    for(int n=0;n<nWarm+nMeasure;++n){
        const double angle=step*static_cast<double>(n);
        const double sn=std::sin(angle),cs=std::cos(angle);
        const double sample=.25*sn;
        const auto out=bax.process({sample,sample});
        if(n>=nWarm){iy+=out.l*sn;qy+=out.l*cs;ix+=sample*sn;qx+=sample*cs;}
    }
    const double amplitude=std::hypot(iy,qy)/std::max(1.e-20,std::hypot(ix,qx));
    return 20.*std::log10(std::max(amplitude,1.e-12));
}
BaxandallParameters stereo() {
    BaxandallParameters p; p.routing=BaxRouting::LeftRight; return p;
}
void checkNeutral(double fs){
    Baxandall eq;eq.prepare(fs);
    auto p=stereo();eq.set(p);
    for(int i=0;i<12000;++i){
        const double a=.5*std::sin(2.*kPi*89.*i/fs),b=.31*std::cos(2.*kPi*987.*i/fs);
        const Stereo y=eq.process({a,b});
        check(std::abs(y.l-a)<1.e-10&&std::abs(y.r-b)<1.e-10,"unity response must be sample exact in LR");
    }
    p.routing=BaxRouting::MidSide;eq.set(p);
    for(int i=0;i<12000;++i){
        const double a=.5*std::sin(2.*kPi*89.*i/fs),b=.31*std::cos(2.*kPi*987.*i/fs);
        const Stereo y=eq.process({a,b});
        check(std::abs(y.l-a)<1.e-10&&std::abs(y.r-b)<1.e-10,"unity response must be sample exact during routing crossfade");
    }
}
void checkShelves(double fs) {
    auto p=stereo();
    p.rows[0].lowFrequency=BaxLowShelf::Hz74;p.rows[0].lowGainDb=5.;
    const double bass=gainAt(fs,20.,p),mid=gainAt(fs,1200.,p);
    check(bass>4.4&&bass<5.2,"low +5 shelf should approach +5 at 20 Hz");
    check(std::abs(mid)<.25,"low shelf must settle by upper mids");
    p.rows[0].lowGainDb=-5.;
    const double cut=gainAt(fs,20.,p);
    check(cut< -4.35&&cut> -5.25,"low -5 shelf should approach -5 at 20 Hz");
    p.rows[0].lowGainDb=0.;p.rows[0].highFrequency=BaxHighShelf::Hz1600;
    p.rows[0].highGainDb=5.;
    check(gainAt(fs,150.,p)<.3,"high shelf must not affect low frequencies");
    const double air=gainAt(fs,11000.,p);
    check(air>4.4&&air<5.2,"high shelf should approach +5 in upper treble");
    p.rows[0].highGainDb=-5.;
    const double airCut=gainAt(fs,11000.,p);
    check(airCut< -4.35&&airCut> -5.3,"high negative shelf must cut treble");
    // Mirror-image +5 and -5 dB shelves away from extreme Nyquist warping.
    p.rows[0].highGainDb=5.;const double boost=gainAt(fs,1600.,p);
    p.rows[0].highGainDb=-5.;const double attenuation=gainAt(fs,1600.,p);
    check(std::abs(boost+attenuation)<.25,"boost and cut must have symmetric shelf shoulders");
}
void checkFilters(double fs) {
    auto p=stereo();
    p.rows[0].hpf=BaxHPF::Hz54;
    check(gainAt(fs,12.,p)<-19.,"54Hz highpass should strongly remove 12Hz rumble");
    check(std::abs(gainAt(fs,1000.,p))<.15,"highpass must not color 1kHz");
    p.rows[0].hpf=BaxHPF::Out;p.rows[0].lpf=BaxLPF::KHz7_5;
    check(std::abs(gainAt(fs,150.,p))<.15,"lowpass must not color 150Hz");
    check(gainAt(fs,18000.,p)< -12.,"7.5kHz lowpass must strongly attenuate 18kHz");
}
void checkRouting(double fs){
    Baxandall eq;eq.prepare(fs);
    auto p=stereo();
    p.rows[0].lowGainDb=5.;p.rows[1].lowGainDb=-5.;
    p.rows[0].lowFrequency=p.rows[1].lowFrequency=BaxLowShelf::Hz230;
    eq.set(p);
    double l=0.,r=0.;
    for(int i=0;i<static_cast<int>(fs*.4);++i){auto y=eq.process({.1,.1});l=y.l;r=y.r;}
    check(l>.14&&r<.075,"LR rows must be independent");
    p.routing=BaxRouting::MidSide;eq.set(p);
    for(int i=0;i<static_cast<int>(fs*.4);++i){auto y=eq.process({.1,.1});l=y.l;r=y.r;}
    check(std::abs(l-r)<1.e-5&&l>.14,"M/S must process mono material only through Mid row");
    p.rows[0].in=false;eq.set(p);
    for(int i=0;i<static_cast<int>(fs*.4);++i){auto y=eq.process({.1,.1});l=y.l;r=y.r;}
    check(std::abs(l-.1)<1.e-5&&std::abs(r-.1)<1.e-5,"independent Mid bypass must work");
}
void checkAutomation(double fs){
    Baxandall eq;eq.prepare(fs);auto p=stereo();
    p.rows[0].lowFrequency=BaxLowShelf::Hz131;
    p.rows[0].lowGainDb=5.;p.rows[0].highGainDb=-5.;eq.set(p);
    double prev=0.;double maxJump=0.;
    for(int i=0;i<static_cast<int>(fs*.7);++i){
        if(i%471==0){
            p.rows[0].lowGainDb=-p.rows[0].lowGainDb;
            p.rows[0].highGainDb=-p.rows[0].highGainDb;
            p.rows[0].lowFrequency=p.rows[0].lowFrequency==BaxLowShelf::Hz131?BaxLowShelf::Hz361:BaxLowShelf::Hz74;
            p.rows[0].highFrequency=p.rows[0].highFrequency==BaxHighShelf::Hz3400?BaxHighShelf::Hz1600:BaxHighShelf::Hz18000;
            p.rows[0].hpf=p.rows[0].hpf==BaxHPF::Out?BaxHPF::Hz54:BaxHPF::Out;
            p.rows[0].lpf=p.rows[0].lpf==BaxLPF::Out?BaxLPF::KHz7_5:BaxLPF::Out;
            eq.set(p);
        }
        const double sample=.12*std::sin(2.*kPi*200.*i/fs);
        const double y=eq.process({sample,sample}).l;
        check(std::isfinite(y)&&std::abs(y)<2.,"fast automation output must remain bounded and finite");
        maxJump=std::max(maxJump,std::abs(y-prev));prev=y;
    }
    check(maxJump<.15,"filter automation must not produce large discontinuities");
}
}
int main(){
    for(const double fs:{44100.,48000.,96000.,192000.}){
        checkNeutral(fs);checkShelves(fs);checkFilters(fs);checkRouting(fs);checkAutomation(fs);
    }
    std::printf("Bax manufacturer-informed spectral/routing/automation assertions: %d PASS\n",assertions);
}
