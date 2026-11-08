// Outputs modeled transfer/frequency response for external hardware calibration.
// This executable DOES NOT contain measurements of physical equipment.
#include "steammaster/DSP.hpp"
#include <cmath>
#include <cstdio>
using namespace steammaster;
namespace {
constexpr double pi=3.14159265358979323846;
double eqResponse(const PultecParameters& p,double hz){
    Pultec eq;eq.prepare(48000);eq.set(p);
    double input=0.,output=0.;
    for(int i=0;i<48000;++i){const double x=.1*std::sin(2.*pi*hz*static_cast<double>(i)/48000.);
        const Stereo y=eq.process({x,x});
        if(i>=24000){input+=x*x;output+=y.l*y.l;}}
    return 10.*std::log10(output/input);
}
double oceanGR(OceanRatio mode,double db){
    Ocean c;c.prepare(48000);OceanParameters p{};p.ratio=mode;p.thresholdDb=-30.;p.attackMs=25.;p.recovery=OceanRecovery::S0_2;c.set(p);
    const double a=dbToGain(db);
    for(int i=0;i<96000;++i){const double x=a*std::sin(2.*pi*1000.*i/48000.);c.process({x,x});}
    return c.gainReductionDb();
}
double sslGR(FurnaceRatio mode,double db){
    Furnace c;c.prepare(48000);FurnaceParameters p{};p.ratio=mode;p.thresholdDb=-20.;p.attack=FurnaceAttack::Ms1;p.release=FurnaceRelease::S0_1;c.set(p);
    const double a=dbToGain(db);
    for(int i=0;i<96000;++i){const double x=a*std::sin(2.*pi*1000.*i/48000.);c.process({x,x});}
    return c.gainReductionDb();
}
double shadowGR(bool optical,double db){
    Shadow c;c.prepare(48000);ShadowParameters p{};p.opticalIn=optical;p.discreteIn=!optical;
    p.opticalThresholdDb=-30.;p.discreteThresholdDb=-30.;
    p.discreteRatio=DiscreteRatio::R3;p.discreteAttack=DiscreteAttack::Ms1;p.discreteRecovery=DiscreteRecovery::S0_2;c.set(p);
    const double a=dbToGain(db);
    for(int i=0;i<96000;++i){const double x=a*std::sin(2.*pi*1000.*i/48000.);c.process({x,x});}
    return optical?c.opticalReductionDb():c.discreteReductionDb();
}
}
int main(){
    std::puts("# SteamMaster modeled curves, NOT physical hardware measurements; 48kHz test environment");
    std::puts("processor,configuration,input_dBFS,frequency_Hz,model_response_dB,model_GR_dB");
    constexpr double hz[]={20.,30.,45.,60.,90.,120.,180.,230.,400.,1000.,2000.,3000.,5000.,8000.,12000.,16000.};
    PultecParameters p{};
    for(int config=0;config<5;++config){
        p={};
        if(config==1)p.lowBoost=10.;
        if(config==2)p.lowAttenuation=10.;
        if(config==3){p.lowBoost=8.;p.lowAttenuation=4.;}
        if(config==4){p.highBoost=8.;p.highAttenuation=3.;}
        const char* labels[]={"flat","LF_boost10","LF_atten10","LF_boost8_atten4","HF_boost8_cut3"};
        for(double f:hz)std::printf("Pultec,%s,-20,%.1f,%.4f,0\n",labels[config],f,eqResponse(p,f));
    }
    for(double db=-42.;db<=0.;db+=6.){
        std::printf("Ocean,COMP_1.5,%.1f,1000,0,%.4f\n",db,oceanGR(OceanRatio::R1_5,db));
        std::printf("Ocean,LIMIT_4,%.1f,1000,0,%.4f\n",db,oceanGR(OceanRatio::R4,db));
        std::printf("Furnace,R2,%.1f,1000,0,%.4f\n",db,sslGR(FurnaceRatio::R2,db));
        std::printf("Furnace,R4,%.1f,1000,0,%.4f\n",db,sslGR(FurnaceRatio::R4,db));
        std::printf("Furnace,R10,%.1f,1000,0,%.4f\n",db,sslGR(FurnaceRatio::R10,db));
        std::printf("Shadow,Optical_2,%.1f,1000,0,%.4f\n",db,shadowGR(true,db));
        std::printf("Shadow,Discrete_3,%.1f,1000,0,%.4f\n",db,shadowGR(false,db));
    }
}
