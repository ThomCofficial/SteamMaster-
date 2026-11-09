#include "steammaster/DSP.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace steammaster {
namespace {
constexpr double kMin = 1.0e-15;
double timeCoefficient(double fs, double seconds) noexcept {
    return std::exp(-1.0 / std::max(1.0, fs * std::max(seconds, 1.0e-6)));
}
double follow(double current, double target, double coefficient) noexcept {
    return target + coefficient * (current - target);
}
double ratioOcean(OceanRatio r) noexcept {
    switch(r) {case OceanRatio::R1_2:return 1.2;case OceanRatio::R1_5:return 1.5;
    case OceanRatio::R2:return 2.;case OceanRatio::R3:return 3.;
    case OceanRatio::R4:return 4.;default:return 6.;}
}
double recoveryOcean(OceanRecovery r) noexcept {
    switch(r) {case OceanRecovery::S0_2:return .2;case OceanRecovery::S0_4:return .4;
    case OceanRecovery::S0_6:return .6;case OceanRecovery::S4:return 4.;default:return 8.;}
}
double ratioFurnace(FurnaceRatio r) noexcept {
    switch(r) {case FurnaceRatio::R2:return 2.;case FurnaceRatio::R4:return 4.;default:return 10.;}
}
double attackFurnace(FurnaceAttack r) noexcept {
    switch(r) {case FurnaceAttack::Ms0_1:return .1;case FurnaceAttack::Ms0_3:return .3;
    case FurnaceAttack::Ms1:return 1.;case FurnaceAttack::Ms3:return 3.;
    case FurnaceAttack::Ms10:return 10.;default:return 30.;}
}
double releaseFurnace(FurnaceRelease r, double gr) noexcept {
    switch(r) {case FurnaceRelease::S0_1:return .1;case FurnaceRelease::S0_3:return .3;
    case FurnaceRelease::S0_6:return .6;case FurnaceRelease::S1_2:return 1.2;
    default:return .1 + .8 * clamp(gr / 12.0, 0.0, 1.0);}
}
double ratioDiscrete(DiscreteRatio r) noexcept {
    switch(r) {case DiscreteRatio::R1_2:return 1.2;case DiscreteRatio::R2:return 2.;
    case DiscreteRatio::R3:return 3.;case DiscreteRatio::R4:return 4.;
    case DiscreteRatio::R6:return 6.;default:return 20.;}
}
double attackDiscrete(DiscreteAttack r) noexcept {
    switch(r) {case DiscreteAttack::Ms0_1:return .1;case DiscreteAttack::Ms0_3:return .3;
    case DiscreteAttack::Ms1:return 1.;case DiscreteAttack::Ms3:return 3.;
    case DiscreteAttack::Ms10:return 10.;default:return 30.;}
}
double recoveryDiscrete(DiscreteRecovery r, double gr) noexcept {
    switch(r) {case DiscreteRecovery::S0_1:return .1;case DiscreteRecovery::S0_2:return .2;
    case DiscreteRecovery::S0_4:return .4;case DiscreteRecovery::S0_6:return .6;
    case DiscreteRecovery::S0_8:return .8;case DiscreteRecovery::S1_2:return 1.2;
    default:return .08 + 1.1*clamp(gr / 14.0, 0.0, 1.0);}
}
double compressDb(double level, double threshold, double ratio, double kneeDb) noexcept {
    const double over = level - threshold;
    const double reductionFactor = 1.0 - 1.0/std::max(1.0, ratio);
    if (over <= -kneeDb*.5) return 0.;
    if (over >= kneeDb*.5) return over * reductionFactor;
    const double t = (over + .5*kneeDb);
    return reductionFactor * t*t/(2.0*kneeDb);
}
double levelOf(Stereo v) noexcept {return std::max(std::abs(v.l),std::abs(v.r));}
Stereo lerp(Stereo a, Stereo b, double wet) noexcept {
    return {a.l + (b.l-a.l)*wet, a.r+(b.r-a.r)*wet};
}
} // namespace

double dbToGain(double db) noexcept {return std::pow(10., clamp(db,-240.,100.) / 20.);}
double gainToDb(double gain) noexcept {return 20.*std::log10(std::max(kMin,gain));}
double sanitized(double v) noexcept {
    if (!std::isfinite(v) || std::abs(v) < 1.0e-30) return 0.;
    return clamp(v,-32.,32.);
}
double clamp(double x,double low,double high) noexcept {
    return std::isfinite(x) ? std::max(low,std::min(high,x)) : low;
}
void Smoother::prepare(double fs, double seconds) noexcept {alpha_=1.-timeCoefficient(fs,seconds);}
void Smoother::reset(double v) noexcept {current_=target_=sanitized(v);}
void Smoother::set(double v) noexcept {target_=sanitized(v);}
double Smoother::next() noexcept {
    current_ += alpha_*(target_-current_);
    if (std::abs(target_-current_) < 1.e-12) current_=target_;
    return current_;
}

