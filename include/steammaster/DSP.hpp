#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include "steammaster/Core.hpp"
#include "steammaster/Oversampling.hpp"
#include "steammaster/PultecCircuit.hpp"

namespace steammaster {

// All setters are called on the audio thread at a block boundary. UI/host-to-audio
// communication must use an SPSC queue or lock-free parameter snapshot upstream.
class Smoother {
public:
    void prepare(double sampleRate, double seconds = 0.010) noexcept;
    void reset(double value) noexcept;
    void set(double target) noexcept;
    double next() noexcept;
    double value() const noexcept { return current_; }
private:
    double current_ = 0.0, target_ = 0.0, alpha_ = 1.0;
};

class DetectorHPF {
public:
    void prepare(double sampleRate) noexcept;
    void setFrequency(double hz) noexcept;
    void reset() noexcept { low_={}; }
    Stereo process(Stereo in) noexcept;
private:
    double fs_ = 48000.0, coeff_ = 0.0;
    Stereo low_{};
    Smoother wet_{};
};

// Stable TPT state-variable filter. Control targets are smoothed in topology
// parameters (g, k, m0/m1/m2), rather than interpolating arbitrary biquad poles.
class TptEQ {
public:
    enum class Mode { Peak, LowShelf, HighShelf, LowPass, HighPass };
    void prepare(double sampleRate) noexcept;
    void configure(Mode mode, double hz, double q, double gainDb = 0.0) noexcept;
    Stereo process(Stereo in) noexcept;
    void clear() noexcept;
private:
    struct State { double ic1 = 0.0, ic2 = 0.0; };
    double run(double x, State& z) noexcept;
    double fs_ = 48000.0;
    State l_{}, r_{};
    Smoother g_{}, k_{}, m0_{}, m1_{}, m2_{};
};

struct CommonControls { bool in = true; double mix = 1.0; };
class Module {
public:
    virtual ~Module() = default;
    virtual void prepare(double sampleRate) noexcept;
    void setCommon(CommonControls p) noexcept;
    Stereo process(Stereo in) noexcept;
    virtual void reset() noexcept = 0;
protected:
    virtual Stereo processWet(Stereo in) noexcept = 0;
    void setWetLatency(std::size_t samples) noexcept { dryDelay_.prepare(samples); }
    void resetDryDelay() noexcept { dryDelay_.reset(); }
    double fs_ = 48000.0;
    Smoother enabled_{}, mix_{};
    StereoDelay dryDelay_{};
};

enum class OceanRatio { R1_2, R1_5, R2, R3, R4, R6 };
enum class OceanRecovery { S0_2, S0_4, S0_6, S4, S8 };
struct OceanParameters {
    CommonControls common{};
    double thresholdDb = -18.0;
    OceanRatio ratio = OceanRatio::R2;
    double attackMs = 40.0; // Approved 25..70 ms
    OceanRecovery recovery = OceanRecovery::S0_4;
    double scHpfHz = 0.0; // OFF, 40, 80, 160, 320, 500 on artwork
};
class Ocean final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const OceanParameters& p) noexcept;
    void reset() noexcept override;
    double gainReductionDb() const noexcept { return reductionDb_; }
    void setOversamplingMode(OversamplingMode quality) noexcept { quality_=quality; tube_.setMode(quality); }
private:
    Stereo processWet(Stereo in) noexcept override;
    static Stereo tubeTransfer(Stereo in, unsigned, void*) noexcept;
    OceanParameters p_{};
    OversampledProcessor tube_{};
    StereoDelay linearDelay_{};
    OversamplingMode quality_ = OversamplingMode::Off;
    DetectorHPF sc_{};
    Smoother threshold_{};
    double envelope_ = 0.0, reductionDb_ = 0.0;
    // Non-allocating, prepared off the audio thread. Curves follow the
    // published progressive Manley GR characteristics.
    static constexpr std::size_t kCurvePoints = 769; // overdrive -16..80 dB, 0.125 dB resolution
    std::array<std::array<float,kCurvePoints>,6> reductionCurves_{};
    void prepareReductionCurves() noexcept;
    double curveReduction(double overDb) const noexcept;
};

