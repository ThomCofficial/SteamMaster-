#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "steammaster/Mastering.hpp"
#include <memory>
#include <array>
#include <atomic>

class SteamMasterProcessor final : public juce::AudioProcessor {
public:
    SteamMasterProcessor();
    const juce::String getName() const override {return "SteamMaster Test Bench";}
    void prepareToPlay(double sampleRate,int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& layout) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    double getTailLengthSeconds() const override {return 0.0;}
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    std::unique_ptr<steammaster::MasteringRack> rack;
    struct MeterSnapshot {float leftPeak=0.f,rightPeak=0.f,limiterReductionDb=0.f,truePeak=0.f,correlation=0.f;};
    MeterSnapshot getMeterSnapshot() const noexcept {
        return {meterLeft_.load(std::memory_order_relaxed),meterRight_.load(std::memory_order_relaxed),
                meterReduction_.load(std::memory_order_relaxed),meterTruePeak_.load(std::memory_order_relaxed),
                meterCorrelation_.load(std::memory_order_relaxed)};
    }
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    std::atomic<float> meterLeft_{0.f},meterRight_{0.f},meterReduction_{0.f};
    std::atomic<float> meterTruePeak_{0.f},meterCorrelation_{0.f};
    std::array<juce::AudioProcessorParameter*,107> monitored_{};
    std::array<float,107> previous_{};
    bool snapshotValid_=false;
    float v(const char* id) const noexcept {auto* p=state.getRawParameterValue(id);return p?p->load(std::memory_order_relaxed):0.f;}
    void updateDSP() noexcept;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterProcessor)
};