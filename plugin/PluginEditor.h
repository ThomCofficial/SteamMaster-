#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <vector>
#include <memory>

class SteamMasterEditor final : public juce::AudioProcessorEditor {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
private:
    struct Page final : juce::Component {
        explicit Page(juce::AudioProcessor&,juce::AudioProcessorValueTreeState&,const juce::String&);
        void resized() override;
        void paint(juce::Graphics&) override;
        juce::String section;
        juce::Viewport viewport;
        juce::Component contents;
        std::vector<std::unique_ptr<juce::Component>> controls;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
        std::vector<std::unique_ptr<juce::Label>> labels;
    };
    SteamMasterProcessor& processor;
    juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
    std::unique_ptr<juce::LookAndFeel_V4> theme;
    std::unique_ptr<juce::ComponentBoundsConstrainer> boundsConstraint;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