enum class FurnaceRatio { R2, R4, R10 };
enum class FurnaceAttack { Ms0_1, Ms0_3, Ms1, Ms3, Ms10, Ms30 };
enum class FurnaceRelease { S0_1, S0_3, S0_6, S1_2, Auto };
struct FurnaceParameters {
    CommonControls common{};
    double thresholdDb = 0.0; // Approved -15..+15 dB
    FurnaceRatio ratio = FurnaceRatio::R4;
    FurnaceAttack attack = FurnaceAttack::Ms10;
    FurnaceRelease release = FurnaceRelease::S0_3;
    double scHpfHz = 0.0;
};
class Furnace final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const FurnaceParameters& p) noexcept;
    void reset() noexcept override;
    double gainReductionDb() const noexcept { return reductionDb_; }
private:
    Stereo processWet(Stereo in) noexcept override;
    FurnaceParameters p_{};
    DetectorHPF sc_{};
    Smoother threshold_{};
    double envelope_ = 0.0, reductionDb_ = 0.0;
    double autoFast_ = 0.0, autoSlow_ = 0.0;
};

struct EQBandParameters {
    TptEQ::Mode mode = TptEQ::Mode::Peak;
    double frequencyHz = 1000.0; // Direct DSP value; UI mapping unresolved
    double q = 1.0; // Direct DSP value; UI mapping unresolved
    double gainDb = 0.0; // Approved -12..+12 dB
};
struct ParametricParameters {
    CommonControls common{};
    std::array<EQBandParameters,4> bands{};
    double outputDb = 0.0; // Output control exists; artwork shows -12..+12
};
class ParametricEQ final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const ParametricParameters& p) noexcept;
    void reset() noexcept override;
private:
    Stereo processWet(Stereo in) noexcept override;
    std::array<TptEQ,4> bands_{};
    Smoother output_{};
};

struct PultecParameters {
    CommonControls common{};
    int lowFrequencyHz = 60; // Approved: 30 / 60 / 100
    double lowBoost = 0.0, lowAttenuation = 0.0; // Approved 0..10
    int highBoostFrequencyHz = 12000; // Approved: 8 / 12 / 16 kHz
    double highBoost = 0.0, bandwidth = 0.5;
    int highAttenuationFrequencyHz = 10000; // Approved: 5 / 10 / 20 kHz; NOT visible on artwork
    double highAttenuation = 0.0;
    double outputDb = 0.0;
};
class Pultec final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const PultecParameters& p) noexcept;
    void reset() noexcept override;
private:
    Stereo processWet(Stereo in) noexcept override;
    PultecCircuit passive_{};
    Smoother output_{};
};

enum class DiscreteRatio { R1_2, R2, R3, R4, R6, Flood20 };
enum class DiscreteAttack { Ms0_1, Ms0_3, Ms1, Ms3, Ms10, Ms30 };
enum class DiscreteRecovery { S0_1, S0_2, S0_4, S0_6, S0_8, S1_2, Dual };
struct ShadowParameters {
    CommonControls common{};
    bool opticalIn = true, discreteIn = true;
    // DSP-native dB parameters. Mapping from visual 1..24 scales is NOT approved.
    double opticalThresholdDb = -18.0, opticalMakeupDb = 0.0;
    double discreteThresholdDb = -12.0, discreteGainDb = 0.0;
    DiscreteRatio discreteRatio = DiscreteRatio::R3;
    DiscreteAttack discreteAttack = DiscreteAttack::Ms3;
    DiscreteRecovery discreteRecovery = DiscreteRecovery::S0_4;
    double scHpfHz = 0.0, outputDb = 0.0;
};
class Shadow final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const ShadowParameters& p) noexcept;
    void reset() noexcept override;
    // Telemetry reflects the ACTUALLY APPLIED stage gain reduction, including
    // individual IN crossfades, rather than detector activity while bypassed.
    double opticalReductionDb() const noexcept { return opticalReductionDb_*opticalEnable_.value(); }
    double discreteReductionDb() const noexcept { return discreteReductionDb_*discreteEnable_.value(); }
