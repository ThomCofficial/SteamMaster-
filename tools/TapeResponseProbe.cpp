#include "steammaster/Mastering.hpp"
#include <array>
#include <cmath>
#include <cstdio>
using namespace steammaster;
namespace {
const char* modeName(TapeType t) noexcept {
    switch(t){case TapeType::Studio:return "Studio";case TapeType::Vintage:return "Vintage";default:return "Modern";}
}
int speedName(TapeSpeed s) noexcept {
    switch(s){case TapeSpeed::IPS7_5:return 75;case TapeSpeed::IPS15:return 150;default:return 300;}
}
double response(TapeType type,TapeSpeed speed,double freq){
    constexpr double fs=48000.;constexpr int n=16800;
    Tape t;t.prepare(fs);TapeParameters p;p.type=type;p.speed=speed;p.driveDb=-10.;p.bias=0.;t.set(p);t.setOversamplingMode(OversamplingMode::X8);
    double input=0.,output=0.;
    for(int i=0;i<n;++i){
        const double x=.05*std::sin(2.*kPi*freq*i/fs);
        const double y=t.process({x,x}).l;
        if(i>n/2){input+=x*x;output+=y*y;}
    }
    return 10.*std::log10(output/input);
}
}
int main(){
    std::puts("profile,tape_speed_ips,frequency_hz,modeled_gain_db");
    for(auto type:{TapeType::Studio,TapeType::Vintage,TapeType::Modern})
        for(auto speed:{TapeSpeed::IPS7_5,TapeSpeed::IPS15,TapeSpeed::IPS30})
            for(double hz:{20.,40.,46.,70.,100.,250.,1000.,4000.,10000.,16000.})
                std::printf("%s,%.1f,%.0f,%.4f\n",modeName(type),speedName(speed)/10.,hz,response(type,speed,hz));
}
