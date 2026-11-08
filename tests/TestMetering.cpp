#include "steammaster/Mastering.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace steammaster;
static void check(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(){
 try {
  for(double fs:{44100.,48000.,96000.,192000.}){
   MasterLimiter lim;lim.prepare(fs);
   LimiterParameters p;p.thresholdDb=-18.;p.ceilingDbTP=-2.;p.lookaheadMs=5.;p.truePeak=true;p.in=true;
   lim.set(p);
   double maxReduction=0.;
   for(int i=0;i<static_cast<int>(fs*.15);++i){
    const double x=1.5*std::sin(2.*kPi*997.*i/fs);
    auto out=lim.process({x,x});
    auto gr=lim.gainReductionDb();
    check(std::isfinite(out.l)&&std::isfinite(out.r),"Nonfinite limiter output");
    check(std::isfinite(gr.l)&&std::isfinite(gr.r),"Nonfinite gain reduction meter");
    check(gr.l>=0.&&gr.r>=0.,"Negative gain reduction meter");
    check(std::abs(gr.l-gr.r)<.001,"Linked stereo gain reduction mismatch");
    maxReduction=std::max(maxReduction,gr.l);
   }
   check(maxReduction>2.,"Gain reduction not reported during active limiting");
   p.in=false;lim.set(p);
   for(int i=0;i<static_cast<int>(fs*.1);++i){
    lim.process({.5,.5});
   }
   auto gr=lim.gainReductionDb();
   check(gr.l<.01&&gr.r<.01,"Bypassed limiter reports active gain reduction");
   lim.reset();
   gr=lim.gainReductionDb();
   check(gr.l==0.&&gr.r==0.,"Reset did not clear gain reduction meter");
   std::cout<<fs<<" Hz metering OK\n";
  }
  std::cout<<"Metering tests passed\n";
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
