#include "steammaster/DSP.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace steammaster;

namespace {
void check(bool ok, const char* message) {
    if(!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
constexpr double fs=48000.;
using Buffer=std::vector<double>;
Buffer render(ExciterMode mode, OversamplingMode quality, double freq, double amp, double low, double high,
              double lowFreq=400.,double highFreq=2500.) {
    Exciter e;e.prepare(fs);
    ExciterParameters p;p.mode=mode;p.lowDrive=low;p.highDrive=high;
    p.crossoverHz=1400.;p.lowFrequencyHz=lowFreq;p.highFrequencyHz=highFreq;
    e.set(p);e.setOversamplingMode(quality);
    constexpr int n=9600;
    Buffer samples; samples.reserve(n);
    for(int i=0;i<n*2;++i) {
        const double x=amp*std::sin(2.*kPi*freq*static_cast<double>(i)/fs);
        const Stereo y=e.process({x,x});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"Nonfinite Exciter output");
        check(std::abs(y.l)<3.5&&std::abs(y.r)<3.5,"Exciter runaway");
        if(i>=n) samples.push_back(y.l);
    }
    return samples;
}
double magnitude(const Buffer& signal,double freq) {
    double cosine=0.,sine=0.;
    for(std::size_t i=0;i<signal.size();++i) {
        const double phase=2.*kPi*freq*static_cast<double>(i)/fs;
        cosine+=signal[i]*std::cos(phase);
        sine+=signal[i]*std::sin(phase);
    }
    return 2.*std::hypot(cosine,sine)/static_cast<double>(signal.size());
}
void checkVoices() {
    const auto tube=render(ExciterMode::Tube,OversamplingMode::X8,125.,.60,10.,0.);
    const auto tape=render(ExciterMode::Tape,OversamplingMode::X8,125.,.60,10.,0.);
    const auto transformer=render(ExciterMode::Transformer,OversamplingMode::X8,125.,.60,10.,0.);
    const double t2=magnitude(tube,250.),t3=magnitude(tube,375.);
    const double a2=magnitude(tape,250.),a3=magnitude(tape,375.),a5=magnitude(tape,625.);
    const double n2=magnitude(transformer,250.),n3=magnitude(transformer,375.);
    std::printf("Voices (2nd / 3rd / 5th): tube %.7g %.7g, tape %.7g %.7g %.7g, transformer %.7g %.7g\n",
                t2,t3,a2,a3,a5,n2,n3);
    check(t2>2.e-5 && t3>2.e-5,"Tube must generate even and odd harmonics");
    check(a3>1.e-4 && a5>5.e-6,"Tape must generate third and fifth harmonics");
    check(a2<a3*.10,"Tape symmetry must suppress second harmonic");
    check(n2>1.e-5 && n3>1.e-5,"Transformer must generate even and odd harmonics");
    check(n2>3.*n3,"Silk-inspired Transformer must be second-harmonic dominant");
    // Provisional digital calibration: -4 dBFS approximates +20 dBu if
    // 0 dBFS = +24 dBu. This is NOT a hardware calibration guarantee.
    const auto silk=render(ExciterMode::Transformer,OversamplingMode::X8,100.,.631,10.,0.);
    const double silkFund=magnitude(silk,100.);
    const double silkTHD=std::hypot(magnitude(silk,200.),magnitude(silk,300.))/silkFund;
    std::printf("Silk-inspired 100 Hz THD2+3 @-4dBFS: %.3f%%\n",100.*silkTHD);
    check(silkTHD>.004 && silkTHD<.030,"Transformer reference-level THD should remain mastering-subtle");
    double differenceTubeTape=0.,differenceTubeTransformer=0.,differenceTapeTransformer=0.;
    for(std::size_t i=0;i<tube.size();++i) {
        differenceTubeTape+=std::pow(tube[i]-tape[i],2);
        differenceTubeTransformer+=std::pow(tube[i]-transformer[i],2);
        differenceTapeTransformer+=std::pow(tape[i]-transformer[i],2);
    }
    check(differenceTubeTape>1.e-3 && differenceTubeTransformer>1.e-3 && differenceTapeTransformer>1.e-3,
          "All modes need genuinely distinct output waveshapes");
}
void checkAlias() {
    // 10.5k * 3 aliases to 16.5k at 48k without oversampling.
    for(const auto mode:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
        const auto off=render(mode,OversamplingMode::Off,10500.,.70,0.,10.);
        const auto eight=render(mode,OversamplingMode::X8,10500.,.70,0.,10.);
        const double offAlias=magnitude(off,16500.);
        const double eightAlias=magnitude(eight,16500.);
        std::printf("Mode %d alias @16.5kHz: 1x %.8f, 8x %.8f\n",
                    static_cast<int>(mode),offAlias,eightAlias);
        check(offAlias>2.e-6,"Aliasing probe must excite the nonlinear engine");
        check(eightAlias<offAlias*.35,"8x oversampling must materially suppress third-harmonic folding");
    }
}
void checkBandIndependence() {
    for(const auto mode:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
        const auto bassDry=render(mode,OversamplingMode::X4,125.,.45,0.,0.);
        const auto bassDrivenHigh=render(mode,OversamplingMode::X4,125.,.45,0.,10.);
        const auto bassDrivenLow=render(mode,OversamplingMode::X4,125.,.45,10.,0.);
        const auto trebleDry=render(mode,OversamplingMode::X4,7500.,.45,0.,0.);
        const auto trebleDrivenHigh=render(mode,OversamplingMode::X4,7500.,.45,0.,10.);
        const auto trebleDrivenLow=render(mode,OversamplingMode::X4,7500.,.45,10.,0.);
        const auto difference=[](const Buffer& a,const Buffer& b){
            double e=0.;for(std::size_t i=0;i<a.size();++i)e+=std::pow(a[i]-b[i],2);
            return std::sqrt(e/static_cast<double>(a.size()));
        };
        const double lowTarget=difference(bassDrivenLow,bassDry);
        const double lowLeak=difference(bassDrivenHigh,bassDry);
        const double highTarget=difference(trebleDrivenHigh,trebleDry);
        const double highLeak=difference(trebleDrivenLow,trebleDry);
        std::printf("Mode %d band isolation RMS (lowTarget,highLeak,highTarget,lowLeak) %.7f %.7f %.7f %.7f\n",
                    static_cast<int>(mode),lowTarget,lowLeak,highTarget,highLeak);
        check(lowTarget>lowLeak*2.,"Low-band drive must predominantly affect low frequencies");
        check(highTarget>highLeak*2.,"High-band drive must predominantly affect high frequencies");
    }
}


void checkAudibleSpectrumAndFrequencyControls() {
    // Plugin Doctor-style probes: the active Exciter must measurably change
    // the fundamental spectrum AND produce harmonics, not just pass audio.
    for(const auto mode:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
        const auto dryHi=render(mode,OversamplingMode::X4,8000.,.3,0.,0.);
        const auto wetHi=render(mode,OversamplingMode::X4,8000.,.3,0.,10.);
        const auto dryLow=render(mode,OversamplingMode::X4,160.,.3,0.,0.);
        const auto wetLow=render(mode,OversamplingMode::X4,160.,.3,10.,0.);
        const double highChange=std::abs(magnitude(wetHi,8000.)-magnitude(dryHi,8000.));
        const double lowChange=std::abs(magnitude(wetLow,160.)-magnitude(dryLow,160.));
        std::printf("Mode %d spectrum: low fundamental change %.6g; high %.6g\\n",
                    int(mode),lowChange,highChange);
        check(lowChange>.003,"Exciter Low Drive must measurably color the bass spectrum");
        check(highChange>.003,"Exciter High Drive must measurably color the treble spectrum");
        const auto low50=render(mode,OversamplingMode::X4,160.,.3,10.,0.,50.,2500.);
        const auto low700=render(mode,OversamplingMode::X4,160.,.3,10.,0.,700.,2500.);
        const auto high1500=render(mode,OversamplingMode::X4,8000.,.3,0.,10.,400.,1500.);
        const auto high16000=render(mode,OversamplingMode::X4,8000.,.3,0.,10.,400.,16000.);
        check(std::abs(magnitude(low50,160.)-magnitude(low700,160.))>.003,
              "Exciter Low Frequency selector must move the spectral contour");
        check(std::abs(magnitude(high1500,8000.)-magnitude(high16000,8000.))>.003,
              "Exciter High Frequency selector must move the spectral contour");
    }
}

void checkExtendedDriveRange() {
    // Increasing the control above 10 must actually increase the processed
    // signal, not just change the GUI readout. Exercise all three voices
    // and both independently driven bands at 2x the previous maximum.
    const auto rmsDifference=[](const Buffer& a,const Buffer& b) {
        double sum=0.;
        for(std::size_t i=0;i<a.size();++i)sum+=std::pow(a[i]-b[i],2);
        return std::sqrt(sum/double(a.size()));
    };
    for(const auto mode:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
        const auto low10=render(mode,OversamplingMode::X4,160.,.25,10.,0.);
        const auto low20=render(mode,OversamplingMode::X4,160.,.25,20.,0.);
        const auto high10=render(mode,OversamplingMode::X4,6000.,.25,0.,10.);
        const auto high20=render(mode,OversamplingMode::X4,6000.,.25,0.,20.);
        const double lowChange=rmsDifference(low10,low20);
        const double highChange=rmsDifference(high10,high20);
        std::printf("Exciter extended range mode %d: low %.7g, high %.7g\\n",
                    int(mode),lowChange,highChange);
        check(lowChange>1.e-4,"Low Drive 20 must differ measurably from 10");
        check(highChange>1.e-4,"High Drive 20 must differ measurably from 10");
    }
    // Instant max-drive combinations at multiple oversampling settings
    // must remain finite and avoid runaway at conservative source level.
    for(const auto quality:{OversamplingMode::Off,OversamplingMode::X2,
                            OversamplingMode::X4,OversamplingMode::X8}) {
        for(const auto mode:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
            auto high=render(mode,quality,5200.,.25,20.,20.);
            for(const auto v:high)check(std::isfinite(v) && std::abs(v)<3.5,
                                        "Extended Exciter must stay finite and bounded");
        }
    }
}

void checkNeutral() {
    // All drives at zero, for every voice and OS mode, must be exactly a
    // latency-aligned unity transfer (without added hiss, phase or coloration).
    for(const auto m:{ExciterMode::Tube,ExciterMode::Tape,ExciterMode::Transformer}) {
        for(const auto q:{OversamplingMode::Off,OversamplingMode::X2,OversamplingMode::X4,OversamplingMode::X8}) {
            Exciter e;e.prepare(fs);ExciterParameters p;p.mode=m;p.lowDrive=p.highDrive=0.;e.set(p);
            e.setOversamplingMode(q);std::vector<double> input(8000,0.);
            for(std::size_t i=0;i<input.size();++i) input[i]=.23*std::sin(i*.097)+.17*std::sin(i*.69);
            double error=0.;
            for(std::size_t i=0;i<input.size();++i) {
                const auto y=e.process({input[i],-input[i]});
                const double wanted=i>=OversampledProcessor::kLatency?input[i-OversampledProcessor::kLatency]:0.;
                if(i>1000) error=std::max(error,std::max(std::abs(y.l-wanted),std::abs(y.r+wanted)));
            }
            check(error<1.e-8,"Zero drive must be transparent at reported latency");
        }
    }
}
void checkAutomation() {
    Exciter e;e.prepare(fs);
    ExciterParameters p;p.lowDrive=8.;p.highDrive=9.;p.lowFrequencyHz=300.;p.highFrequencyHz=3100.;
    Stereo previous{};double largest=0.;
    for(int i=0;i<24000;++i) {
        if(i%211==0) {
            p.mode=static_cast<ExciterMode>((i/211)%3);
            p.lowDrive=(i/211)%2?10.:4.;
            p.highDrive=(i/211)%3?10.:6.;
            p.crossoverHz=(i/211)%2?1000.:2100.;
            p.common.in=(i/211)%13!=0;
            e.set(p);
            e.setOversamplingMode(static_cast<OversamplingMode>((i/211)%4));
        }
        const double x=.33*std::sin(2.*kPi*491.*i/fs)+.10*std::sin(2.*kPi*9013.*i/fs);
        const auto y=e.process({x,-.83*x});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"Rapid automation must stay finite");
        check(std::abs(y.l)<3.5&&std::abs(y.r)<3.5,"Rapid automation amplitude guard");
        if(i>2000)largest=std::max(largest,std::max(std::abs(y.l-previous.l),std::abs(y.r-previous.r)));
        previous=y;
    }
    std::printf("Automation largest sample discontinuity: %.6g\n",largest);
    check(largest<.32,"Mode and oversampling automation must not create dangerous clicks");
    e.reset();
    for(int i=0;i<24000;++i) {
        auto y=e.process({0.,0.});
        check(std::isfinite(y.l)&&std::isfinite(y.r),"Zero after reset finite");
        if(i>1000)check(std::abs(y.l)+std::abs(y.r)<1.e-7,"Reset must erase magnetic history");
    }
}
}
int main() {
    checkNeutral();
    checkVoices();
    checkAlias();
    checkBandIndependence();
    checkAudibleSpectrumAndFrequencyControls();
    checkExtendedDriveRange();
    checkAutomation();
    std::puts("Exciter DSP: topology, harmonics, aliasing, neutral, automation PASSED");
}
