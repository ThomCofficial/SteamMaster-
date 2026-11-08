#pragma once
#include <array>
#include "steammaster/Core.hpp"

namespace steammaster {

// Independent, original circuit-derived approximation of the Pultec EQP-1A
// passive filter sections. Passive low RC network uses a two-node trapezoidal
// nodal solver and branch currents. Upper controls use physically parameterized
// RLC resonator and RC shelf. These are reduced networks: NOT the complete
// proprietary hardware, transformer/tube model, or the GPL EQP-WDF-1A source.
class PultecCircuit {
public:
    void prepare(double sampleRate) noexcept;
    void configure(int lowHz, double boost, double atten,
                   int highBoostHz, double highBoost, double width,
                   int highCutHz, double highCut) noexcept;
    Stereo process(Stereo in) noexcept;
    void reset() noexcept;
private:
    struct Branch { double capacitorVoltage=0.0, current=0.0; };
    struct Channel {
        Branch cut{}, boost{};
        double highBoostZ1=0.0, highBoostZ2=0.0;
        double highCutX1=0.0, highCutY1=0.0;
    };
    struct Model {
        // Analogue passive network: 620R source, 80kR series, 10kR load.
        // The source/control loading is reduced and calibrated independently;
        // it is NOT an exact copy of the original five-port EQP-1A network.
        static constexpr double kRs = 620., kRseries = 80000., kRload = 10000.;
        double gc=0., gb=0., ec=0., eb=0., rcut=0., rboost=0.;
        double m00=0., m01=0., m11=0., invDet=0., makeup=1.;
        double hB0=1.,hB1=0.,hB2=0.,hA1=0.,hA2=0.;
        double cB0=1.,cB1=0.,cA1=0.;
        std::array<Channel,2> channels{};
        void configure(double fs,int lf,double lb,double la,int hf,double hb,double width,int hcf,double hca) noexcept;
        Stereo process(Stereo in) noexcept;
        void clear() noexcept { channels={}; }
        double sample(double x,Channel& c) noexcept;
    };
    struct Settings {
        int lf=60,hf=12000,hcf=10000;
        double lb=0.,la=0.,hb=0.,bw=.5,hca=0.;
        bool operator==(const Settings& o) const noexcept {
            return lf==o.lf && hf==o.hf && hcf==o.hcf && lb==o.lb && la==o.la && hb==o.hb && bw==o.bw && hca==o.hca;
        }
    } settings_{};
    Model active_{},incoming_{};
    double fs_=48000.,fade_=1.,fadeStep_=1./(48000*.02);
    bool initialized_=false;
};

} // namespace steammaster