void DetectorHPF::prepare(double fs) noexcept {
    fs_=fs; low_={};wet_.prepare(fs,.006);wet_.reset(0.);
}
void DetectorHPF::setFrequency(double hz) noexcept {
    hz=clamp(hz,0.,fs_*.45);
    coeff_=hz > 0. ? std::exp(-2.*kPi*hz/fs_) : 0.;
    wet_.set(hz>0. ? 1.:0.);
}
Stereo DetectorHPF::process(Stereo x) noexcept {
    // Preserve detector continuity when bypassed; the filter state evolves.
    // Using coefficient on each new sample avoids sidechain startup transients.
    low_.l = (1. - coeff_)*x.l + coeff_*low_.l;
    low_.r = (1. - coeff_)*x.r + coeff_*low_.r;
    if(std::abs(low_.l)<1.e-30)low_.l=0.;
    if(std::abs(low_.r)<1.e-30)low_.r=0.;
    const double wet=wet_.next();
    return {x.l - wet*low_.l, x.r - wet*low_.r};
}

void TptEQ::prepare(double fs) noexcept {
    fs_=fs;clear();
    for (auto* s: {&g_,&k_,&m0_,&m1_,&m2_}) s->prepare(fs,.012);
    g_.reset(.05);k_.reset(1.);m0_.reset(1.);m1_.reset(0.);m2_.reset(0.);
}
void TptEQ::clear() noexcept {l_={};r_={};}
void TptEQ::configure(Mode mode,double hz,double q,double dB) noexcept {
    hz=clamp(hz,5.,fs_*.45);q=clamp(q,.15,20.);dB=clamp(dB,-24.,24.);
    const double A=std::pow(10.,dB/40.);
    double g=std::tan(kPi*hz/fs_),k=1./q,m0=1.,m1=0.,m2=0.;
    if(mode==Mode::Peak) { k=1./(q*A);m1=k*(A*A-1.); }
    else if(mode==Mode::LowShelf) {g/=std::sqrt(A);m1=k*(A-1.);m2=A*A-1.;}
    else if(mode==Mode::HighShelf) {g*=std::sqrt(A);m0=A*A;m1=k*(1.-A)*A;m2=1.-A*A;}
    else if(mode==Mode::LowPass) {m0=0.;m2=1.;}
    else if(mode==Mode::HighPass) {m0=1.;m1=-k;m2=-1.;}
    g_.set(g);k_.set(k);m0_.set(m0);m1_.set(m1);m2_.set(m2);
}
double TptEQ::run(double x,State& z) noexcept {
    const double g=g_.value(),k=k_.value();
    const double a1=1./(1.+g*(g+k));
    const double a2=g*a1, a3=g*a2;
    const double v3=x-z.ic2;
    const double v1=a1*z.ic1+a2*v3;
    const double v2=z.ic2+a2*z.ic1+a3*v3;
    z.ic1=sanitized(2.*v1-z.ic1);
    z.ic2=sanitized(2.*v2-z.ic2);
    return sanitized(m0_.value()*x + m1_.value()*v1 + m2_.value()*v2);
}
Stereo TptEQ::process(Stereo in) noexcept {
    g_.next();k_.next();m0_.next();m1_.next();m2_.next();
    return {run(in.l,l_),run(in.r,r_)};
}

void Module::prepare(double fs) noexcept {
    fs_=fs;enabled_.prepare(fs,.006);mix_.prepare(fs,.010);
    enabled_.reset(1.);mix_.reset(1.);
}
void Module::setCommon(CommonControls p) noexcept {
    enabled_.set(p.in?1.:0.);mix_.set(clamp(p.mix,0.,1.));
}
Stereo Module::process(Stereo in) noexcept {
    auto w=processWet(in);
    auto dry=dryDelay_.process(in);
    return lerp(dry,w,enabled_.next()*mix_.next());
}

