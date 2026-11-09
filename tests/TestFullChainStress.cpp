#include "steammaster/Mastering.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
using namespace steammaster;
int main(){
  std::mt19937_64 rng(0x515445414dULL);
  auto unit=[&](){return std::generate_canonical<double,53>(rng);};
  auto range=[&](double lo,double hi){return lo+(hi-lo)*unit();};
  std::uint64_t total=0;
  for(double fs:{44100.,48000.,96000.,192000.}) {
    auto rack=std::make_unique<MasteringRack>();
    rack->prepare(fs);
    for(int block=0;block<75;++block){
      GlobalParameters g;g.inputDb=range(-24,24);g.outputDb=range(-24,12);g.in=(block%13!=0);g.oversampling=OversamplingMode(block%4);g.monoMakerHz=(block%3?range(20,300):0);g.stereoWidth=range(0,2);g.widthFrequencyHz=range(800,5000);rack->setGlobal(g);
      OceanParameters o;o.common={block%5!=0,range(0,1)};o.thresholdDb=range(-40,10);o.ratio=OceanRatio(block%6);o.attackMs=range(25,70);o.recovery=OceanRecovery(block%5);o.scHpfHz=range(0,500);rack->firstSix.ocean.set(o);
      FurnaceParameters f;f.common={block%6!=0,range(0,1)};f.thresholdDb=range(-15,15);f.ratio=FurnaceRatio(block%3);f.attack=FurnaceAttack(block%6);f.release=FurnaceRelease(block%5);f.scHpfHz=range(0,500);rack->firstSix.furnace.set(f);
      ParametricParameters eq;eq.common={block%7!=0,range(0,1)};eq.outputDb=range(-12,12);for(int i=0;i<4;++i){eq.bands[size_t(i)].frequencyHz=range(20,20000);eq.bands[size_t(i)].q=range(.2,8);eq.bands[size_t(i)].gainDb=range(-12,12);eq.bands[size_t(i)].mode=(i==0?TptEQ::Mode::LowShelf:(i==3?TptEQ::Mode::HighShelf:TptEQ::Mode::Peak));}rack->firstSix.parametric.set(eq);
      PultecParameters p;p.common={block%8!=0,range(0,1)};p.lowFrequencyHz=(block%3==0?30:block%3==1?60:100);p.lowBoost=range(0,10);p.lowAttenuation=range(0,10);p.highBoostFrequencyHz=(block%3==0?8000:block%3==1?12000:16000);p.highBoost=range(0,10);p.bandwidth=unit();p.highAttenuationFrequencyHz=(block%3==0?5000:block%3==1?10000:20000);p.highAttenuation=range(0,10);p.outputDb=range(-12,12);rack->firstSix.pultec.set(p);
      ShadowParameters sh;sh.common={block%9!=0,range(0,1)};sh.opticalIn=block%3!=0;sh.discreteIn=block%4!=0;sh.opticalThresholdDb=range(-40,10);sh.opticalMakeupDb=range(-12,18);sh.discreteThresholdDb=range(-40,10);sh.discreteGainDb=range(-12,18);sh.discreteRatio=DiscreteRatio(block%6);sh.discreteAttack=DiscreteAttack(block%6);sh.discreteRecovery=DiscreteRecovery(block%7);sh.scHpfHz=range(0,500);sh.outputDb=range(-12,12);rack->firstSix.shadow.set(sh);
      ExciterParameters ex;ex.common={block%10!=0,range(0,1)};ex.mode=ExciterMode(block%3);ex.lowDrive=range(0,10);ex.highDrive=range(0,10);ex.lowFrequencyHz=range(20,800);ex.highFrequencyHz=range(1000,18000);ex.crossoverHz=range(100,5000);ex.outputDb=range(-12,12);rack->firstSix.exciter.set(ex);
      BaxandallParameters bax;bax.routing=BaxRouting(block%2);for(auto& row:bax.rows){row.hpf=BaxHPF(block%8);row.lowFrequency=BaxLowShelf(block%8);row.lowGainDb=range(-5,5);row.highFrequency=BaxHighShelf(block%8);row.highGainDb=range(-5,5);row.lpf=BaxLPF(block%8);row.outputDb=range(-4,4);row.in=block%4!=0;}rack->baxandall.set(bax);
      TapeParameters t;t.common={block%11!=0,range(0,1)};t.type=TapeType(block%3);t.speed=TapeSpeed(block%3);t.driveDb=range(-10,10);t.bias=range(-5,5);t.hfTrimDb=range(-6,6);rack->tape.set(t);
      LimiterParameters l;l.in=block%12!=0;l.mode=LimiterMode(block%3);l.thresholdDb=range(-24,0);l.ceilingDbTP=range(-2,0);l.releaseMs=range(10,1000);l.autoRelease=block%2!=0;l.lookaheadMs=range(.1,5);l.stereoLink=unit();l.truePeak=block%2!=0;rack->limiter.set(l);
      for(int i=0;i<256;++i){const double x=range(-1.2,1.2);auto y=rack->process({x,range(-1.2,1.2)});if(!std::isfinite(y.l)||!std::isfinite(y.r)||std::abs(y.l)>4.01||std::abs(y.r)>4.01)throw std::runtime_error("nonfinite/out of bounds");++total;}
    }
    rack->reset();for(int i=0;i<4096;++i){const auto y=rack->process({0,0});if(!std::isfinite(y.l)||!std::isfinite(y.r))throw std::runtime_error("reset instability");}
    std::cout<<"sampleRate="<<fs<<" invalid="<<rack->invalidEvents()<<" emergency="<<rack->emergencyEvents()<<"\n";
    if(rack->invalidEvents()!=0)throw std::runtime_error("nonfinite internal DSP event");
  }
  std::cout<<"PASS full chain "<<total<<" samples; 4 rates, 300 randomized all-module settings\n";
}
