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
    hpf_.prepare(fs);lpf_.prepare(fs);
    lowShelf_.prepare(fs,BaxShelf::Band::Low);highShelf_.prepare(fs,BaxShelf::Band::High);
    for(auto* x:{&hpfMix_,&lpfMix_,&bypass_,&output_})x->prepare(fs,.012);
    hpfMix_.reset(0.);lpfMix_.reset(0.);bypass_.reset(1.);output_.reset(0.);
}
void Baxandall::Row::set(const BaxRowParameters& p,double fs) noexcept {
    const double hp=hpfPositions[index(p.hpf)], lp=lpfPositions[index(p.lpf)];
    hpf_.configure(TptEQ::Mode::HighPass,hp>0. ? hp:24.,.7071067811865);
    hpfMix_.set(hp>0.?1.:0.);
    lowShelf_.set(lowPositions[index(p.lowFrequency)],quantHalf(p.lowGainDb,-5.,5.));
    highShelf_.set(highPositions[index(p.highFrequency)],quantHalf(p.highGainDb,-5.,5.));
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
    a.l=highShelf_.process(lowShelf_.process(a.l));
    a.r=a.l;
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
    saturator_.prepare(fs);magnetics_.prepare(fs);
    dcAlpha_=1.-std::exp(-2.*kPi*5./fs_);
    bump_.prepare(fs);reproLoss_.prepare(fs);hfTrim_.prepare(fs);
    drive_.prepare(fs,.012);bias_.prepare(fs,.012);
    speedFactor_.prepare(fs,.018);reproMakeup_.prepare(fs,.020);transientGain_.prepare(fs,.020);
    for(unsigned i=0;i<3;++i){types_[i].prepare(fs,.020);types_[i].reset(i==0?1.:0.);}
    drive_.reset(0.);bias_.reset(0.);speedFactor_.reset(1.);reproMakeup_.reset(1.);transientGain_.reset(0.);
    set(params_);reset();
}
void Tape::set(const TapeParameters& p) noexcept {
    params_=p;setCommon(p.common);
    drive_.set(clamp(p.driveDb,-10.,10.));bias_.set(clamp(p.bias,-5.,5.));
    const unsigned type=std::min(2u,static_cast<unsigned>(p.type));
    for(unsigned i=0;i<3;++i) types_[i].set(i==type?1.:0.);
    double bumpHz=70.,hfHz=14000.,bumpDb=1.25,lossHz=14000.,lossDb=-1.15,speed=1.0;
    switch(p.speed){
    case TapeSpeed::IPS7_5:
        bumpHz=46.;hfHz=8700.;bumpDb=2.1;lossHz=9000.;lossDb=-2.8;speed=1.35;break;
    case TapeSpeed::IPS15:
        bumpHz=70.;hfHz=14000.;bumpDb=1.25;lossHz=14000.;lossDb=-1.15;speed=1.0;break;
    case TapeSpeed::IPS30:
        bumpHz=102.;hfHz=18000.;bumpDb=.65;lossHz=18500.;lossDb=-.30;speed=.67;break;
    }
    // Modern high-headroom mode has a slightly more restrained head bump,
    // Vintage has a slightly more emphatic resonance. These values are not
    // hardware-measured profiles for any identifiable ATR/A800 machine.
    if(type==1)bumpDb*=1.2;
    if(type==2)bumpDb*=.60;
    bump_.configure(TptEQ::Mode::Peak,bumpHz,.62,bumpDb);
    // Replay loss happens AFTER magnetic saturation, while HF Trim is a
    // separate post-repro user EQ. Overbias increases HF loss.
    lossDb+=p.bias>0.?-.26*clamp(p.bias,0.,5.):.09*clamp(-p.bias,0.,5.);
    reproLoss_.configure(TptEQ::Mode::HighShelf,lossHz,.7071067811865,lossDb);
    hfTrim_.configure(TptEQ::Mode::HighShelf,hfHz,.7071067811865,clamp(p.hfTrimDb,-6.,6.));
    speedFactor_.set(speed);
    // Level compensation at low programme level (-10 dB drive), measured
    // against the *own modeled* 1 kHz response at 48 kHz. This prevents
    // false loudness advantage between machine voicings/speeds; NOT a
    // hardware-measured flux calibration or extra user control.
    constexpr double makeupsDb[3][3]={{1.07,.77,.50},{1.58,1.13,.73},{.26,.19,.12}};
    const unsigned sp=(p.speed==TapeSpeed::IPS7_5?0u:(p.speed==TapeSpeed::IPS15?1u:2u));
    reproMakeup_.set(dbToGain(makeupsDb[type][sp]));
}
void Tape::reset() noexcept {
    bump_.clear();reproLoss_.clear();hfTrim_.clear();
    saturator_.reset();saturator_.setMode(quality_);magnetics_.reset();
    envelope_=0.;dc_={};resetDryDelay();
}
Stereo Tape::saturate(Stereo input,unsigned quality,void* raw) noexcept {
    auto& c=*static_cast<Context*>(raw);
    return c.self->magnetics_.process(input,quality,c.drive,c.bias,c.speedFactor,c.weights);
}
Stereo Tape::processWet(Stereo input) noexcept {
    const double target=magnitude(input);
    const bool attack=target>envelope_;
    envelope_=target+smoothCoef(fs_,attack?.012:.140)*(envelope_-target);
    envelope_=sanitized(envelope_);
    const double weights[3]={types_[0].next(),types_[1].next(),types_[2].next()};
    const double sum=std::max(1.e-9,weights[0]+weights[1]+weights[2]);
    // Amount of tape compression increases with flux (drive and programme
    // level), with vintage ATR-style slightly more compressive. Linked stereo
    // detection prevents stereo image modulation of common transients.
    const double threshold=(-4.*weights[0]-7.*weights[1]+0.*weights[2])/sum;
    const double slope=(.14*weights[0]+.20*weights[1]+.055*weights[2])/sum;
    const double maxGr=(3.0*weights[0]+4.0*weights[1]+1.5*weights[2])/sum;
    const double overload=std::max(0.,gainToDb(envelope_)-threshold);
    transientGain_.set(-std::min(maxGr,overload*slope));
    const double comp=dbToGain(transientGain_.next());
    const double d=1.26*dbToGain(drive_.next()*.55);
    Context c{this,d,bias_.next(),speedFactor_.next(),
              {weights[0]/sum,weights[1]/sum,weights[2]/sum}};
    const auto magnetic=saturator_.process({input.l*comp,input.r*comp},&Tape::saturate,&c);
    const auto playback=hfTrim_.process(reproLoss_.process(bump_.process(magnetic)));
    // Very slow DC servo and no gratuitous tape noise or wow/flutter.
    dc_.l=sanitized(dc_.l+dcAlpha_*(playback.l-dc_.l));
    dc_.r=sanitized(dc_.r+dcAlpha_*(playback.r-dc_.r));
    const double makeup=reproMakeup_.next();
    return {(playback.l-dc_.l)*makeup,(playback.r-dc_.r)*makeup};
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
    // Additional fixed post-limiter lookahead for true-peak correction;
    // kept in the latency even with TP OFF so mode changes are click-free.
    postLatency_=35u+static_cast<std::size_t>(std::ceil(fs_*.002));
    detector2x_.prepare();detector4x_.prepare();detector8x_.prepare();
    meter2x_.prepare();meter4x_.prepare();meter8x_.prepare();
    post2x_.prepare();post4x_.prepare();post8x_.prepare();
    for(auto* s:{&inputGain_,&ceiling_,&bypass_,&lookahead_,&link_,&postMix_})s->prepare(fs_,.006);
    inputGain_.reset(1.);ceiling_.reset(1.);bypass_.reset(1.);
    lookahead_.reset(3.);link_.reset(1.);postMix_.reset(1.);
    set(LimiterParameters{});reset();
}
void MasterLimiter::set(const LimiterParameters& p) noexcept {
    inputGain_.set(dbToGain(-clamp(p.thresholdDb,-24.,0.)));
    ceilingHard_=dbToGain(clamp(p.ceilingDbTP,-2.,0.));
    ceiling_.set(ceilingHard_);
    lookahead_.set(clamp(p.lookaheadMs,.1,5.));
    link_.set(clamp(p.stereoLink,0.,1.));
    bypass_.set(p.in?1.:0.);
    postMix_.set(p.in&&p.truePeak?1.:0.);
    truePeak_=p.truePeak;autoRelease_=p.autoRelease;
    releaseMs_=clamp(p.releaseMs,10.,1000.);mode_=p.mode;
}
void MasterLimiter::reset() noexcept {
    audioBuffer_.fill({});dryBuffer_.fill({});peakBuffer_.fill({});postBuffer_.fill({});
    audioCursor_=peakCursor_=postCursor_=0;gain_={1.,1.};postGain_={1.,1.};reductionDb_={};prePostEffectiveGain_={1.,1.};outputTruePeak_=0.;
    releaseHold_.fill(0);postHold_.fill(0);safetyClipEvents_=0;
    detector2x_.reset();detector4x_.reset();detector8x_.reset();
    meter2x_.reset();meter4x_.reset();meter8x_.reset();
    post2x_.reset();post4x_.reset();post8x_.reset();
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
Stereo MasterLimiter::postPeak(Stereo input) noexcept {
    // This completely independent 8x detector checks the *already limited*
    // waveform, including any ISP created by modulation and peak guards.
    Stereo even{},odd{},q0{},q1{},q2{},q3{};
    post2x_.up(input,even,odd);
    post4x_.up(even,q0,q1);
    post4x_.up(odd,q2,q3);
    Stereo result{std::abs(input.l),std::abs(input.r)};
    for(const Stereo q : {q0,q1,q2,q3}) {
        Stereo a{},b{};
        post8x_.up(q,a,b);
        result.l=std::max({result.l,std::abs(a.l),std::abs(b.l)});
        result.r=std::max({result.r,std::abs(a.r),std::abs(b.r)});
    }
    return result;
}
Stereo MasterLimiter::postTruePeakGuard(Stereo input,double ceiling) noexcept {
    // Causal correction stage. The detector sees a reconstructed peak at a
    // 35-sample FIR delay; its associated waveform emerges from the post
    // delay another 2ms later, allowing correction before output. This stage
    // carries NO new user parameters or GUI features.
    const auto p=postPeak(input);
    const double target=ceiling*dbToGain(-.15); // small interpolation margin
    const double linked=std::max(p.l,p.r);
    const double weight=forceLinked_?1.:link_.value();
    const double l=p.l+(linked-p.l)*weight;
    const double r=p.r+(linked-p.r)*weight;
    const Stereo requested{std::min(1.,target/std::max(kSmall,l)),
                           std::min(1.,target/std::max(kSmall,r))};
    const std::size_t ahead=postLatency_-35u;
    const double release=smoothCoef(fs_,.045);
    auto follow=[&](double needed,double& gain,std::size_t& hold) noexcept {
        if(needed<gain){ gain=needed;hold=ahead; }
        else if(hold>0){ --hold; }
        else { gain=needed+release*(gain-needed); }
        gain=clamp(gain,0.,1.);
    };
    follow(requested.l,postGain_.l,postHold_[0]);
    follow(requested.r,postGain_.r,postHold_[1]);
    postBuffer_[postCursor_]=input;
    const auto read=(postCursor_+kCapacity-(postLatency_%kCapacity))%kCapacity;
    const auto delayed=postBuffer_[read];
    if(++postCursor_==kCapacity)postCursor_=0;
    const double mix=postMix_.next();
    // Meter the actual applied attenuation of BOTH gain-control stages.
    // The bypass crossfade and post-stage crossfade are accounted for; the
    // threshold's pre-gain is deliberately excluded (it is not reduction).
    const double finalL=clamp(prePostEffectiveGain_.l*(1.-mix+mix*postGain_.l),1.e-15,1.);
    const double finalR=clamp(prePostEffectiveGain_.r*(1.-mix+mix*postGain_.r),1.e-15,1.);
    reductionDb_={-gainToDb(finalL),-gainToDb(finalR)};
    return {delayed.l*(1.-mix+mix*postGain_.l),
            delayed.r*(1.-mix+mix*postGain_.r)};
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
    audioBuffer_[audioCursor_]=driven;
    dryBuffer_[audioCursor_]=input;
    const auto delayedIndex=(audioCursor_+kCapacity-(audioLatency_%kCapacity))%kCapacity;
    const Stereo delayed=audioBuffer_[delayedIndex];
    const Stereo dry=dryBuffer_[delayedIndex];
    const double en=bypass_.next();
    prePostEffectiveGain_={1.-en+en*gain_.l,1.-en+en*gain_.r};
    Stereo wet{delayed.l*gain_.l,delayed.r*gain_.r};
    // Last-resort sample-domain guard: count every intervention so clipping
    // cannot masquerade as successful transparent gain reduction. Not TP proof.
    if (std::abs(wet.l)>ceiling || std::abs(wet.r)>ceiling) ++safetyClipEvents_;
    wet.l=clamp(wet.l,-ceiling,ceiling);wet.r=clamp(wet.r,-ceiling,ceiling);
    const auto out=blend(dry,wet,en);
    if(++audioCursor_==kCapacity)audioCursor_=0;
    if(++peakCursor_==kCapacity)peakCursor_=0;
    return measureOutput(postTruePeakGuard({sanitized(out.l),sanitized(out.r)},ceilingHard_));
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