// Progressive Manley Variable Mu COMP and LIMIT characteristics. The
// published owner's manual plots curved GR characteristics and explicitly
// reports roughly 1.5:1 at 2 dB GR, 2:1 at 5 dB, 5:1 at 10 dB and up to
// approximately 8:1 on heavily driven COMP; LIMIT rises toward 20:1.
// These sparse published landmarks do NOT specify an entire transfer curve.
// Smooth interpolation and the knee width below are engineering fits.
double manleyCompRatio(double reductionDb) noexcept {
    constexpr double depths[]={0.,2.,5.,10.,17.};
    constexpr double ratios[]={1.28,1.50,2.00,5.00,8.00};
    if(reductionDb>=depths[4])return ratios[4];
    for(int i=0;i<4;++i) {
        if(reductionDb<=depths[i+1]) {
            const double t=clamp((reductionDb-depths[i])/(depths[i+1]-depths[i]),0.,1.);
            const double eased=t*t*(3.-2.*t);
            return ratios[i]+(ratios[i+1]-ratios[i])*eased;
        }
    }
    return ratios[4];
}
void Ocean::prepareReductionCurves() noexcept {
    constexpr double start=-16.,step=.125;
    for(std::size_t k=0;k<reductionCurves_.size();++k) {
        const OceanRatio mode=static_cast<OceanRatio>(k);
        const double selected=ratioOcean(mode);
        const double knee=selected<=2.0?13.0:5.0;
        double reduction=0.;
        for(std::size_t i=0;i<kCurvePoints;++i) {
            const double over=start+static_cast<double>(i)*step;
            double ratio=selected;
            if(selected<=2.) {
                const double scale=(selected-1.)/.5;
                ratio=1.+(manleyCompRatio(reduction)-1.)*scale;
            } else {
                const double t=clamp((reduction-8.)/10.,0.,1.);
                const double eased=t*t*(3.-2.*t);
                ratio=selected+(20.-selected)*eased;
            }
            // The extra ratio modes 1.2/2/3/6 are SteamMaster extensions,
            // not controls on the original COMP/LIMIT hardware.
            const double kneeProgress=clamp((over+.5*knee)/knee,0.,1.);
            const double slope=kneeProgress*(1.-1./std::max(1.001,ratio));
            if(i>0)reduction+=step*slope;
            reductionCurves_[k][i]=static_cast<float>(reduction);
        }
    }
}
double Ocean::curveReduction(double overDb) const noexcept {
    constexpr double start=-16.,step=.125;
    const auto& curve=reductionCurves_[static_cast<unsigned>(p_.ratio)];
    const double x=(overDb-start)/step;
    if(x<=0.)return 0.;
    if(x>=static_cast<double>(kCurvePoints-1)) {
        // Deep limiting beyond the documented grid is conservatively bounded.
        return static_cast<double>(curve.back())+(overDb-80.)*.95;
    }
    const auto i=static_cast<std::size_t>(x);
    const double f=x-static_cast<double>(i);
    return static_cast<double>(curve[i])*(1.-f)+static_cast<double>(curve[i+1])*f;
}

void Ocean::prepare(double fs) noexcept {
    prepareReductionCurves();
    Module::prepare(fs);setWetLatency(OversampledProcessor::kLatency);
    tube_.prepare(fs);tube_.setMode(quality_);linearDelay_.prepare(OversampledProcessor::kLatency);
    sc_.prepare(fs);threshold_.prepare(fs,.012);threshold_.reset(p_.thresholdDb);set(p_);reset();
}
void Ocean::set(const OceanParameters& p) noexcept {
    p_=p;
    if(static_cast<unsigned>(p_.ratio)>=6u)p_.ratio=OceanRatio::R1_5;
    p_.attackMs=clamp(p.attackMs,25.,70.);
    p_.thresholdDb=clamp(p.thresholdDb,-60.,12.);
    sc_.setFrequency(p.scHpfHz);threshold_.set(p_.thresholdDb);setCommon(p.common);
}
void Ocean::reset() noexcept {
    envelope_=reductionDb_=0.;sc_.reset();tube_.reset();tube_.setMode(quality_);
    linearDelay_.reset();resetDryDelay();
}
Stereo Ocean::tubeTransfer(Stereo x,unsigned,void*) noexcept {
    // Calibrated *topology-inspired* transfer, not an extracted 5670/6BA6 curve.
    // Differential push-pull cancellation strongly reduces even harmonics;
    // small asymmetry leaves a modest second harmonic at higher levels.
    constexpr double drive=0.90;
    const auto valve=[](double sample) noexcept {
        constexpr double bias=0.045;
        const double y=0.5*(std::tanh(drive*sample+bias)+std::tanh(drive*sample-bias));
        const double norm=drive*(1.-std::tanh(bias)*std::tanh(bias));
        const double asym=0.0025*sample*sample/(1.+sample*sample);
        return sanitized(y/norm+asym);
    };
    return {valve(x.l),valve(x.r)};
}
Stereo Ocean::processWet(Stereo in) noexcept {
    const auto sc=sc_.process(in);
    const double target=levelOf(sc);
    const bool increasing=target>envelope_;
    // The five recovery positions and 25..70 ms attack follow Manley's manual.
    // The extra level-dependent recovery is a modeling hypothesis, not a bench fit.
    const double release=recoveryOcean(p_.recovery)*(1.+.18*clamp(reductionDb_/12.,0.,1.));
    envelope_=follow(envelope_,target,timeCoefficient(fs_,increasing?p_.attackMs*.001:release));
    if(envelope_<1.e-30)envelope_=0.;
    const double level=gainToDb(envelope_);
    const double threshold=threshold_.next();
    // GR curve interpolated from a fixed, allocation-free LUT whose local
    // slopes are anchored to the manufacturer's published characteristics.
    const double requested=curveReduction(level-threshold);
    reductionDb_=follow(reductionDb_,requested,timeCoefficient(fs_,.005));
    const double gain=dbToGain(-reductionDb_);
    const Stereo compressed{in.l*gain,in.r*gain};
    const auto colored=tube_.process(compressed,&Ocean::tubeTransfer,nullptr);
    const auto linear=linearDelay_.process(compressed);
    // Conservative coloration at nominal operating level. Real harmonics
    // depend on transformer loading, tube revision and drive calibration.
    return {linear.l*.95+colored.l*.05,linear.r*.95+colored.r*.05};
}

