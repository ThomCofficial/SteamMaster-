#include "steammaster/Mastering.hpp"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
using namespace steammaster;
int main(int argc,char**argv){
 constexpr double fs=48000.;MasterLimiter lim;lim.prepare(fs);
 LimiterParameters p;p.thresholdDb=-12.;p.ceilingDbTP=-1.;p.lookaheadMs=5.;p.stereoLink=0.;p.truePeak=true;lim.set(p);
 std::ofstream o(argc>1?argv[1]:"tp-test.f32",std::ios::binary);
 if(!o)return 1;
 std::uint64_t seed=0x1234aaccbcULL;
 auto noise=[&](){seed^=seed<<13;seed^=seed>>7;seed^=seed<<17;return 2.*((seed>>16)&0xffff)/65535.-1.;};
 for(int i=0;i<48000*3;++i){
  if(i==48000){p.mode=LimiterMode::Punch;p.lookaheadMs=.1;lim.set(p);}
  if(i==96000){p.mode=LimiterMode::Modern;p.ceilingDbTP=-1.;p.lookaheadMs=5.;lim.set(p);}
  double a=0.92*std::sin(2*kPi*11000.*i/fs)+.42*noise()+((i%977)==100?2.2:0.);
  double b=.92*std::sin(2*kPi*7900.*i/fs+.3)+.39*noise()-((i%877)==91?1.7:0.);
  Stereo s=lim.process({a,b});float f[2]={static_cast<float>(s.l),static_cast<float>(s.r)};o.write(reinterpret_cast<const char*>(f),sizeof f);
 }
}
