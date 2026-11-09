#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <vector>
class SteamMasterEditor final : public juce::AudioProcessorEditor {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    void resized() override;
private:
    struct Page : juce::Component {
        explicit Page(juce::AudioProcessor&,juce::AudioProcessorValueTreeState&, const juce::String& section);
        void resized() override;
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