void Furnace::prepare(double fs) noexcept {
    Module::prepare(fs);sc_.prepare(fs);threshold_.prepare(fs,.012);threshold_.reset(p_.thresholdDb);set(p_);reset();
}
void Furnace::set(const FurnaceParameters& p) noexcept {
    p_=p;p_.thresholdDb=clamp(p.thresholdDb,-15.,15.);
    sc_.setFrequency(p.scHpfHz);threshold_.set(p_.thresholdDb);setCommon(p.common);
}
void Furnace::reset() noexcept {envelope_=reductionDb_=autoFast_=autoSlow_=0.;sc_.reset();}
Stereo Furnace::processWet(Stereo in) noexcept {
    auto sc=sc_.process(in);
    const double target=levelOf(sc);
    const bool attack=target>envelope_;
    const double seconds=attack?attackFurnace(p_.attack)*.001:releaseFurnace(p_.release,reductionDb_);
    envelope_=follow(envelope_,target,timeCoefficient(fs_,seconds));
    if(envelope_<1.e-30)envelope_=0.;
    const double threshold=threshold_.next();
    // SSL documents an unusually wide, ratio-dependent 2:1 knee. The 24 dB
    // numerical width is a conservative working approximation; not a measured fit.
    const double knee=p_.ratio==FurnaceRatio::R2?24.0:(p_.ratio==FurnaceRatio::R4?6.0:2.0);
    const double requested=compressDb(gainToDb(envelope_),threshold,ratioFurnace(p_.ratio),knee);
    if(p_.release==FurnaceRelease::Auto) {
        // Parallel fast/slow recovery memories: transient GR releases promptly,
        // sustained GR leaves a longer tail. Both states always track the detector.
        autoFast_=follow(autoFast_,requested,timeCoefficient(fs_,requested>autoFast_?.002:.10));
        autoSlow_=follow(autoSlow_,requested,timeCoefficient(fs_,requested>autoSlow_?.025:1.15));
        reductionDb_=std::max(0.0,.70*autoFast_+.30*autoSlow_);
    } else {
        reductionDb_=follow(reductionDb_,requested,timeCoefficient(fs_,.002));
        autoFast_=autoSlow_=reductionDb_;
    }
    const double gain=dbToGain(-reductionDb_);
    return {in.l*gain,in.r*gain};
}

void ParametricEQ::prepare(double fs) noexcept {
    Module::prepare(fs);for(auto& b:bands_)b.prepare(fs);
    output_.prepare(fs,.01);output_.reset(0.);
}
void ParametricEQ::set(const ParametricParameters& p) noexcept {
    setCommon(p.common);output_.set(clamp(p.outputDb,-12.,12.));
    for(std::size_t i=0;i<bands_.size();++i) {
        auto q=p.bands[i];
        auto mode=q.mode;
        if(i==1 || i==2) mode=TptEQ::Mode::Peak; // Only bands 1 and 4 switch shelf/peak
        if(i==0 && mode!=TptEQ::Mode::LowShelf) mode=TptEQ::Mode::Peak;
        if(i==3 && mode!=TptEQ::Mode::HighShelf) mode=TptEQ::Mode::Peak;
        bands_[i].configure(mode,q.frequencyHz,q.q,clamp(q.gainDb,-12.,12.));
    }
}
void ParametricEQ::reset() noexcept {for(auto& b:bands_)b.clear();}
Stereo ParametricEQ::processWet(Stereo in) noexcept {
    for(auto& b:bands_) in=b.process(in);
    const double gain=dbToGain(output_.next());
    return {in.l*gain,in.r*gain};
}

