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

void Ocean::prepare(double fs) noexcept {
    Module::prepare(fs);setWetLatency(OversampledProcessor::kLatency);
    tube_.prepare(fs);tube_.setMode(quality_);linearDelay_.prepare(OversampledProcessor::kLatency);
    sc_.prepare(fs);threshold_.prepare(fs,.012);threshold_.reset(p_.thresholdDb);set(p_);reset();
}
void Ocean::set(const OceanParameters& p) noexcept {
    p_=p;p_.attackMs=clamp(p.attackMs,25.,70.);
    p_.thresholdDb=clamp(p.thresholdDb,-60.,12.);
    sc_.setFrequency(p.scHpfHz);threshold_.set(p_.thresholdDb);setCommon(p.common);
}
void Ocean::reset() noexcept {
    envelope_=reductionDb_=0.;sc_.reset();tube_.reset();tube_.setMode(quality_);
    linearDelay_.reset();resetDryDelay();
}
Stereo Ocean::tubeTransfer(Stereo x,unsigned,void*) noexcept {
    constexpr double normalizer=1./.8177540779702877; // 1 / tanh(1.15)
    return {std::tanh(x.l*1.15)*normalizer,std::tanh(x.r*1.15)*normalizer};
}
Stereo Ocean::processWet(Stereo in) noexcept {
    auto sc=sc_.process(in);
    const double target=levelOf(sc);
    const bool increasing=target>envelope_;
    const double release = recoveryOcean(p_.recovery)*(1.+.28*clamp(reductionDb_/12.,0.,1.));
    envelope_=follow(envelope_,target,timeCoefficient(fs_,increasing?p_.attackMs*.001:release));
    if(envelope_<1.e-30)envelope_=0.;
    const double over=gainToDb(envelope_)-threshold_.next();
    const double progress=clamp((over+6.)/18.,0.,1.);
    const double ratio=1.+(ratioOcean(p_.ratio)-1.)*(.45+.55*progress);
    const double requested=compressDb(gainToDb(envelope_),threshold_.value(),ratio,8.);
    reductionDb_=follow(reductionDb_,requested,timeCoefficient(fs_,.005));
    const double gain=dbToGain(-reductionDb_);
    // Low-level, gentle tube-like coloration; NOT a physical valve model.
    const Stereo compressed{in.l*gain,in.r*gain};
    const auto colored=tube_.process(compressed,&Ocean::tubeTransfer,nullptr);
    const auto linear=linearDelay_.process(compressed);
    return {linear.l*.93 + colored.l*.07,linear.r*.93+colored.r*.07};
}

