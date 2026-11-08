#pragma once
#include "steammaster/DSP.hpp"
#include "steammaster/BaxShelf.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace steammaster {

// Positions are EXACTLY those approved in the written Baxandall specification.
// This layer accepts direct physical units. GUI LINK behavior is controller-side.
enum class BaxHPF : std::uint8_t { Out, Hz12, Hz18, Hz24, Hz30, Hz36, Hz43, Hz54 };
enum class BaxLowShelf : std::uint8_t { Hz74, Hz84, Hz98, Hz116, Hz131, Hz166, Hz230, Hz361 };
enum class BaxHighShelf : std::uint8_t { Hz1600, Hz1800, Hz2100, Hz2500, Hz3400, Hz4800, Hz7100, Hz18000 };
enum class BaxLPF : std::uint8_t { Out, KHz70, KHz28, KHz18, KHz12_6, KHz11_1, KHz9, KHz7_5 };
enum class BaxRouting : std::uint8_t { MidSide, LeftRight };
struct BaxRowParameters {
    BaxHPF hpf = BaxHPF::Out;
    BaxLowShelf lowFrequency = BaxLowShelf::Hz116;
    double lowGainDb = 0.0;            // -5..+5, quantized to 0.5 dB
    double highGainDb = 0.0;           // -5..+5, quantized to 0.5 dB
    BaxHighShelf highFrequency = BaxHighShelf::Hz3400;
    BaxLPF lpf = BaxLPF::Out;
    double outputDb = 0.0;             // -4..+4 dB
    bool in = true;
};
struct BaxandallParameters {
    std::array<BaxRowParameters,2> rows{};  // Mid/Side or Left/Right, same two row faces
    BaxRouting routing = BaxRouting::MidSide;
    bool link = false;  // For the future GUI parameter controller; no audio side effects
};
class Baxandall final {
public:
    void prepare(double sampleRate) noexcept;
    void set(const BaxandallParameters& parameters) noexcept;
    void reset() noexcept;
    Stereo process(Stereo input) noexcept;
private:
    struct Row {
        void prepare(double sampleRate) noexcept;
        void set(const BaxRowParameters& p, double sampleRate) noexcept;
        void reset() noexcept;
        double run(double input) noexcept;  // scalar channel packed into LEFT
        TptEQ hpf_, lpf_;
        BaxShelf lowShelf_, highShelf_;
        Smoother hpfMix_, lpfMix_, bypass_, output_;
    };
    std::array<Row,2> ms_{}, lr_{};
    Smoother routing_{};
    double fs_ = 48000.0;
};

enum class TapeType : std::uint8_t { Studio, Vintage, Modern };
enum class TapeSpeed : std::uint8_t { IPS7_5, IPS15, IPS30 };
struct TapeParameters {
    CommonControls common{};
    TapeType type = TapeType::Studio;
    TapeSpeed speed = TapeSpeed::IPS15;
    double driveDb = 0.0; // -10..+10
    double bias = 0.0;    // -5..+5, internal DSP mapping experimental
    double hfTrimDb = 0.0; // -6..+6
};
class Tape final : public Module {
public:
    void prepare(double sampleRate) noexcept override;
    void set(const TapeParameters& params) noexcept;
    void reset() noexcept override;
    void setOversamplingMode(OversamplingMode mode) noexcept { quality_=mode; saturator_.setMode(mode); }
    OversamplingMode oversamplingMode() const noexcept {return saturator_.requestedMode();}
private:
    Stereo processWet(Stereo input) noexcept override;
    static Stereo saturate(Stereo input, unsigned quality, void* context) noexcept;
    struct Context { Tape* self; double drive; double bias; std::array<double,3> weights; };
    TapeParameters params_{};
    OversampledProcessor saturator_{};
    TptEQ bump_{}, hfTrim_{};
    Smoother drive_{}, bias_{}, transientGain_{};
    std::array<Smoother,3> types_{};
    std::array<Stereo,4> memory_{};
    std::array<Stereo,4> previous_{};
    double envelope_ = 0.0;
    Stereo dc_{};
    OversamplingMode quality_ = OversamplingMode::Off;
};

