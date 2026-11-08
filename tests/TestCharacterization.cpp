#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace steammaster;
namespace {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
// A separate 8x output reconstruction path. This tests intersample peaks that
// the limiter's internal 4x detector and sample-domain guard could miss.
class Reference8xPeak final {
public:
    Reference8xPeak() { a_.prepare(); b_.prepare(); c_.prepare(); }
    double step(Stereo in) noexcept {
        Stereo a{},b{},c{},d{},e{},f{},g{},h{};
        a_.up(in,a,b);
        b_.up(a,c,d);
        b_.up(b,e,f);
        double peak=std::max(std::abs(in.l),std::abs(in.r));
        for(auto v : {c,d,e,f}) {
            c_.up(v,g,h);
            peak=std::max({peak,std::abs(g.l),std::abs(g.r),std::abs(h.l),std::abs(h.r)});
        }
        return peak;
    }
private:
    FIRHalfBand2x a_{},b_{},c_{};
};
double eqResponse(TptEQ::Mode mode, double hz, double gain, double testHz, double fs) {
    TptEQ eq; eq.prepare(fs); eq.configure(mode,hz,.7071067811865475,gain);
    const int samples=static_cast<int>(fs*.18);
    double inPower=0, outPower=0;
    for(int i=0;i<samples;++i){
        const double x=.08*std::sin(2.*kPi*testHz*i/fs);
        const double y=eq.process({x,x}).l;
        require(std::isfinite(y),"EQ nonfinite");
        if(i>samples/2){inPower+=x*x;outPower+=y*y;}
    }
    return 10.*std::log10(outPower/inPower);
}
void eqCalibration(double fs) {
    // These are digital topology tests; they do NOT certify a Pultec/Bax hardware match.
    for(double dB: {-12.,-6.,6.,12.}) {
        auto low=eqResponse(TptEQ::Mode::LowShelf,1000.,dB,40.,fs);
        auto lowPass=eqResponse(TptEQ::Mode::LowShelf,1000.,dB,fs*.25,fs);
        auto high=eqResponse(TptEQ::Mode::HighShelf,1000.,dB,fs*.25,fs);
        auto highPass=eqResponse(TptEQ::Mode::HighShelf,1000.,dB,40.,fs);
        auto peak=eqResponse(TptEQ::Mode::Peak,1000.,dB,1000.,fs);
        require(std::abs(low-dB)<.5,"Low shelf gain calibration");
        require(std::abs(lowPass)<.6,"Low shelf opposite passband");
        require(std::abs(high-dB)<.6,"High shelf gain calibration");
        require(std::abs(highPass)<.6,"High shelf opposite passband");
        require(std::abs(peak-dB)<.2,"Parametric peak gain calibration");
    }
}
std::uint64_t rng=0x1234c0ffee98765ULL;
double noise() noexcept {
    rng^=rng<<13; rng^=rng>>7; rng^=rng<<17;
    return 2.*(static_cast<double>((rng>>16)&0xffffu)/65535.)-1.;
}
void limiterStress(double fs) {
    const int n=static_cast<int>(fs*.12);
    for(double lookahead: {.1,1.,5.}) {
        double offPeakDb=0.0;
        for(bool tp : {false,true}) {
            // Each TP mode receives precisely the same deterministic signal.
            rng=0x1234c0ffee98765ULL;
            MasterLimiter lim; lim.prepare(fs);
            LimiterParameters p; p.ceilingDbTP=-1.;p.thresholdDb=-9.;
            p.lookaheadMs=lookahead;p.releaseMs=60.;p.truePeak=tp;
            lim.set(p);
            Reference8xPeak external;
            double maxSample=0,max8=0;
            for(int i=0;i<n;++i) {
                const double burst=(i%977==100)?2.2:0.;
                const double x=.98*std::sin(2*kPi*11000.*i/fs)+.45*noise()+burst;
                const double r=.9*std::sin(2*kPi*6300.*i/fs+.61)+.35*noise()-burst*.9;
                const auto output=lim.process({x,r});
                require(std::isfinite(output.l)&&std::isfinite(output.r),"Limiter produced nonfinite");
                maxSample=std::max({maxSample,std::abs(output.l),std::abs(output.r)});
                max8=std::max(max8,external.step(output));
                if(i==n/2) {p.stereoLink=0.;p.mode=LimiterMode::Punch;lim.set(p);}
            }
            // Bypass and TP-OFF deliberately cannot guarantee ceiling.
            const double db8=gainToDb(max8),ceiling=-1.;
            std::cout<<"  "<<fs<<" Hz "<<lookahead<<" ms, TP="<<tp
                     <<" peak8x="<<db8<<" dBFS, emergency samples="<<lim.safetyClipEvents()<<"\n";
            require(maxSample<=dbToGain(ceiling)+1e-8,"Limiter violated sample ceiling");
            if(tp) {
                // Characterization guard only; this is not a certified hard ceiling.
                // The deliberately adversarial signal is permitted 0.75 dB
                // overshoot while a proper TP limiter algorithm is developed.
                require(db8<=ceiling+.75,"True-peak exploratory overshoot bound exceeded");
                require(db8<offPeakDb-.2,"TP detection failed to improve reconstructed output");
            } else {
                offPeakDb=db8;
            }
            require(lim.safetyClipEvents() < static_cast<std::uint64_t>(n),"Limiter hard-clipped every sample");
        }
    }
}
// Direct harmonic-alias characterization of all three Tape personalities.
// A 11 kHz fundamental produces a 33 kHz third harmonic, which aliases to
// 15 kHz at 48 kHz without adequate oversampling/decimation.
double tapeAliasAmplitude(TapeType type, OversamplingMode mode) {
    constexpr double fs=48000., fundamental=11000., alias=15000.;
    Tape tape;tape.prepare(fs);
    TapeParameters p;p.type=type;p.driveDb=10.;p.speed=TapeSpeed::IPS30;
    tape.set(p);tape.setOversamplingMode(mode);
    double sinSum=0.,cosSum=0.;
    for(int i=0;i<48000;++i) {
        const double x=.83*std::sin(2.*kPi*fundamental*i/fs);
        const double y=tape.process({x,x}).l;
        require(std::isfinite(y),"Tape alias test nonfinite");
        if(i>=24000) {
            sinSum+=y*std::sin(2.*kPi*alias*i/fs);
            cosSum+=y*std::cos(2.*kPi*alias*i/fs);
        }
    }
    return 2.*std::hypot(sinSum,cosSum)/24000.;
}
void tapeAliasTest() {
    for(TapeType t : {TapeType::Studio,TapeType::Vintage,TapeType::Modern}) {
        const double baseline=tapeAliasAmplitude(t,OversamplingMode::Off);
        const double high=tapeAliasAmplitude(t,OversamplingMode::X8);
        const double rejection=gainToDb(baseline)-gainToDb(high);
        std::cout<<"  Tape type "<<static_cast<int>(t)<<" 8x alias rejection: "<<rejection<<" dB\n";
        require(rejection>25.,"Tape 8x did not suppress 3rd harmonic alias sufficiently");
    }
}
void tapeAutomation(double fs) {
    Tape tape; tape.prepare(fs); TapeParameters p;p.common.mix=1.;p.common.in=true;
    for(int i=0;i<static_cast<int>(fs*.28);++i) {
        if(i%173==0) {
            p.type=static_cast<TapeType>((i/173)%3);
            p.speed=static_cast<TapeSpeed>((i/173)%3);
            p.driveDb=(i/173)%2?10.:-10.;p.bias=(i/173)%2?5.:-5.;
            tape.set(p);
            tape.setOversamplingMode(static_cast<OversamplingMode>((i/173)%4));
        }
        const double sample=.6*std::sin(2*kPi*997.*i/fs);
        const auto result=tape.process({sample,sample});
        require(std::isfinite(result.l)&&std::isfinite(result.r),"Tape automation nonfinite");
        require(std::abs(result.l-result.r)<1.e-10,"Tape broke identical stereo imaging");
        require(std::abs(result.l)<4.,"Tape automation produced high-amplitude spike");
    }
}
}
int main() {
    try {
        for(double fs : {44100.,48000.,88200.,96000.,192000.}) {
            eqCalibration(fs);
            tapeAutomation(fs);
            limiterStress(fs);
        }
        tapeAliasTest();
        std::cout << "PASS DSP characterization and high-rate overshoot diagnostics\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "FAIL characterization: " << e.what() << "\n";
        return 1;
    }
}
