#include "steammaster/Mastering.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
using namespace steammaster;
namespace {
void check(bool ok,const char* what) {if(!ok){std::fprintf(stderr,"FAIL: %s\n",what);std::exit(1);}}
struct Signature { double fundamental=0.,second=0.,third=0.,fifth=0.,rms=0.; };
Signature measure(TapeType type,TapeSpeed speed,double bias,double drive,double hz=1000.,double amplitude=.65, OversamplingMode quality=OversamplingMode::X8) {
    constexpr double fs=48000.;constexpr int n=12000,warmup=6000;
    Tape t;t.prepare(fs);TapeParameters p;p.type=type;p.speed=speed;p.bias=bias;p.driveDb=drive;t.set(p);t.setOversamplingMode(quality);
    std::array<double,4> si{},co{};double energy=0;
    for(int i=0;i<n;++i) {
        const double x=amplitude*std::sin(2.*kPi*hz*static_cast<double>(i)/fs);
        const auto y=t.process({x,x});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"Tape nonfinite");
        check(std::abs(y.l-y.r)<1.e-11,"Tape breaks identical stereo input");
        check(std::abs(y.l)<4.0,"Tape output unexpectedly high");
        if(i<warmup)continue;
        energy+=y.l*y.l;
        for(int j=0;j<4;++j){int harmonic=j==3?5:j+1;
            const double phase=2.*kPi*hz*static_cast<double>(harmonic*i)/fs;
            si[j]+=y.l*std::sin(phase);co[j]+=y.l*std::cos(phase);
        }
    }
    auto component=[&](int k){return 2.*std::hypot(si[k],co[k])/static_cast<double>(n-warmup);};
    return {component(0),component(1),component(2),component(3),std::sqrt(energy/(n-warmup))};
}
void signatures() {
    auto studio=measure(TapeType::Studio,TapeSpeed::IPS15,0.,9.);
    auto vintage=measure(TapeType::Vintage,TapeSpeed::IPS15,0.,9.);
    auto modern=measure(TapeType::Modern,TapeSpeed::IPS15,0.,9.);
    std::printf("Tape v1.4 H3 Studio/Vintage/Modern: %.7f / %.7f / %.7f\n",studio.third,vintage.third,modern.third);
    check(studio.third>modern.third*1.4,"Studio must differ from cleaner Modern");
    check(vintage.third>studio.third*1.12,"Vintage must distort more than Studio");
    check(vintage.second>modern.second*1.7,"Vintage must retain more even harmonic character");
    auto under=measure(TapeType::Vintage,TapeSpeed::IPS15,-5.,9.);
    auto over=measure(TapeType::Vintage,TapeSpeed::IPS15,5.,9.);
    std::printf("Tape under/overbias H3: %.7f / %.7f\n",under.third,over.third);
    check(under.third>over.third*1.15,"Overbias should reduce saturation distortion");
    auto light=measure(TapeType::Vintage,TapeSpeed::IPS15,0.,-10.);
    check(light.third<vintage.third*.75,"Drive should control harmonic generation");
}
void speedProfiles() {
    auto lo=measure(TapeType::Studio,TapeSpeed::IPS7_5,0.,-6.,10000.,.07);
    auto hi=measure(TapeType::Studio,TapeSpeed::IPS30,0.,-6.,10000.,.07);
    std::printf("Tape 10k 7.5/30 IPS fundamental: %.7f / %.7f\n",lo.fundamental,hi.fundamental);
    check(lo.fundamental<hi.fundamental*.92,"Slow tape must lose more upper treble");
    auto bump=measure(TapeType::Studio,TapeSpeed::IPS7_5,0.,-6.,46.,.08);
    auto highSpeedBass=measure(TapeType::Studio,TapeSpeed::IPS30,0.,-6.,46.,.08);
    std::printf("Tape 46Hz 7.5/30 IPS: %.7f / %.7f\n",bump.fundamental,highSpeedBass.fundamental);
    check(bump.fundamental>highSpeedBass.fundamental*1.04,"Head bump must follow speed");
    auto over=measure(TapeType::Studio,TapeSpeed::IPS7_5,5.,-6.,10000.,.07);
    check(over.fundamental<lo.fundamental,"Overbias should increase HF loss");
}
void levelMatching() {
    // Own low-drive, low-level 1 kHz calibration at 48kHz. This asserts
    // the model is not accidentally louder when switching tape personalities.
    for(auto type:{TapeType::Studio,TapeType::Vintage,TapeType::Modern})
        for(auto speed:{TapeSpeed::IPS7_5,TapeSpeed::IPS15,TapeSpeed::IPS30}){
            const auto a=measure(type,speed,0.,-10.,1000.,.05);
            const double gain=20.*std::log10(a.fundamental/.05);
            std::printf("Tape unity profile=%d speed=%d: %.4f dB\n",
                        static_cast<int>(type),static_cast<int>(speed),gain);
            check(std::abs(gain)<.18,"Tape 1k level mismatch at gentle level");
        }
}
void switching(double fs) {
    Tape t;t.prepare(fs);TapeParameters p;t.setOversamplingMode(OversamplingMode::X8);
    double last=0.;double largestJump=0.;
    const int n=static_cast<int>(fs*.25);
    for(int i=0;i<n;++i){
        if(i%257==0){const int j=i/257;
            p.type=static_cast<TapeType>(j%3);p.speed=static_cast<TapeSpeed>((j/3)%3);
            p.bias=j%2?5.:-5.;p.driveDb=j%2?10.:-10.;p.hfTrimDb=j%2?6.:-6.;
            t.set(p);t.setOversamplingMode(static_cast<OversamplingMode>(j%4));
        }
        const double x=.50*std::sin(2.*kPi*997.*i/fs);
        const auto y=t.process({x,x});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"Tape automation produced NaN");
        check(std::abs(y.l)<4.,"Tape automation amplitude exceeded safety bound");
        largestJump=std::max(largestJump,std::abs(y.l-last));last=y.l;
    }
    std::printf("Tape max automation adjacent sample delta @%.0f: %.6f\n",fs,largestJump);
    check(largestJump<1.5,"Tape automation jump exceeds guard");
    t.reset();for(int i=0;i<500;++i){const auto y=t.process({});check(std::abs(y.l)<1.e-10&&std::abs(y.r)<1.e-10,"Reset silence must remain silent");}
}
}
int main(){signatures();speedProfiles();levelMatching();for(double fs:{44100.,48000.,96000.,192000.})switching(fs);std::puts("PASS tape hardware-inspired regression");}
