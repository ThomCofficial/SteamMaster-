#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

// Module 01 is a self-contained JUCE Component. The full one-window rack will
// compose this component with other modules; no Viewport or scrolling involved.
class SteamMasterHeader final : public juce::Component, private juce::Timer {
public:
    explicit SteamMasterHeader(SteamMasterProcessor&);
    ~SteamMasterHeader() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    static constexpr float sourceWidth = 2048.f;
    static constexpr float sourceHeight = 438.f;
private:
    struct ImageStateButton;
    struct DialLook;
    SteamMasterProcessor& processor;
    juce::Image faceplate;
    std::unique_ptr<DialLook> dialLook;
    std::unique_ptr<juce::Slider> input, output;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputBinding, outputBinding;
    std::vector<std::unique_ptr<ImageStateButton>> artButtons;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> globalBinding;
    juce::ValueTree slotA, slotB;
    bool usingB = false;
    juce::String presetName = "Default";
    float lastLeft = -90.f, lastRight = -90.f;
    void timerCallback() override;
    void changeOversampling(int index);
    void switchSnapshot(bool toB);
    void previousOrNextPreset(int direction);
    void savePreset();
    juce::File presetDirectory() const;
    juce::Rectangle<int> mapped(float x,float y,float w,float h) const;
    ImageStateButton& button(const juce::String& name,const char* off,int offLen,
                             const char* on,int onLen,
                             float x,float y,float w,float h,std::function<void()> callback);
    struct Layout { ImageStateButton* button; float x,y,w,h; };
    std::vector<Layout> placements;
    ImageStateButton* globalButton = nullptr;
    std::array<ImageStateButton*,4> oversamplingButtons{};
    ImageStateButton* aButton=nullptr;
    ImageStateButton* bButton=nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterHeader)
};
