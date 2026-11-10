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
        int desiredHeight(int width) const;
        juce::String section;
        juce::Viewport viewport;
        struct ArtCanvas final : juce::Component {
            juce::Image art;
            int artHeight = 0;
            void paint(juce::Graphics& g) override {
                g.fillAll(juce::Colour(0xff19252a));
                if(art.isValid() && artHeight > 0) {
                    g.drawImage(art, juce::Rectangle<float>(0.f,0.f,(float)getWidth(),(float)artHeight),
                                juce::RectanglePlacement::stretchToFit);
                }
            }
        } contents;
        std::vector<std::unique_ptr<juce::Component>> controls;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
        std::vector<std::unique_ptr<juce::Label>> labels;
        std::vector<juce::String> ids;
    };
    SteamMasterProcessor& processor;
    juce::Viewport rackViewport;
    juce::Component rackContent;
    std::vector<std::unique_ptr<Page>> modules;
    std::unique_ptr<juce::LookAndFeel_V4> theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