void Pultec::prepare(double fs) noexcept {
    Module::prepare(fs);
    passive_.prepare(fs);
    output_.prepare(fs,.012);
    output_.reset(0.);
}
void Pultec::set(const PultecParameters& p) noexcept {
    setCommon(p.common);
    output_.set(clamp(p.outputDb,-12.,12.));
    passive_.configure(p.lowFrequencyHz,p.lowBoost,p.lowAttenuation,
                       p.highBoostFrequencyHz,p.highBoost,p.bandwidth,
                       p.highAttenuationFrequencyHz,p.highAttenuation);
}
void Pultec::reset() noexcept { passive_.reset(); }
Stereo Pultec::processWet(Stereo x) noexcept {
    const auto y=passive_.process(x);
    const double gain=dbToGain(output_.next());
    return {y.l*gain,y.r*gain};
}

void Shadow::prepare(double fs) noexcept {
    Module::prepare(fs);opticalSC_.prepare(fs);discreteSC_.prepare(fs);
    for(auto* s:{&opticalEnable_,&discreteEnable_,&opticalThreshold_,&discreteThreshold_,&opticalGain_,&discreteGain_,&output_})s->prepare(fs,.009);
    opticalEnable_.reset(1.);discreteEnable_.reset(1.);
    opticalThreshold_.reset(p_.opticalThresholdDb);discreteThreshold_.reset(p_.discreteThresholdDb);
    opticalGain_.reset(0.);discreteGain_.reset(0.);output_.reset(0.);
    set(p_);reset();
}
void Shadow::set(const ShadowParameters& p) noexcept {
    p_=p;setCommon(p.common);
    opticalSC_.setFrequency(p.scHpfHz);discreteSC_.setFrequency(p.scHpfHz);
    opticalEnable_.set(p.opticalIn?1.:0.);discreteEnable_.set(p.discreteIn?1.:0.);
    opticalThreshold_.set(clamp(p.opticalThresholdDb,-60.,12.));
    discreteThreshold_.set(clamp(p.discreteThresholdDb,-60.,12.));
    opticalGain_.set(clamp(p.opticalMakeupDb,-24.,24.));
    discreteGain_.set(clamp(p.discreteGainDb,-24.,24.));
    output_.set(clamp(p.outputDb,-12.,12.));
}
void Shadow::reset() noexcept {
    opticalEnv_=discreteEnv_=0.;opticalReductionDb_=discreteReductionDb_=0.;
    opticalFastReduction_=opticalSlowReduction_=0.;
    discreteFastReduction_=discreteSlowReduction_=0.;
    opticalSC_.reset();discreteSC_.reset();
}
Stereo Shadow::processWet(Stereo in) noexcept {
    // Optical: long release memory and broad knee, stereo-linked detector.
    auto optSC=opticalSC_.process(in);
    const double detector=levelOf(optSC);
    // Shadow Hills optical topology: nominal 2:1, with a dual-time-constant
    // release memory (fast majority and slower residual). Not hardware calibrated.
    opticalEnv_=follow(opticalEnv_,detector,timeCoefficient(fs_,detector>opticalEnv_?.010:.055));
    if(opticalEnv_<1.e-30)opticalEnv_=0.;
    const double optWanted=compressDb(gainToDb(opticalEnv_),opticalThreshold_.next(),2.0,12.);
    opticalFastReduction_=follow(opticalFastReduction_,optWanted,
        timeCoefficient(fs_,optWanted>opticalFastReduction_?.010:.12));
    opticalSlowReduction_=follow(opticalSlowReduction_,optWanted,
        timeCoefficient(fs_,optWanted>opticalSlowReduction_?.018:1.40));
    opticalReductionDb_=std::max(0.0,.80*opticalFastReduction_+.20*opticalSlowReduction_);
    const double optGain=dbToGain(-opticalReductionDb_+opticalGain_.next());
    Stereo optOut{in.l*optGain,in.r*optGain};
    Stereo midway=lerp(in,optOut,opticalEnable_.next());
    // Discrete VCA: stepped timings, 1.2:1 to Flood 20:1.
    auto discSC=discreteSC_.process(midway);
    const double det2=levelOf(discSC);
    // Shadow Hills DISCRETE Dual mirrors the optical two-phase recovery.
    // Do not mistake a GR-dependent single time constant for the documented
    // fast ~80% / slow ~20% recovery characteristic.
    const bool dual=p_.discreteRecovery==DiscreteRecovery::Dual;
    const double rel=dual?.055:recoveryDiscrete(p_.discreteRecovery,discreteReductionDb_);
    const double attack=attackDiscrete(p_.discreteAttack)*.001;
    discreteEnv_=follow(discreteEnv_,det2,timeCoefficient(fs_,det2>discreteEnv_?attack:rel));
    if(discreteEnv_<1.e-30)discreteEnv_=0.;
    const double discWanted=compressDb(gainToDb(discreteEnv_),discreteThreshold_.next(),ratioDiscrete(p_.discreteRatio),4.);
    if(dual) {
        discreteFastReduction_=follow(discreteFastReduction_,discWanted,
            timeCoefficient(fs_,discWanted>discreteFastReduction_?attack:.12));
        discreteSlowReduction_=follow(discreteSlowReduction_,discWanted,
            timeCoefficient(fs_,discWanted>discreteSlowReduction_?attack:1.40));
        discreteReductionDb_=std::max(0.,.80*discreteFastReduction_+.20*discreteSlowReduction_);
    } else {
        discreteReductionDb_=follow(discreteReductionDb_,discWanted,timeCoefficient(fs_,.002));
        discreteFastReduction_=discreteSlowReduction_=discreteReductionDb_;
    }
    const double discGain=dbToGain(-discreteReductionDb_+discreteGain_.next());
    Stereo discreteOut{midway.l*discGain,midway.r*discGain};
    Stereo out=lerp(midway,discreteOut,discreteEnable_.next());
    const double makeup=dbToGain(output_.next());
    return {out.l*makeup,out.r*makeup};
}

