#include "steammaster/DSP.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace steammaster;
namespace {
constexpr double pi=3.14159265358979323846;
void check(bool cond,const char* msg){if(!cond){std::fprintf(stderr,"FAIL %s\n",msg);std::exit(1);}}
double dB(PultecParameters p,double freq,double fs=48000.){
    Pultec eq;eq.prepare(fs);p.common={true,1.};eq.set(p);
    double in2=0.,out2=0.;const auto count=static_cast<int>(fs*.45);
    for(int i=0;i<count;i++){
        const double x=.12*std::sin(2.*pi*freq*i/fs);
        const auto y=eq.process({x,x});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"frequency sweep finite");
        if(i>count/2){in2+=x*x;out2+=y.l*y.l;}
    }
    return 10.*std::log10(out2/in2);
}
void controlCurves(){
    for(double fs : {44100.,48000.,96000.,192000.}) {
        PultecParameters p{};
        check(std::abs(dB(p,50.,fs))<.18,"neutral LF");
        check(std::abs(dB(p,1000.,fs))<.18,"neutral 1k");
        check(std::abs(dB(p,12000.,fs))<.18,"neutral HF");
        p.lowFrequencyHz=60;p.lowBoost=10.;
        check(dB(p,30.,fs)>9.&&dB(p,30.,fs)<15.,"low boost from passive RC ladder");
        check(dB(p,1000.,fs)<1.,"low boost spares upper mids");
        p={};p.lowFrequencyHz=60;p.lowAttenuation=10.;
        check(dB(p,30.,fs)<-8.,"passive capacitive LF atten");
        check(dB(p,700.,fs)>-3.,"LF cut spares higher bands");
        p={};p.lowBoost=8.;p.lowAttenuation=4.;
        check(dB(p,45.,fs)>dB(p,230.,fs)+2.,"true branch interaction low shelf+mid dip");
        p={};p.highBoost=10.;p.bandwidth=0.;p.highBoostFrequencyHz=8000;
        check(dB(p,8000.,fs)>7.,"RLC high peak");
        p.bandwidth=1.;check(dB(p,2000.,fs)>0.,"RLC wide shoulder");
        p={};p.highAttenuation=10.;p.highAttenuationFrequencyHz=5000;
        check(dB(p,16000.,fs)<-6.,"RC high attenuation");
    }
}
void resetAndAutomation(){
    Pultec eq;eq.prepare(48000.);
    PultecParameters p{};p.common={true,1.};eq.set(p);
    for(int i=0;i<5000;++i)eq.process({.17*std::sin(2.*pi*200.*i/48000.),0.});
    p.lowBoost=10.;p.lowAttenuation=6.;p.highBoost=8.;eq.set(p);
    for(int i=0;i<50;++i)eq.process({.17*std::sin(2.*pi*200.*i/48000.),0.});
    // Reset while crossfading must not revert to the old pre-automation controls.
    eq.reset();
    double input2=0.,output2=0.;
    for(int i=0;i<48000;++i){const double x=.10*std::sin(2.*pi*45.*i/48000.);
        auto y=eq.process({x,0.});check(std::isfinite(y.l)&&std::abs(y.l)<5.,"reset after automation finite");
        check(std::abs(y.r)<1e-12,"stereo channel isolation");
        if(i>24000){input2+=x*x;output2+=y.l*y.l;}}
    const double response=10.*std::log10(output2/input2);
    check(response>0.1,"reset retained new LF settings");
    // Rapid automation is finite and bounded for several sample rates.
    for(double fs : {44100.,48000.,96000.,192000.}) {
        eq.prepare(fs);p={};p.common={true,1.};
        double peak=0.;
        for(int n=0;n<static_cast<int>(fs*.22);++n) {
            if(n%127==0) {
                const int mode=(n/127)%10;
                p.lowBoost=mode;p.lowAttenuation=10-mode;
                p.highBoost=mode;p.highAttenuation=10-mode;
                p.bandwidth=mode*.111;
                p.lowFrequencyHz=(mode%3==0?30:(mode%3==1?60:100));
                p.highBoostFrequencyHz=(mode%3==0?8000:(mode%3==1?12000:16000));
                p.highAttenuationFrequencyHz=(mode%3==0?5000:(mode%3==1?10000:20000));
                eq.set(p);
            }
            const double x=.10*std::sin(2.*pi*440.*n/fs);
            const auto y=eq.process({x,x*.23});
            check(std::isfinite(y.l)&&std::isfinite(y.r),"automation finite");
            peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));
        }
        check(peak<6.,"automated circuit bounded");
    }
}
void linearity(){
    Pultec a,b; a.prepare(48000.);b.prepare(48000.);
    PultecParameters p{};p.common={true,1.};p.lowBoost=6;p.lowAttenuation=5;
    p.highBoost=4;p.highAttenuation=7;
    a.set(p);b.set(p);
    for(int i=0;i<26000;++i){const double x=.2*std::sin(2.*pi*1111.*i/48000.);
        const auto first=a.process({x,x*.7}),second=b.process({.5*x,.5*x*.7});
        check(std::abs(second.l-.5*first.l)<2e-9 && std::abs(second.r-.5*first.r)<2e-9,
              "pure passive section is linear and does not invent harmonic distortion");}
}
}
int main(){controlCurves();resetAndAutomation();linearity();std::puts("Pultec independent circuit tests passed");}
