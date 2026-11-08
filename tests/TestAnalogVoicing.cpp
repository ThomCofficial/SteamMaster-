#include "steammaster/DSP.hpp"
#include "steammaster/Mastering.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace steammaster;
static void check(bool ok,const char* msg){if(!ok){std::fprintf(stderr,"FAIL: %s\n",msg);std::exit(1);}}
static double response(PultecParameters p,double hz){
    Pultec eq;eq.prepare(48000);p.common.in=true;p.common.mix=1.;eq.set(p);
    double in2=0.,out2=0.;
    for(int i=0;i<48000;++i){double x=.1*std::sin(2.*3.141592653589793*hz*i/48000.);
        auto y=eq.process({x,x});if(i>24000){in2+=x*x;out2+=y.l*y.l;}}
    return 10.*std::log10(out2/in2);
}
int main(){
    PultecParameters p{};
    check(std::abs(response(p,100.))<.15,"neutral Pultec must be transparent");
    p.lowBoost=8.;check(response(p,40.)>3.,"low boost must add bass");
    p.lowAttenuation=8.;const double low=response(p,40.),upper=response(p,250.);
    check(low>upper+1.,"coupled low boost/cut must form bass hump and dip");
    p={};p.highBoost=8.;p.bandwidth=.3;
    check(response(p,10000.)>1.,"high boost must add upper presence");
    Tape t;t.prepare(48000);TapeParameters q{};q.common.in=true;q.common.mix=1.;q.driveDb=8.;t.set(q);
    double peak=0.;for(int i=0;i<48000;++i){auto y=t.process({.6*std::sin(i*.35),.6*std::sin(i*.35)});check(std::isfinite(y.l)&&std::isfinite(y.r),"tape must stay finite");peak=std::max(peak,std::abs(y.l));}
    check(peak<4.,"tape must not produce amplitude bursts");
    std::puts("Analog voicing checks passed");
}