void Exciter::Pole::prepare(double fs) noexcept {
    coeff.prepare(fs,.015); coeff.reset(0.); reset();
}
void Exciter::Pole::set(double fs,double hz) noexcept {
    hz=clamp(hz,1.,fs*.45);coeff.set(1.-std::exp(-2.*kPi*hz/fs));
}
Stereo Exciter::Pole::low(Stereo x) noexcept {
    const double a=coeff.next();
    l += a*(x.l-l);r += a*(x.r-r);
    l=sanitized(l);r=sanitized(r);
    return {l,r};
}
void Exciter::prepare(double fs) noexcept {
    Module::prepare(fs);lowDrive_.prepare(fs,.010);highDrive_.prepare(fs,.010);output_.prepare(fs,.010);
    crossover_.prepare(fs);lowTone_.prepare(fs);highTone_.prepare(fs);dcRemoval_.prepare(fs);
    for(unsigned i=0;i<4;++i) {
        const double rate=std::max(8000.,fs)*static_cast<double>(1u<<i);
        tapeCoeff_[i]=std::exp(-1./(rate*.009));
        transformerCoeff_[i]=std::exp(-1./(rate*.0025));
    }
    setWetLatency(OversampledProcessor::kLatency);
    lowSaturator_.prepare(fs);highSaturator_.prepare(fs);
    inputDelay_.prepare(OversampledProcessor::kLatency);
    bassDelay_.prepare(OversampledProcessor::kLatency);
    trebleDelay_.prepare(OversampledProcessor::kLatency);
    lowFocusDelay_.prepare(OversampledProcessor::kLatency);
    highFocusDelay_.prepare(OversampledProcessor::kLatency);
    for(std::size_t i=0;i<modeWeights_.size();++i) {modeWeights_[i].prepare(fs,.015);modeWeights_[i].reset(i==0?1.:0.);}
    lowDrive_.reset(0.);highDrive_.reset(0.);output_.reset(0.);
    set(p_);reset();
}
void Exciter::set(const ExciterParameters& p) noexcept {
    p_=p;setCommon(p.common);
    for(std::size_t i=0;i<modeWeights_.size();++i)modeWeights_[i].set(i==static_cast<std::size_t>(p.mode)?1.:0.);
    lowDrive_.set(clamp(p.lowDrive,0.,10.));highDrive_.set(clamp(p.highDrive,0.,10.));
    output_.set(clamp(p.outputDb,-12.,12.));
    crossover_.set(fs_,p.crossoverHz);lowTone_.set(fs_,p.lowFrequencyHz);
    highTone_.set(fs_,p.highFrequencyHz);dcRemoval_.set(fs_,8.);
}
void Exciter::reset() noexcept {
    crossover_.reset();lowTone_.reset();highTone_.reset();dcRemoval_.reset();
    for(auto& row:voiceStates_) for(auto& v:row)v={};
    for(std::size_t i=0;i<modeWeights_.size();++i)
        modeWeights_[i].reset(i==static_cast<std::size_t>(p_.mode)?1.:0.);
    lowSaturator_.reset();highSaturator_.reset();
    lowSaturator_.setMode(quality_);highSaturator_.setMode(quality_);
    inputDelay_.reset();bassDelay_.reset();trebleDelay_.reset();
    lowFocusDelay_.reset();highFocusDelay_.reset();resetDryDelay();
}
Stereo Exciter::saturationTransfer(Stereo in,unsigned quality,void* raw) noexcept {
    const auto& c=*static_cast<SaturationContext*>(raw);
    auto& state=c.engine->voiceStates_[c.band][quality];
    const double amount=c.amount;
    // Normalize around silence: stage(x=0)==0 and d(stage)/dx at 0 == 1.
    // This preserves low-level gain while introducing drive-dependent curves.
    const auto valve=[](double x,double drive,double bias) noexcept {
        const double t=std::tanh(bias);
        const double slope=drive*(1.-t*t);
        return (std::tanh(drive*x+bias)-t)/slope;
    };
    const auto apply=[valve,quality](double x,double& flux,double& previous,
                        double& transformerFlux,double tapeCoeff,double transformerCoeff,
                        double amt,unsigned band,const std::array<double,3>& w) noexcept {
        // Keep magnetic state alive during mode fades. Tape's odd-order
        // transfer and slew softening are HEDD-inspired, not its proprietary code.
        flux=tapeCoeff*flux+(1.-tapeCoeff)*x;
        transformerFlux=transformerCoeff*transformerFlux+(1.-transformerCoeff)*x;
        double y=0.;
        if(w[0]>1.e-9) {
            // HG-2 signal-path inspiration: filtered parallel tube saturation
            // feeds a pentode-like stage, then a triode-like stage. Existing
            // low/high band filters stand in for the HG-2 sat-frequency switch.
            const double parallel=valve(x,1.+1.35*amt,-.21);
            const double fed=x+.30*amt*(parallel-x);
            const double pentode=valve(fed,1.+1.10*amt,.11+.13*amt);
            const double triode=valve(pentode,.85+1.05*amt,-.06);
            y+=w[0]*(.88*triode+.12*x);
        }
        if(w[1]>1.e-9) {
            // Tape: symmetrical 3rd/5th harmonic soft knee, low-frequency
            // magnetic memory and level-dependent HF slew compression.
            // No intentionally injected even harmonics in this path.
            const double magnetized=x+.040*amt*(flux-x);
            const double drive=1.+2.3*amt;
            const double compressed=std::tanh(drive*magnetized)/drive;
            const double target=.20*magnetized+.80*compressed;
            const double slew=(target-previous);
            const double rateComp=static_cast<double>(1u<<quality);
            const double scaled=1./(1.+.23*amt*rateComp*std::abs(slew));
            previous+=scaled*slew;
            y+=w[1]*previous;
        } else {
            // Running history while inaudible avoids surprise when switching.
            previous=x;
        }
        if(w[2]>1.e-9) {
            // Silk-inspired transformer: frequency-dependent core memory,
            // deliberately stronger even-order texture in the high band.
            // These are not matched Neve Red/Blue curves.
            const double magnetized=x+.07*amt*(transformerFlux-x);
            const double bias=(band==0?.48:.62)*(.7+.3*amt);
            const double saturated=valve(magnetized,.75+.50*amt,bias);
            y+=w[2]*(.14*saturated+.86*x);
        }
        return sanitized(y);
    };
    return {
        apply(in.l,state.tapeFlux.l,state.tapeOutput.l,state.transformerFlux.l,
              c.engine->tapeCoeff_[quality],c.engine->transformerCoeff_[quality],amount,c.band,c.weights),
        apply(in.r,state.tapeFlux.r,state.tapeOutput.r,state.transformerFlux.r,
              c.engine->tapeCoeff_[quality],c.engine->transformerCoeff_[quality],amount,c.band,c.weights)
    };
}