private:
    Stereo processWet(Stereo in) noexcept override;
    ShadowParameters p_{};
    DetectorHPF opticalSC_{}, discreteSC_{};
    Smoother opticalEnable_{}, discreteEnable_{}, opticalThreshold_{}, discreteThreshold_{};
    Smoother opticalGain_{}, discreteGain_{}, output_{};
    double opticalEnv_ = 0.0, discreteEnv_ = 0.0;
    double opticalFastReduction_ = 0.0, opticalSlowReduction_ = 0.0;
    double discreteFastReduction_ = 0.0, discreteSlowReduction_ = 0.0;
    double opticalReductionDb_ = 0.0, discreteReductionDb_ = 0.0;
};

enum class ExciterMode { Tube, Tape, Transformer };
struct ExciterParameters {
    CommonControls common{};
    ExciterMode mode = ExciterMode::Tube;
    double lowDrive = 0.0, highDrive = 0.0; // Artwork 0..10
    double lowFrequencyHz = 100.0, highFrequencyHz = 5000.0;
    double crossoverHz = 800.0;
    double outputDb = 0.0;
};
class Exciter final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const ExciterParameters& p) noexcept;
    void reset() noexcept override;
    void setOversamplingMode(OversamplingMode quality) noexcept {
        quality_=quality; lowSaturator_.setMode(quality); highSaturator_.setMode(quality);
    }
private:
    Stereo processWet(Stereo in) noexcept override;
    struct SaturationContext {
        Exciter* engine; unsigned band; double drive;
        double invTanhDrive, invAtanDrive, tubeBias;
        std::array<double,3> weights;
    };
    static Stereo saturationTransfer(Stereo in, unsigned quality, void* ctx) noexcept;
    struct Pole { double l = 0.0, r = 0.0; Smoother coeff; void prepare(double fs) noexcept; void set(double fs,double hz) noexcept; Stereo low(Stereo v) noexcept; void reset() noexcept { l=r=0.0; } };
    ExciterParameters p_{};
    Pole crossover_{}, lowTone_{}, highTone_{}, dcRemoval_{};
    OversampledProcessor lowSaturator_{}, highSaturator_{};
    StereoDelay inputDelay_{}, bassDelay_{}, trebleDelay_{};
    std::array<std::array<Stereo,4>,2> tapeMemory_{};
    std::array<double,4> tapeCoeff_{};
    OversamplingMode quality_ = OversamplingMode::Off;
    Smoother lowDrive_{}, highDrive_{}, output_{};
    std::array<Smoother,3> modeWeights_{};
};

// Verified DSP chain of the first SIX modules ONLY. No limiter, no VST3 host
// integration or product-grade oversampling is claimed in this milestone.
class SixModuleRack {
public:
    void prepare(double sampleRate) noexcept;
    Stereo process(Stereo input) noexcept;
    void reset() noexcept;
    void setOversamplingMode(OversamplingMode quality) noexcept {
        ocean.setOversamplingMode(quality);
        exciter.setOversamplingMode(quality);
    }
    static constexpr std::size_t latencySamples() noexcept {return 2*OversampledProcessor::kLatency;}
    Ocean ocean;
    Furnace furnace;
    ParametricEQ parametric;
    Pultec pultec;
    Shadow shadow;
    Exciter exciter;
    uint64_t nonFiniteEvents() const noexcept { return nonFiniteEvents_.load(std::memory_order_relaxed); }
    uint64_t emergencyLimitEvents() const noexcept { return emergencyLimitEvents_.load(std::memory_order_relaxed); }
private:
    std::atomic<uint64_t> nonFiniteEvents_{0}, emergencyLimitEvents_{0};
};
}
