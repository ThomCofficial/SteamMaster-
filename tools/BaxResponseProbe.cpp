#include "steammaster/Mastering.hpp"
#include <array>
#include <cmath>
#include <cstdio>
using namespace steammaster;
namespace {
double response(double fs,double hz,BaxandallParameters p) {
    Baxandall eq;eq.prepare(fs);eq.set(p);
    const int warm=static_cast<int>(fs*.4),samples=static_cast<int>(fs*.15);
    const double w=2.*kPi*hz/fs;
    double re=0.,im=0.,rx=0.,ix=0.;
    for(int n=0;n<warm+samples;++n){
        const double sn=std::sin(w*n),cs=std::cos(w*n);
        const double y=eq.process({.1*sn,.1*sn}).l;
        if(n>=warm){re+=y*sn;im+=y*cs;rx+=.1*sn*sn;ix+=.1*sn*cs;}
    }
    return 20.*std::log10(std::max(1.e-12,std::hypot(re,im)/std::hypot(rx,ix)));
}
}
int main(){
    auto p=BaxandallParameters{};p.routing=BaxRouting::LeftRight;
    std::printf("preset,sample_rate_hz,frequency_hz,response_db\n");
    const std::array<double,20> f{10.,20.,30.,50.,74.,100.,150.,230.,400.,700.,1000.,1600.,2500.,3400.,5000.,7500.,10000.,12000.,16000.,20000.};
    for(int preset=0;preset<7;++preset){
        p.rows[0]=BaxRowParameters{};
        const char* name="neutral";
        switch(preset){
        case 1:p.rows[0].lowGainDb=5.;p.rows[0].lowFrequency=BaxLowShelf::Hz74;name="low+5_74";break;
        case 2:p.rows[0].lowGainDb=-5.;p.rows[0].lowFrequency=BaxLowShelf::Hz74;name="low-5_74";break;
        case 3:p.rows[0].highGainDb=5.;p.rows[0].highFrequency=BaxHighShelf::Hz1600;name="high+5_1600";break;
        case 4:p.rows[0].highGainDb=-5.;p.rows[0].highFrequency=BaxHighShelf::Hz1600;name="high-5_1600";break;
        case 5:p.rows[0].hpf=BaxHPF::Hz54;name="highpass54";break;
        case 6:p.rows[0].lpf=BaxLPF::KHz7_5;name="lowpass7500";break;
        }
        for(auto hz:f)std::printf("%s,96000,%.1f,%.5f\n",name,hz,response(96000.,hz,p));
    }
}