void Furnace::prepare(double fs) noexcept {
    Module::prepare(fs);sc_.prepare(fs);threshold_.prepare(fs,.012);threshold_.reset(p_.thresholdDb);set(p_);reset();
}
void Furnace::set(const FurnaceParameters& p) noexcept {
    p_=p;p_.thresholdDb=clamp(p.thresholdDb,-15.,15.);
    sc_.setFrequency(p.scHpfHz);threshold_.set(p_.thresholdDb);setCommon(p.common);
}
void Furnace::reset() noexcept {envelope_=reductionDb_=0.;sc_.reset();}
Stereo Furnace::processWet(Stereo in) noexcept {
    auto sc=sc_.process(in);
    const double target=levelOf(sc);
    const bool attack=target>envelope_;
    const double seconds=attack?attackFurnace(p_.attack)*.001:releaseFurnace(p_.release,reductionDb_);
    envelope_=follow(envelope_,target,timeCoefficient(fs_,seconds));
    if(envelope_<1.e-30)envelope_=0.;
    const double threshold=threshold_.next();
    const double requested=compressDb(gainToDb(envelope_),threshold,ratioFurnace(p_.ratio),3.);
    reductionDb_=follow(reductionDb_,requested,timeCoefficient(fs_,.002));
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
    Module::prepare(fs);lowBoost_.prepare(fs);lowCut_.prepare(fs);
    highBoost_.prepare(fs);highCut_.prepare(fs);
    output_.prepare(fs,.012);output_.reset(0.);
}
void Pultec::set(const PultecParameters& p) noexcept {
    setCommon(p.common);output_.set(clamp(p.outputDb,-12.,12.));
    const int low=(p.lowFrequencyHz==30 || p.lowFrequencyHz==60 || p.lowFrequencyHz==100) ? p.lowFrequencyHz:60;
    const int high=(p.highBoostFrequencyHz==8000 || p.highBoostFrequencyHz==12000 || p.highBoostFrequencyHz==16000) ? p.highBoostFrequencyHz:12000;
    const int atten=(p.highAttenuationFrequencyHz==5000 || p.highAttenuationFrequencyHz==10000 || p.highAttenuationFrequencyHz==20000) ? p.highAttenuationFrequencyHz:10000;
    const double width=clamp(p.bandwidth,0.,1.);
    // DSP curve scaling is explicitly EXPERIMENTAL; 0..10 controls preserved.
    lowBoost_.configure(TptEQ::Mode::LowShelf,static_cast<double>(low),.65,clamp(p.lowBoost,0.,10.)*1.15);
    lowCut_.configure(TptEQ::Mode::LowShelf,static_cast<double>(low)*1.8,.7,-clamp(p.lowAttenuation,0.,10.)*.95);
    highBoost_.configure(TptEQ::Mode::Peak,static_cast<double>(high),.28+width*.92,clamp(p.highBoost,0.,10.)*.95);
    highCut_.configure(TptEQ::Mode::HighShelf,static_cast<double>(atten),.7,-clamp(p.highAttenuation,0.,10.)*.95);
}
void Pultec::reset() noexcept {lowBoost_.clear();lowCut_.clear();highBoost_.clear();highCut_.clear();}
Stereo Pultec::processWet(Stereo x) noexcept {
    x=lowBoost_.process(x);x=lowCut_.process(x);x=highBoost_.process(x);x=highCut_.process(x);
    const double gain=dbToGain(output_.next());return{x.l*gain,x.r*gain};
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
    opticalSC_.reset();discreteSC_.reset();
}
Stereo Shadow::processWet(Stereo in) noexcept {
    // Optical: long release memory and broad knee, stereo-linked detector.
    auto optSC=opticalSC_.process(in);
    const double detector=levelOf(optSC);
    const double optRelease=.22+1.4*clamp(opticalReductionDb_/15.,0.,1.);
    opticalEnv_=follow(opticalEnv_,detector,timeCoefficient(fs_,detector>opticalEnv_?.010:optRelease));
    if(opticalEnv_<1.e-30)opticalEnv_=0.;
    const double optWanted=compressDb(gainToDb(opticalEnv_),opticalThreshold_.next(),2.8,12.);
    opticalReductionDb_=follow(opticalReductionDb_,optWanted,timeCoefficient(fs_,.008));
    const double optGain=dbToGain(-opticalReductionDb_+opticalGain_.next());
    Stereo optOut{in.l*optGain,in.r*optGain};
    Stereo midway=lerp(in,optOut,opticalEnable_.next());
    // Discrete VCA: stepped timings, 1.2:1 to Flood 20:1.
    auto discSC=discreteSC_.process(midway);
    const double det2=levelOf(discSC);
    const double rel=recoveryDiscrete(p_.discreteRecovery,discreteReductionDb_);
    discreteEnv_=follow(discreteEnv_,det2,timeCoefficient(fs_,det2>discreteEnv_?attackDiscrete(p_.discreteAttack)*.001:rel));
    if(discreteEnv_<1.e-30)discreteEnv_=0.;
    const double discWanted=compressDb(gainToDb(discreteEnv_),discreteThreshold_.next(),ratioDiscrete(p_.discreteRatio),4.);
    discreteReductionDb_=follow(discreteReductionDb_,discWanted,timeCoefficient(fs_,.002));
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
    for(unsigned i=0;i<4;++i) tapeCoeff_[i]=std::exp(-1./(std::max(8000.,fs)*static_cast<double>(1u<<i)*.012));
    setWetLatency(OversampledProcessor::kLatency);
    lowSaturator_.prepare(fs);highSaturator_.prepare(fs);
    inputDelay_.prepare(OversampledProcessor::kLatency);
    bassDelay_.prepare(OversampledProcessor::kLatency);
    trebleDelay_.prepare(OversampledProcessor::kLatency);
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
    for(auto& row:tapeMemory_) for(auto& v:row)v={};
    lowSaturator_.reset();highSaturator_.reset();
    lowSaturator_.setMode(quality_);highSaturator_.setMode(quality_);
    inputDelay_.reset();bassDelay_.reset();trebleDelay_.reset();resetDryDelay();
}
Stereo Exciter::saturationTransfer(Stereo in,unsigned quality,void* raw) noexcept {
    const auto& c=*static_cast<SaturationContext*>(raw);
    auto& mem=c.engine->tapeMemory_[c.band][quality];
    const double memoryCoeff=c.engine->tapeCoeff_[quality];
    mem.l=memoryCoeff*mem.l+(1.-memoryCoeff)*in.l;
    mem.r=memoryCoeff*mem.r+(1.-memoryCoeff)*in.r;
    Stereo result{};
    // A single active personality costs only one nonlinear transfer. During
    // mode automation the outgoing/incoming transfers are blended smoothly.
    if(c.weights[0]>1.e-9) {
        result.l+=c.weights[0]*(std::tanh(c.drive*(in.l+.12))-c.tubeBias)*c.invTanhDrive;
        result.r+=c.weights[0]*(std::tanh(c.drive*(in.r+.12))-c.tubeBias)*c.invTanhDrive;
    }
    if(c.weights[1]>1.e-9) {
        result.l+=c.weights[1]*std::tanh(c.drive*(in.l+.035*mem.l))*c.invTanhDrive;
        result.r+=c.weights[1]*std::tanh(c.drive*(in.r+.035*mem.r))*c.invTanhDrive;
    }
    if(c.weights[2]>1.e-9) {
        result.l+=c.weights[2]*std::atan(c.drive*in.l)*c.invAtanDrive;
        result.r+=c.weights[2]*std::atan(c.drive*in.r)*c.invAtanDrive;
    }
    result.l=sanitized(result.l);
    result.r=sanitized(result.r);
    return result;
}
Stereo Exciter::processWet(Stereo x) noexcept {
    const auto lowBand=crossover_.low(x);
    const Stereo highBand{x.l-lowBand.l,x.r-lowBand.r};
    const auto bass=lowTone_.low(lowBand);
    const auto highLP=highTone_.low(highBand);
    const Stereo treble{highBand.l-highLP.l,highBand.r-highLP.r};
    const double lo=lowDrive_.next()/10.,hi=highDrive_.next()/10.;
    const std::array<double,3> weights{modeWeights_[0].next(),modeWeights_[1].next(),modeWeights_[2].next()};
    const double lowD=1.+lo*3.5,highD=1.+hi*3.5;
    SaturationContext low{this,0u,lowD,1./std::tanh(lowD),1./std::atan(lowD),std::tanh(.12*lowD),weights};
    SaturationContext high{this,1u,highD,1./std::tanh(highD),1./std::atan(highD),std::tanh(.12*highD),weights};
    const auto colorBass=lowSaturator_.process(bass,&Exciter::saturationTransfer,&low);
    const auto colorTreble=highSaturator_.process(treble,&Exciter::saturationTransfer,&high);
    const auto delayedBass=bassDelay_.process(bass);
    const auto delayedTreble=trebleDelay_.process(treble);
    const auto delayedInput=inputDelay_.process(x);
    const Stereo delta{lo*(colorBass.l-delayedBass.l)+hi*(colorTreble.l-delayedTreble.l),
                       lo*(colorBass.r-delayedBass.r)+hi*(colorTreble.r-delayedTreble.r)};
    const auto dc=dcRemoval_.low(delta);
    const double gain=dbToGain(output_.next());
    return {(delayedInput.l+delta.l-dc.l)*gain,(delayedInput.r+delta.r-dc.r)*gain};
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
