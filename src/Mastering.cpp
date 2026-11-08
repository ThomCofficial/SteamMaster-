#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <cmath>

namespace steammaster {
namespace {
constexpr double kSmall=1.e-15;
double smoothCoef(double fs,double seconds) noexcept {return std::exp(-1./std::max(1.,fs*std::max(.00001,seconds)));}
double quantHalf(double v,double minimum,double maximum) noexcept {
    return .5*std::round(2.*clamp(v,minimum,maximum));
}
double magnitude(Stereo x) noexcept {return std::max(std::abs(x.l),std::abs(x.r));}
Stereo blend(Stereo a,Stereo b,double t) noexcept {return {a.l+(b.l-a.l)*t,a.r+(b.r-a.r)*t};}
constexpr std::array<double,8> hpfPositions{{0.,12.,18.,24.,30.,36.,43.,54.}};
constexpr std::array<double,8> lowPositions{{74.,84.,98.,116.,131.,166.,230.,361.}};
constexpr std::array<double,8> highPositions{{1600.,1800.,2100.,2500.,3400.,4800.,7100.,18000.}};
constexpr std::array<double,8> lpfPositions{{0.,70000.,28000.,18000.,12600.,11100.,9000.,7500.}};
template <typename E> unsigned index(E e) noexcept {return std::min(7u,static_cast<unsigned>(e));}
} // namespace

void Baxandall::Row::prepare(double fs) noexcept {
    hpf_.prepare(fs);lowShelf_.prepare(fs);highShelf_.prepare(fs);lpf_.prepare(fs);
    for(auto* x:{&hpfMix_,&lpfMix_,&bypass_,&output_})x->prepare(fs,.012);
    hpfMix_.reset(0.);lpfMix_.reset(0.);bypass_.reset(1.);output_.reset(0.);
}
void Baxandall::Row::set(const BaxRowParameters& p,double fs) noexcept {
    const double hp=hpfPositions[index(p.hpf)], lp=lpfPositions[index(p.lpf)];
    hpf_.configure(TptEQ::Mode::HighPass,hp>0. ? hp:24.,.7071067811865);
    hpfMix_.set(hp>0.?1.:0.);
    lowShelf_.configure(TptEQ::Mode::LowShelf,lowPositions[index(p.lowFrequency)],.7071067811865,
                        quantHalf(p.lowGainDb,-5.,5.));
    highShelf_.configure(TptEQ::Mode::HighShelf,highPositions[index(p.highFrequency)],.7071067811865,
                         quantHalf(p.highGainDb,-5.,5.));
    // Frequencies beyond representable Nyquist are effectively OUT at this sample rate;
    // never silently substitute a different labelled knob position.
    lpf_.configure(TptEQ::Mode::LowPass,lp>0.?std::min(lp,.45*fs):12000.,.7071067811865);
    lpfMix_.set(lp>0. && lp<.45*fs?1.:0.);
    output_.set(clamp(p.outputDb,-4.,4.));
    bypass_.set(p.in?1.:0.);
}
void Baxandall::Row::reset() noexcept {
    hpf_.clear();lowShelf_.clear();highShelf_.clear();lpf_.clear();
}
double Baxandall::Row::run(double x) noexcept {
    Stereo in{x,x};
    auto hp=hpf_.process(in);
    auto a=blend(in,hp,hpfMix_.next());
    a=lowShelf_.process(a);a=highShelf_.process(a);
    auto lp=lpf_.process(a);
    a=blend(a,lp,lpfMix_.next());
    const double y=a.l*dbToGain(output_.next());
    return x+(y-x)*bypass_.next();
}
void Baxandall::prepare(double fs) noexcept {
    fs_=clamp(fs,8000.,384000.);
    for(auto& x:ms_)x.prepare(fs_);
    for(auto& x:lr_)x.prepare(fs_);
    routing_.prepare(fs_,.014);routing_.reset(0.);
    set(BaxandallParameters{});
}
void Baxandall::set(const BaxandallParameters& p) noexcept {
    for(unsigned i=0;i<2;++i){ms_[i].set(p.rows[i],fs_);lr_[i].set(p.rows[i],fs_);}
    routing_.set(p.routing==BaxRouting::LeftRight?1.:0.);
}
void Baxandall::reset() noexcept {
    for(auto& x:ms_) x.reset();
    for(auto& x:lr_) x.reset();
}
Stereo Baxandall::process(Stereo x) noexcept {
    const double m=.5*(x.l+x.r),s=.5*(x.l-x.r);
    const double pm=ms_[0].run(m),ps=ms_[1].run(s);
    const Stereo ms{pm+ps,pm-ps};
    const Stereo lr{lr_[0].run(x.l),lr_[1].run(x.r)};
    return blend(ms,lr,routing_.next());
}

void Tape::prepare(double fs) noexcept {
    Module::prepare(fs);
    setWetLatency(OversampledProcessor::kLatency);
    saturator_.prepare(fs);
    bump_.prepare(fs);hfTrim_.prepare(fs);
    drive_.prepare(fs,.012);bias_.prepare(fs,.012);transientGain_.prepare(fs,.020);
    for(unsigned i=0;i<3;++i){types_[i].prepare(fs,.012);types_[i].reset(i==0?1.:0.);}
    drive_.reset(0.);bias_.reset(0.);transientGain_.reset(0.);
    set(params_);reset();
}
void Tape::set(const TapeParameters& p) noexcept {
    params_=p;setCommon(p.common);
    drive_.set(clamp(p.driveDb,-10.,10.));bias_.set(clamp(p.bias,-5.,5.));
    for(unsigned i=0;i<3;++i) types_[i].set(i==static_cast<unsigned>(p.type)?1.:0.);
    // Speed voicings are provisional engineering calibration, not approved knob edits.
    double bumpHz=68.,hfHz=12500.,bumpDb=1.5;
    switch(p.speed){
    case TapeSpeed::IPS7_5:bumpHz=48.;hfHz=7800.;bumpDb=2.3;break;
    case TapeSpeed::IPS15:bumpHz=68.;hfHz=12500.;bumpDb=1.5;break;
    case TapeSpeed::IPS30:bumpHz=94.;hfHz=16000.;bumpDb=.8;break;
    }
    bump_.configure(TptEQ::Mode::Peak,bumpHz,.65,bumpDb);
    hfTrim_.configure(TptEQ::Mode::HighShelf,hfHz,.7,clamp(p.hfTrimDb,-6.,6.));
}
void Tape::reset() noexcept {
    bump_.clear();hfTrim_.clear();saturator_.reset();saturator_.setMode(quality_);
    memory_.fill({});envelope_=0.;dc_={};resetDryDelay();
}
Stereo Tape::saturate(Stereo input,unsigned quality,void* raw) noexcept {
    auto& c=*static_cast<Context*>(raw);
    auto& self=*c.self;
    quality=std::min(quality,3u);
    const double alpha=std::exp(-1./std::max(8000.,self.fs_*static_cast<double>(1u<<quality)*.012));
    auto& mem=self.memory_[quality];
    mem.l=alpha*mem.l+(1.-alpha)*input.l;
    mem.r=alpha*mem.r+(1.-alpha)*input.r;
    const auto calc=[&](double x,double memory) noexcept {
        const double bias=c.bias*.10;
        const double d=c.drive;
        const double clean=(std::tanh(d*(x+.5*bias))-std::tanh(d*.5*bias))/std::tanh(d);
        const double old=(std::atan(d*(x+.8*bias+.10*memory))-
                          std::atan(d*(.8*bias+.10*memory)))/std::atan(d);
        const double modern=(std::tanh(d*(x+.2*bias+.035*memory))-
                             std::tanh(d*(.2*bias+.035*memory)))/std::tanh(d);
        return sanitized(c.weights[0]*clean+c.weights[1]*old+c.weights[2]*modern);
    };
    return {calc(input.l,mem.l),calc(input.r,mem.r)};
}
Stereo Tape::processWet(Stereo input) noexcept {
    auto pre=hfTrim_.process(bump_.process(input));
    const double target=magnitude(pre);
    const bool attack=target>envelope_;
    envelope_=target+smoothCoef(fs_,attack?.012:.140)*(envelope_-target);
    envelope_=sanitized(envelope_);
    const double overload=std::max(0.,gainToDb(envelope_)-(-4.));
    const double gr=std::min(3.0,overload*.18);
    transientGain_.set(-gr);
    const double comp=dbToGain(transientGain_.next());
    const double d=1.35*dbToGain(drive_.next()*.55);
    Context c{this,d,bias_.next(),{types_[0].next(),types_[1].next(),types_[2].next()}};
    const auto y=saturator_.process({pre.l*comp,pre.r*comp},&Tape::saturate,&c);
    // Very slow DC servo; no noise floor or wow/flutter introduced.
    const double a=1.-std::exp(-2.*kPi*5./fs_);
    dc_.l=sanitized(dc_.l+a*(y.l-dc_.l));dc_.r=sanitized(dc_.r+a*(y.r-dc_.r));
    return {y.l-dc_.l,y.r-dc_.r};
}

void GlobalStereo::prepare(double fs) noexcept {
    fs_=clamp(fs,8000.,384000.);
    sideLowpass_.prepare(fs_);sideHighpass_.prepare(fs_);
    width_.prepare(fs_,.012);monoMaker_.prepare(fs_,.012);
    width_.reset(1.);monoMaker_.reset(0.);
    set(GlobalParameters{});reset();
}
void GlobalStereo::set(const GlobalParameters& p) noexcept {
    width_.set(clamp(p.stereoWidth,0.,2.));
    sideLowpass_.configure(TptEQ::Mode::LowPass,clamp(p.widthFrequencyHz,800.,5000.),.7071067811865);
    const double cutoff=clamp(p.monoMakerHz,20.,300.);
    sideHighpass_.configure(TptEQ::Mode::HighPass,cutoff,.7071067811865);
    monoMaker_.set(p.monoMakerHz>0.?1.:0.);
}
void GlobalStereo::reset() noexcept {sideLowpass_.clear();sideHighpass_.clear();}
Stereo GlobalStereo::process(Stereo input) noexcept {
    const double mid=.5*(input.l+input.r),side=.5*(input.l-input.r);
    const double low=sideLowpass_.process({side,side}).l;
    const double width=width_.next();
    const double widened=side+(width-1.)*(side-low); // No widening below width cutoff
    const double hi=sideHighpass_.process({widened,widened}).l;
    const double shaped=widened+(hi-widened)*monoMaker_.next();
    return {mid+shaped,mid-shaped};
}

void MasterLimiter::prepare(double sampleRate) noexcept {
    fs_=clamp(sampleRate,8000.,384000.);
    audioLatency_=35u+static_cast<std::size_t>(std::ceil(fs_*.005));
    detector2x_.prepare();detector4x_.prepare();detector8x_.prepare();
    meter2x_.prepare();meter4x_.prepare();meter8x_.prepare();
    for(auto* s:{&inputGain_,&ceiling_,&bypass_,&lookahead_,&link_})s->prepare(fs_,.006);
    inputGain_.reset(1.);ceiling_.reset(1.);bypass_.reset(1.);
    lookahead_.reset(3.);link_.reset(1.);
    set(LimiterParameters{});reset();
}
void MasterLimiter::set(const LimiterParameters& p) noexcept {
    inputGain_.set(dbToGain(-clamp(p.thresholdDb,-24.,0.)));
    ceilingHard_=dbToGain(clamp(p.ceilingDbTP,-2.,0.));
    ceiling_.set(ceilingHard_);
    lookahead_.set(clamp(p.lookaheadMs,.1,5.));
    link_.set(clamp(p.stereoLink,0.,1.));
    bypass_.set(p.in?1.:0.);
    truePeak_=p.truePeak;autoRelease_=p.autoRelease;
    releaseMs_=clamp(p.releaseMs,10.,1000.);mode_=p.mode;
}
void MasterLimiter::reset() noexcept {
    audioBuffer_.fill({});dryBuffer_.fill({});peakBuffer_.fill({});
    audioCursor_=peakCursor_=0;gain_={1.,1.};reductionDb_={};outputTruePeak_=0.;
    releaseHold_.fill(0);safetyClipEvents_=0;
    detector2x_.reset();detector4x_.reset();detector8x_.reset();
    meter2x_.reset();meter4x_.reset();meter8x_.reset();
}
Stereo MasterLimiter::getPeak(Stereo input) noexcept {
    Stereo even{},odd{},q0{},q1{},q2{},q3{};
    detector2x_.up(input,even,odd);
    detector4x_.up(even,q0,q1);
    detector4x_.up(odd,q2,q3);
    Stereo result{};
    for(const Stereo q : {q0,q1,q2,q3}) {
        Stereo a{},b{};
        detector8x_.up(q,a,b);
        result.l=std::max({result.l,std::abs(a.l),std::abs(b.l)});
        result.r=std::max({result.r,std::abs(a.r),std::abs(b.r)});
    }
    return result;
}
Stereo MasterLimiter::measureOutput(Stereo output) noexcept {
    Stereo even{},odd{},q0{},q1{},q2{},q3{};
    meter2x_.up(output,even,odd);
    meter4x_.up(even,q0,q1);
    meter4x_.up(odd,q2,q3);
    double peak=std::max(std::abs(output.l),std::abs(output.r));
    for(const Stereo q : {q0,q1,q2,q3}) {
        Stereo a{},b{};
        meter8x_.up(q,a,b);
        peak=std::max({peak,std::abs(a.l),std::abs(a.r),std::abs(b.l),std::abs(b.r)});
    }
    outputTruePeak_=std::max(peak,outputTruePeak_*.99998);
    return output;
}

Stereo MasterLimiter::process(Stereo input) noexcept {
    const double pre=inputGain_.next();
    const Stereo driven{sanitized(input.l*pre),sanitized(input.r*pre)};
    const auto tp=getPeak(driven);
    peakBuffer_[peakCursor_]=tp;
    // The 8x FIR reconstruction detector has an inherent 35 host-sample delay.
    // Sample-peak mode is delayed equivalently to maintain identical timing.
    // Selected lookahead is the lead between target detection and delayed audio.
    const std::size_t maxAhead=audioLatency_-35u;
    const auto userAhead=static_cast<std::size_t>(std::round(clamp(lookahead_.next(),.1,5.)*.001*fs_));
    const auto offset=maxAhead-std::min(userAhead,maxAhead)+(truePeak_?0u:35u);
    const auto idx=(peakCursor_+kCapacity-(offset%kCapacity))%kCapacity;
    const Stereo detector=truePeak_?peakBuffer_[idx]:Stereo{std::abs(driven.l),std::abs(driven.r)};
    // For non-TP mode, use independently delayed raw samples rather than FIR peaks.
    // The raw path is stored in the same audio ring and delayed to match FIR timing.
    const auto rawIdx=(audioCursor_+kCapacity-(offset%kCapacity))%kCapacity;
    const auto raw=audioBuffer_[rawIdx];
    const Stereo p=truePeak_?detector:Stereo{std::abs(raw.l),std::abs(raw.r)};
    const double ceiling=std::min(ceilingHard_,ceiling_.next());
    const double linked=std::max(p.l,p.r);
    const double weight=forceLinked_?1.:link_.next();
    const double pl=p.l+(linked-p.l)*weight,pr=p.r+(linked-p.r)*weight;
    const double wantL=std::min(1.,ceiling/std::max(kSmall,pl));
    const double wantR=std::min(1.,ceiling/std::max(kSmall,pr));
    const double peakMax=std::max(pl,pr);
    const double decay=autoRelease_?clamp(80.+peakMax*90.,10.,1000.):releaseMs_;
    const double personality=mode_==LimiterMode::Punch?.7:(mode_==LimiterMode::Modern?1.3:1.);
    const double c=smoothCoef(fs_,.001*decay*personality);
    // A detected transient precedes its corresponding delayed audio by
    // userAhead samples. Holding the requested gain through that interval
    // prevents release recovery from raising the transient while it is still
    // travelling through the lookahead buffer. No dynamic storage or scans.
    auto track=[&](double requested,double& current,std::size_t& remaining) noexcept {
        if(requested<current) {
            current=requested;
            remaining=userAhead;
        } else if(remaining>0) {
            --remaining;
        } else {
            current=requested+c*(current-requested);
        }
    };
    track(wantL,gain_.l,releaseHold_[0]);
    track(wantR,gain_.r,releaseHold_[1]);
    gain_.l=clamp(gain_.l,0.,1.);gain_.r=clamp(gain_.r,0.,1.);
    reductionDb_={-gainToDb(gain_.l),-gainToDb(gain_.r)};
    audioBuffer_[audioCursor_]=driven;
    dryBuffer_[audioCursor_]=input;
    const auto delayedIndex=(audioCursor_+kCapacity-(audioLatency_%kCapacity))%kCapacity;
    const Stereo delayed=audioBuffer_[delayedIndex];
    const Stereo dry=dryBuffer_[delayedIndex];
    const double en=bypass_.next();
    Stereo wet{delayed.l*gain_.l,delayed.r*gain_.r};
    // Last-resort sample-domain guard: count every intervention so clipping
    // cannot masquerade as successful transparent gain reduction. Not TP proof.
    if (std::abs(wet.l)>ceiling || std::abs(wet.r)>ceiling) ++safetyClipEvents_;
    wet.l=clamp(wet.l,-ceiling,ceiling);wet.r=clamp(wet.r,-ceiling,ceiling);
    const auto out=blend(dry,wet,en);
    if(++audioCursor_==kCapacity)audioCursor_=0;
    if(++peakCursor_==kCapacity)peakCursor_=0;
    return measureOutput({sanitized(out.l),sanitized(out.r)});
}

void MasteringRack::prepare(double fs) noexcept {
    fs=clamp(fs,8000.,192000.); // v0.3 tested ceiling; higher rates require extended delay validation.
    firstSix.prepare(fs);baxandall.prepare(fs);tape.prepare(fs);stereo.prepare(fs);limiter.prepare(fs);
    input_.prepare(fs,.010);output_.prepare(fs,.010);bypass_.prepare(fs,.006);
    input_.reset(1.);output_.reset(1.);bypass_.reset(1.);
    setGlobal(global_);reset();
}
void MasteringRack::setGlobal(const GlobalParameters& p) noexcept {
    global_=p;
    input_.set(dbToGain(clamp(p.inputDb,-24.,24.)));
    output_.set(dbToGain(clamp(p.outputDb,-24.,12.)));
    bypass_.set(p.in?1.:0.);
    stereo.set(p);
    firstSix.setOversamplingMode(p.oversampling);
    tape.setOversamplingMode(p.oversampling);
    // With LF mono fold-down active, fully linked gain control preserves equal LF gains.
    // This is a conservative provisional rule; independent LF/MF detector routing is future work.
    limiter.setRequireLinkedGain(p.monoMakerHz>0.);
}
void MasteringRack::reset() noexcept {
    firstSix.reset();baxandall.reset();tape.reset();stereo.reset();limiter.reset();
    bypassBuffer_.fill({});bypassCursor_=0;
    nonFinite_.store(0,std::memory_order_relaxed);emergency_.store(0,std::memory_order_relaxed);
}
Stereo MasteringRack::guard(Stereo x) noexcept {
    for(double* v:{&x.l,&x.r}) {
        if(!std::isfinite(*v)) {nonFinite_.fetch_add(1,std::memory_order_relaxed);*v=0.;}
        else if(std::abs(*v)>4.) {emergency_.fetch_add(1,std::memory_order_relaxed);*v=clamp(*v,-4.,4.);}
        else if(std::abs(*v)<1.e-30) *v=0.;
    }
    return x;
}
Stereo MasteringRack::process(Stereo input) noexcept {
    input=guard(input);
    const double inGain=input_.next();
    auto x=guard({input.l*inGain,input.r*inGain});
    x=guard(firstSix.process(x));
    x=guard(baxandall.process(x));
    x=guard(tape.process(x));
    x=guard(stereo.process(x));
    const double outGain=output_.next();
    x=guard({x.l*outGain,x.r*outGain});
    const Stereo wet=guard(limiter.process(x));
    const std::size_t delay=latencySamples();
    bypassBuffer_[bypassCursor_]=input;
    const auto read=(bypassCursor_+bypassBuffer_.size()-(delay%bypassBuffer_.size()))%bypassBuffer_.size();
    const Stereo dry=bypassBuffer_[read];
    if(++bypassCursor_==bypassBuffer_.size())bypassCursor_=0;
    return guard(blend(dry,wet,bypass_.next()));
}
} // namespace steammaster