Stereo Exciter::processWet(Stereo x) noexcept {
    const auto lowBand=crossover_.low(x);
    const Stereo highBand{x.l-lowBand.l,x.r-lowBand.r};
    const auto lowFocus=lowTone_.low(lowBand);
    const auto highLP=highTone_.low(highBand);
    const Stereo highFocus{highBand.l-highLP.l,highBand.r-highLP.r};

    // FIX: the old filtering excited only frequencies below Low Freq and
    // above High Freq. At default 100 Hz / 5 kHz almost the entire music
    // spectrum never reached the nonlinear circuits.
    // Excite the actual crossover bands; the tone controls still focus
    // the response within each band rather than deleting its midrange.
    const Stereo bass{.75*lowBand.l+.25*lowFocus.l,
                      .75*lowBand.r+.25*lowFocus.r};
    const Stereo treble{.75*highBand.l+.25*highFocus.l,
                        .75*highBand.r+.25*highFocus.r};

    const double lo=lowDrive_.next()/10.,hi=highDrive_.next()/10.;
    const std::array<double,3> weights{modeWeights_[0].next(),modeWeights_[1].next(),modeWeights_[2].next()};
    SaturationContext low{this,0u,lo,weights};
    SaturationContext high{this,1u,hi,weights};
    const auto colorBass=lowSaturator_.process(bass,&Exciter::saturationTransfer,&low);
    const auto colorTreble=highSaturator_.process(treble,&Exciter::saturationTransfer,&high);
    const auto delayedBass=bassDelay_.process(bass);
    const auto delayedTreble=trebleDelay_.process(treble);
    const auto delayedInput=inputDelay_.process(x);
    const auto delayedLowFocus=lowFocusDelay_.process(lowFocus);
    const auto delayedHighFocus=highFocusDelay_.process(highFocus);

    // Restore the ORIGINAL dual-drive weighting, including its nonlinear
    // drive curves; do not alter the transfer models or other modules.
    const Stereo delta{lo*(colorBass.l-delayedBass.l)+hi*(colorTreble.l-delayedTreble.l),
                       lo*(colorBass.r-delayedBass.r)+hi*(colorTreble.r-delayedTreble.r)};
    const auto dc=dcRemoval_.low(delta);

    // Alongside harmonics, real exciters change the measured frequency
    // response. These gentle, mode-specific contours are time-aligned
    // with the oversampled saturation. Tube adds air, Tape softens the
    // upper band, and Transformer adds upper presence. All are silent
    // at zero drive, without hidden coloration when bypassed.
    const double lowContour=lo*(.18*weights[0]+.24*weights[1]+.14*weights[2]);
    const double highContour=hi*(.24*weights[0]-.13*weights[1]+.17*weights[2]);
    const Stereo tone{lowContour*delayedLowFocus.l+highContour*delayedHighFocus.l,
                      lowContour*delayedLowFocus.r+highContour*delayedHighFocus.r};

    const double gain=dbToGain(output_.next());
    return {(delayedInput.l+delta.l-dc.l+tone.l)*gain,
            (delayedInput.r+delta.r-dc.r+tone.r)*gain};
}

