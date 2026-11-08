#include "steammaster/DSP.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace steammaster;
namespace {
constexpr double pi=3.14159265358979323846;
void require(bool ok,const char* why){if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
double response(PultecParameters p,double f,double fs=48000.){
    Pultec eq; eq.prepare(fs);p.common={true,1.};eq.set(p);
    double sumInput=0.,sumOutput=0.;
    const auto count=static_cast<int>(fs*.75);
    for(int i=0;i<count;++i){const double x=.1*std::sin(2.*pi*f*i/fs);
        auto y=eq.process({x,x});require(std::isfinite(y.l)&&std::isfinite(y.r),"Pultec nonfinite");
        if(i>count/2){sumInput+=x*x;sumOutput+=y.l*y.l;}}
    return 10.*std::log10(sumOutput/sumInput);
}
double oceanGR(OceanRatio ratio,double levelDb){
    Ocean ocean;ocean.prepare(48000);OceanParameters p{};
    p.common={true,1.};p.ratio=ratio;p.thresholdDb=-30.;p.attackMs=25.;p.recovery=OceanRecovery::S0_2;
    ocean.set(p);
    const double amp=dbToGain(levelDb);
    for(int i=0;i<48000*3;++i){const double x=amp*std::sin(2.*pi*1000.*i/48000.);auto y=ocean.process({x,x});
        require(std::isfinite(y.l)&&std::isfinite(y.r),"Ocean nonfinite");}
    return ocean.gainReductionDb();
}
double sslGR(FurnaceRatio ratio,double levelDb){
    Furnace f;f.prepare(48000);FurnaceParameters p{};p.common={true,1.};
    p.thresholdDb=-20.;p.ratio=ratio;p.attack=FurnaceAttack::Ms1;p.release=FurnaceRelease::S0_1;
    f.set(p);const double amp=dbToGain(levelDb);
    for(int i=0;i<96000;++i){auto y=f.process({amp*std::sin(2.*pi*1000.*i/48000.),0.});
        require(std::isfinite(y.l)&&std::isfinite(y.r),"Furnace nonfinite");}
    return f.gainReductionDb();
}
void shadowDualRelease(){
    Shadow sh;sh.prepare(48000);ShadowParameters p{};p.common={true,1.};
    p.opticalIn=false;p.discreteIn=true;p.discreteThresholdDb=-32.;
    p.discreteRatio=DiscreteRatio::R4;p.discreteAttack=DiscreteAttack::Ms1;
    p.discreteRecovery=DiscreteRecovery::Dual;sh.set(p);
    for(int i=0;i<144000;++i){auto y=sh.process({.5*std::sin(2.*pi*1000.*i/48000.),.5*std::sin(2.*pi*1000.*i/48000.)});
        require(std::isfinite(y.l)&&std::isfinite(y.r),"Shadow nonfinite");}
    const double initial=sh.discreteReductionDb();
    require(initial>3.,"Shadow Dual stage must gain-reduce");
    for(int i=0;i<24000;++i)sh.process({0.,0.});
    const double fast=sh.discreteReductionDb();
    for(int i=0;i<96000;++i)sh.process({0.,0.});
    const double tail=sh.discreteReductionDb();
    require(fast<initial*.55,"Shadow Dual fast 80 percent recovery must function");
    require(fast>0.05,"Shadow Dual must retain a slow recovery tail");
    require(tail<fast,"Shadow slow tail must continue recovering");
    std::printf("Shadow Dual GR: initial %.3f dB, +0.5s %.3f, +2.5s %.3f\n",initial,fast,tail);
}
} // namespace
int main(){
    PultecParameters p{};
    require(std::abs(response(p,60.))<.15,"Pultec unity 60Hz");
    require(std::abs(response(p,10000.))<.15,"Pultec unity 10kHz");
    p.lowFrequencyHz=60;p.lowBoost=10.;
    const double boost30=response(p,30.);
    require(boost30>9.&&boost30<15.,"Pultec LF max boost within published 13.5dB bound");
    p={};p.lowFrequencyHz=60;p.lowAttenuation=10.;
    require(response(p,30.) < -8.,"Pultec low cut must be a low shelf, not a bell");
    require(response(p,700.)>-3.,"Pultec low shelf should spare midrange");
    p={};p.lowFrequencyHz=60;p.lowBoost=8.;p.lowAttenuation=4.;
    std::printf("Pultec both boost/cut low=%.2f upper=%.2f\n",response(p,45.),response(p,230.));
    require(response(p,45.)>response(p,230.)+1.,"Pultec simultaneous boost/cut must focus lows");
    p={};p.highBoostFrequencyHz=8000;p.highBoost=10.;p.bandwidth=0.;
    const double narrow=response(p,8000.),narrowLow=response(p,2000.);
    require(narrow>7.,"Pultec high resonant boost");
    p.bandwidth=1.;
    const double wide=response(p,8000.),wideLow=response(p,2000.);
    require(wideLow>narrowLow,"Pultec bandwidth widening affects shoulder");
    require(wide<narrow,"Pultec broad bandwidth reduces peak gain");
    p={};p.highAttenuation=10.;p.highAttenuationFrequencyHz=5000;
    require(response(p,16000.)<-6.,"Pultec high shelf attenuation");
    const double comp10=oceanGR(OceanRatio::R1_5,-20.);
    const double comp20=oceanGR(OceanRatio::R1_5,-10.);
    const double comp30=oceanGR(OceanRatio::R1_5,0.);
    require(comp10>0.5&&comp20>comp10+1.5&&comp30>comp20+3.,"Manley progressive COMP response");
    const double limit20=oceanGR(OceanRatio::R4,-10.);
    require(limit20>comp20+3.,"Manley LIMIT reduces more than COMP");
    std::printf("SSL below knee R2 %.3f R10 %.3f\n",sslGR(FurnaceRatio::R2,-22.),sslGR(FurnaceRatio::R10,-22.));
    require(sslGR(FurnaceRatio::R2,-22.)>sslGR(FurnaceRatio::R10,-22.)+.05,
            "SSL 2:1 knee must extend below threshold farther than 10:1");
    shadowDualRelease();
    std::printf("Reference checks pass. Ocean COMP over10=%.2f over20=%.2f over30=%.2f, LIMIT over20=%.2f dB\n",comp10,comp20,comp30,limit20);
}
