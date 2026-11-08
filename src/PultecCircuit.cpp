#include "steammaster/PultecCircuit.hpp"
#include <algorithm>
#include <cmath>
#include <complex>

namespace steammaster {
namespace {
constexpr double pi=3.14159265358979323846;
double capCut(int hz) noexcept {
    switch(hz) {case 30:return 27e-9;case 100:return 12e-9;default:return 8.2e-9;}
}
double capBoost(int hz) noexcept {
    switch(hz) {case 30:return .82e-6;case 100:return .33e-6;default:return .22e-6;}
}
double capHighCut(int hz) noexcept {
    switch(hz) {case 5000:return .22e-6;case 20000:return .068e-6;default:return .068e-6;}
}
// Original coil/cap selector values given in the independent Sowter schematic.
void highResonance(int hz,double& c,double& l) noexcept {
    switch(hz) {
        case 8000: c=8.2e-9;l=47e-3;break;
        case 16000:c=3.3e-9;l=27e-3;break;
        default: c=5.6e-9;l=27e-3;break;
    }
}
double positive(double x) noexcept {return std::max(0.,std::min(1.,std::isfinite(x)?x:0.));}
}

void PultecCircuit::Model::configure(double fs,int lf,double lb,double la,int hf,double hb,double width,int hcf,double hca) noexcept {
    const double cut=positive(la/10.),boost=positive(lb/10.);
    rcut=cut>0.?20000./std::pow(cut,1.35):0.;
    rboost=boost>0.?2000./std::pow(boost,1.6):0.;
    gc=cut>0.?2.*fs*capCut(lf):0.;
    gb=boost>0.?2.*fs*capBoost(lf):0.;
    ec=gc>0.?1./(1.+gc*rcut):0.;
    eb=gb>0.?1./(1.+gb*rboost):0.;
    const double Gc=gc*ec,Gb=gb*eb;
    const double Gs=1./kRs,Gpath=1./kRseries+Gc,Gl=1./kRload;
    const double a=Gs+Gpath,d=Gpath+Gl+Gb;
    m00=a;m01=-Gpath;m11=d;
    invDet=1./std::max(1.e-20,a*d-Gpath*Gpath);

    // Compensate the passive network insertion loss at 4 kHz (rather than
    // injecting a fake low shelf): nonlinear passive branches themselves
    // determine the shape of the simultaneous bass boost/attenuation contour.
    const std::complex<double> s{0.,2.*pi*4000.};
    const auto cutY=gc>0.?s*capCut(lf)/(1.+s*capCut(lf)*rcut):std::complex<double>{0.,0.};
    const auto boostY=gb>0.?s*capBoost(lf)/(1.+s*capBoost(lf)*rboost):std::complex<double>{0.,0.};
    const auto Gp=std::complex<double>{1./kRseries,0.}+cutY;
    const auto H=Gs*Gp/(Gs*(Gp+Gl+boostY)+Gp*(Gl+boostY));
    makeup=1./std::max(1.e-8,std::abs(H));

    // Series-RLC current is a bandpass resonator. With the passive-network
    // voltage divider it gives the EQP family its characteristic HF peak.
    double C=0.,L=0.;highResonance(hf,C,L);
    const double f0=1./(2.*pi*std::sqrt(L*C));
    const double fSafe=std::min(f0,fs*.44);
    const double w0=2.*fs*std::tan(pi*fSafe/fs);
    const double R=900.+6000.*positive(width);  // effective series damping
    const double Q=std::max(.12,(2.*pi*f0*L)/R);
    const double damping=w0/Q;
    const double amount=std::pow(positive(hb/10.),1.2);
    const double maxDb=(18.-4.*positive(width))*amount;
    const double strength=std::pow(10.,maxDb/20.)-1.;
    const double k=2.*fs;
    const double den0=k*k+damping*k+w0*w0;
    hB0=(k*k+(1.+strength)*damping*k+w0*w0)/den0;
    hB1=(-2.*k*k+2.*w0*w0)/den0;
    hB2=(k*k-(1.+strength)*damping*k+w0*w0)/den0;
    hA1=(-2.*k*k+2.*w0*w0)/den0;
    hA2=(k*k-damping*k+w0*w0)/den0;

    // RC shelving stage. 75-ohm and 1k pot family / selector capacitors from
    // Sowter. An equivalent two-resistor transfer substitutes for the full
    // loaded high-frequency attenuation bridge; no invented tube saturation.
    const double cutAmount=positive(hca/10.);
    const double shelfHighGain=std::pow(10.,-16.*std::pow(cutAmount,1.2)/20.);
    const double selectedCap=capHighCut(hcf);
    const double poleHz=std::min(fs*.43,static_cast<double>(hcf)*.63);
    const double Rp=1./(2.*fs*std::tan(pi*poleHz/fs)*selectedCap);
    const double Rz=Rp*shelfHighGain;
    const double prewarp=2.*fs;
    const double zp=Rz*selectedCap*prewarp,pp=Rp*selectedCap*prewarp;
    cB0=(1.+zp)/(1.+pp);
    cB1=(1.-zp)/(1.+pp);
    cA1=(1.-pp)/(1.+pp);
}

double PultecCircuit::Model::sample(double x,Channel& ch) noexcept {
    const double jc=ec*(-gc*ch.cut.capacitorVoltage-ch.cut.current);
    const double jb=eb*(-gb*ch.boost.capacitorVoltage-ch.boost.current);
    const double source=(x/kRs)-jc;
    const double sink=jc-jb;
    const double v0=(source*m11-m01*sink)*invDet;
    const double v1=(m00*sink-m01*source)*invDet;
    if(gc>0.) {
        const double ic=(gc*ec)*(v0-v1)+jc;
        ch.cut.capacitorVoltage=(v0-v1)-rcut*ic;
        ch.cut.current=ic;
    }
    if(gb>0.) {
        const double ib=(gb*eb)*v1+jb;
        ch.boost.capacitorVoltage=v1-rboost*ib;
        ch.boost.current=ib;
    }
    const double low=v1*makeup;
    const double high=hB0*low+ch.highBoostZ1;
    ch.highBoostZ1=hB1*low-hA1*high+ch.highBoostZ2;
    ch.highBoostZ2=hB2*low-hA2*high;
    const double output=cB0*high+cB1*ch.highCutX1-cA1*ch.highCutY1;
    ch.highCutX1=high;
    ch.highCutY1=output;
    return std::isfinite(output)?std::clamp(output,-1.e6,1.e6):0.;
}
Stereo PultecCircuit::Model::process(Stereo in) noexcept {
    return {sample(in.l,channels[0]),sample(in.r,channels[1])};
}
void PultecCircuit::prepare(double fs) noexcept {
    fs_=std::clamp(std::isfinite(fs)?fs:48000.,8000.,384000.);
    fadeStep_=1./(fs_*.02);
    active_.configure(fs_,60,0.,0.,12000,0.,.5,10000,0.);
    incoming_=active_;
    fade_=1.;settings_={}; initialized_=true;
}
void PultecCircuit::configure(int lf,double lb,double la,int hf,double hb,double width,int hcf,double hca) noexcept {
    if(!initialized_)prepare(48000.);
    Settings s{};
    s.lf=(lf==30||lf==60||lf==100)?lf:60;
    s.hf=(hf==8000||hf==12000||hf==16000)?hf:12000;
    s.hcf=(hcf==5000||hcf==10000||hcf==20000)?hcf:10000;
    s.lb=positive(lb/10.)*10.;s.la=positive(la/10.)*10.;
    s.hb=positive(hb/10.)*10.;s.bw=positive(width);s.hca=positive(hca/10.)*10.;
    if(s==settings_)return;
    if(fade_<1.) {
        // Continue from the nearest currently audible network rather than
        // starting a new transition from a stale control state.
        if(fade_>=.5)active_=incoming_;
    }
    incoming_=active_;
    incoming_.configure(fs_,s.lf,s.lb,s.la,s.hf,s.hb,s.bw,s.hcf,s.hca);
    settings_=s;
    fade_=0.;
}
Stereo PultecCircuit::process(Stereo in) noexcept {
    if(!initialized_)return in;
    if(fade_>=1.)return active_.process(in);
    const auto a=active_.process(in);
    const auto b=incoming_.process(in);
    fade_=std::min(1.,fade_+fadeStep_);
    const auto out=Stereo{a.l+(b.l-a.l)*fade_,a.r+(b.r-a.r)*fade_};
    if(fade_>=1.)active_=incoming_;
    return out;
}
void PultecCircuit::reset() noexcept {
    // A reset must preserve the latest selected controls, including during a
    // transition (e.g. Cubase transport stop after an automated parameter).
    if(fade_<1.)active_=incoming_;
    active_.clear();incoming_=active_;fade_=1.;
}
} // namespace steammaster
