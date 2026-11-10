#pragma once
#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <memory>

// Embeddable original-artwork module. Coordinates originate from the approved
// user-supplied module manifests, not guessed pixels or generic control rows.
// All nine instances can coexist without viewports in one AudioProcessorEditor.
class SteamMasterModule final : public juce::Component, private juce::Timer {
public:
    explicit SteamMasterModule(SteamMasterProcessor&,const juce::String& moduleId);
    ~SteamMasterModule() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void setFocusRequest(std::function<void(const juce::String&)>);
    const juce::String& getModuleId() const noexcept {return id;}
    void setFocused(bool focused);
private:
    struct KnobLook;
    struct LampLook;
    struct Control {
        juce::String parameterId;
        juce::String kind;
        float x=0,y=0,r=25;
        std::unique_ptr<juce::Component> widget;
        std::unique_ptr<KnobLook> knobLook;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderBinding;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonBinding;
    };
    SteamMasterProcessor& processor;
    juce::String id;
    juce::Image plate,rotorAtlas;
    int sourceWidth=1536, sourceHeight=512;
    std::vector<std::unique_ptr<Control>> controls;
    std::unique_ptr<LampLook> lampLook;
    std::function<void(const juce::String&)> focusRequest;
    float reelPhase=0.f;
    bool focused=false;
    juce::Rectangle<float> artRect() const;
    juce::Point<float> pointAt(float x,float y) const;
    void timerCallback() override;
    void paintMeter(juce::Graphics&,juce::String label,float value,float minimum,
                    float maximum,float x,float y) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterModule)
};