// Provisional standalone limiter engine. No claim of verified EBU true-peak ceiling.
// This engine exposes ONLY the controls proposed in the development brief.
enum class LimiterMode : std::uint8_t { Clean, Punch, Modern };
struct LimiterParameters {
    bool in = true;
    LimiterMode mode = LimiterMode::Clean;
    double thresholdDb = -6.0; // -24..0
    double ceilingDbTP = -1.0;  // -2..0
    double releaseMs = 150.0;  // 10..1000
    bool autoRelease = false;
    double lookaheadMs = 3.0; // 0.1..5
    double stereoLink = 1.0;  // 0..1 (UI 0..100%)
    bool truePeak = true;
};
class MasterLimiter final {
public:
    void prepare(double fs) noexcept;
    void set(const LimiterParameters& p) noexcept;
    void setRequireLinkedGain(bool locked) noexcept {forceLinked_=locked;}
    void reset() noexcept;
    Stereo process(Stereo input) noexcept;
    std::size_t latencySamples() const noexcept {return audioLatency_+postLatency_;}
    Stereo gainReductionDb() const noexcept {return reductionDb_;}
    double measuredOutputTruePeak() const noexcept {return outputTruePeak_;}
    // Counts samples where emergency sample-domain clipping was needed.
    // Nonzero here is a warning that protective limiting was insufficient.
    std::uint64_t safetyClipEvents() const noexcept {return safetyClipEvents_;}
private:
    static constexpr std::size_t kCapacity=4096;
    std::array<Stereo,kCapacity> audioBuffer_{}, dryBuffer_{}, peakBuffer_{}, postBuffer_{};
    std::size_t audioCursor_=0, peakCursor_=0, postCursor_=0, audioLatency_=0, postLatency_=0;
    FIRHalfBand2x detector2x_{}, detector4x_{}, detector8x_{}, post2x_{}, post4x_{}, post8x_{}, meter2x_{}, meter4x_{}, meter8x_{};
    Smoother inputGain_{}, ceiling_{}, bypass_{}, lookahead_{}, link_{}, postMix_{};
    Stereo gain_ {1.0,1.0}, postGain_{1.0,1.0}, reductionDb_{}, prePostEffectiveGain_{1.0,1.0};
    std::array<std::size_t,2> releaseHold_{}, postHold_{};
    std::uint64_t safetyClipEvents_=0;
    double fs_=48000.0, outputTruePeak_=0.0, ceilingHard_=1.0;
    bool truePeak_=true, autoRelease_=false, forceLinked_=false;
    LimiterMode mode_=LimiterMode::Clean;
    double releaseMs_=150.;
    Stereo getPeak(Stereo input) noexcept;
    Stereo postPeak(Stereo input) noexcept;
    Stereo postTruePeakGuard(Stereo input, double ceiling) noexcept;
    Stereo measureOutput(Stereo output) noexcept;
};

struct GlobalParameters {
    bool in = true;
    double inputDb = 0.0;      // -24..+24
    double outputDb = 0.0;     // -24..+12; PRE-LIMITER
    double monoMakerHz = 0.0;  // 0=OFF, otherwise 20..300
    double stereoWidth = 1.0;  // 0..2, 1=neutral
    double widthFrequencyHz = 2000.0; // 800..5000
    OversamplingMode oversampling = OversamplingMode::Off;
};
class GlobalStereo final {
public:
    void prepare(double sampleRate) noexcept;
    void set(const GlobalParameters& p) noexcept;
    void reset() noexcept;
    Stereo process(Stereo input) noexcept;
private:
    double fs_=48000.0;
    TptEQ sideLowpass_{}, sideHighpass_{};
    Smoother width_{}, monoMaker_{};
};

// Full v0.3 DSP integration: no VST3 or GUI binding included.
class MasteringRack final {
public:
    void prepare(double sampleRate) noexcept;
    void setGlobal(const GlobalParameters& p) noexcept;
    void reset() noexcept;
    Stereo process(Stereo in) noexcept;
    std::size_t latencySamples() const noexcept {return SixModuleRack::latencySamples()+OversampledProcessor::kLatency+limiter.latencySamples();}
    uint64_t invalidEvents() const noexcept {return nonFinite_.load(std::memory_order_relaxed);}
    uint64_t emergencyEvents() const noexcept {return emergency_.load(std::memory_order_relaxed);}
    SixModuleRack firstSix{};
    Baxandall baxandall{};
    Tape tape{};
    GlobalStereo stereo{};
    MasterLimiter limiter{};
private:
    Stereo guard(Stereo in) noexcept;
    GlobalParameters global_{};
    Smoother input_{}, output_{}, bypass_{};
    std::array<Stereo,4096> bypassBuffer_{};
    std::size_t bypassCursor_=0;
    std::atomic<uint64_t> nonFinite_{0}, emergency_{0};
};
} // namespace steammaster