void SixModuleRack::prepare(double fs) noexcept {
    fs=clamp(fs,8000.,384000.);
    ocean.prepare(fs);furnace.prepare(fs);parametric.prepare(fs);
    pultec.prepare(fs);shadow.prepare(fs);exciter.prepare(fs);
    // Default neutral EQ & exciters; initialized deliberately, no implicit GUI mapping.
    parametric.set(ParametricParameters{});
    pultec.set(PultecParameters{});
    exciter.set(ExciterParameters{});
    nonFiniteEvents_.store(0,std::memory_order_relaxed);
    emergencyLimitEvents_.store(0,std::memory_order_relaxed);
}
void SixModuleRack::reset() noexcept {
    ocean.reset();furnace.reset();parametric.reset();pultec.reset();shadow.reset();exciter.reset();
}
Stereo SixModuleRack::process(Stereo in) noexcept {
    // Emergency containment is intentionally separate from any mastered ceiling.
    // It is NOT a true-peak limiter and does not imply release-grade protection.
    auto guard=[this](Stereo x) noexcept {
        for(double* v:{&x.l,&x.r}) {
            if(!std::isfinite(*v)) {nonFiniteEvents_.fetch_add(1,std::memory_order_relaxed);*v=0.;}
            else if(std::abs(*v)>4.) {emergencyLimitEvents_.fetch_add(1,std::memory_order_relaxed);*v=clamp(*v,-4.,4.);}
            else if(std::abs(*v)<1.e-30) *v=0.;
        }
        return x;
    };
    in=guard(in);
    in=guard(ocean.process(in));in=guard(furnace.process(in));
    in=guard(parametric.process(in));in=guard(pultec.process(in));
    in=guard(shadow.process(in));in=guard(exciter.process(in));
    return in;
}
} // namespace steammaster
