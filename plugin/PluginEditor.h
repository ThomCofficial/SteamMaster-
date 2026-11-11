#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <vector>
#include <memory>
#include <array>

class SteamMasterEditor final : public juce::AudioProcessorEditor {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
    bool saveFullRackPreview(const juce::File& destination);
    bool verifyResizeAnchors();
private:
    struct Page final : juce::Component, private juce::Timer {
        explicit Page(juce::AudioProcessor&,juce::AudioProcessorValueTreeState&,const juce::String&);
        void resized() override;
        void paint(juce::Graphics&) override;
        int desiredHeight(int width) const;
        void timerCallback() override;
        int artPixelWidth() const;
        int artPixelHeight() const;
        SteamMasterProcessor& source;
        juce::String section;
        struct ArtCanvas final : juce::Component {
            juce::Image art;
            int artHeight = 0;
            juce::Image leftReel, rightReel;
            juce::Image headerMeterAtlas;
            std::array<juce::Image,6> exciterModes, limiterModes;
            juce::Image shadowNeedle, exciterNeedle, correlationNeedle, limiterNeedle;
            juce::String section;
            SteamMasterProcessor* processor = nullptr;
            float reelAngle = 0.f;
            void drawMeterNeedle(juce::Graphics& g, const juce::Image& needle,
                                 float cx, float cy, float radius, float fraction);
            void paint(juce::Graphics& g) override {
                g.fillAll(juce::Colour(0xff10191d));
                if(art.isValid() && artHeight > 0) {
                    g.drawImage(art, juce::Rectangle<float>(0.f,0.f,(float)getWidth(),(float)artHeight),
                                juce::RectanglePlacement::stretchToFit);
                    paintMovingParts(g);
                    if(section=="Global") paintHeaderInstruments(g);
                }
            }
            void paintMovingParts(juce::Graphics& g);
            void paintHeaderInstruments(juce::Graphics& g);
        } contents;
        std::vector<std::unique_ptr<juce::Component>> controls;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
        std::vector<std::unique_ptr<juce::Label>> labels;
        std::vector<juce::String> ids;
        std::vector<juce::String> overflowIDs;
        std::vector<std::unique_ptr<juce::TextButton>> headerButtons;
        std::unique_ptr<juce::Label> presetLabel;
        juce::ValueTree slotA,slotB;
        bool usingB=false;
        juce::String presetTitle="Default";
        void navigatePreset(int direction);
        void savePreset();
        void switchSlot(bool toB);
        static juce::File presetDirectory();
    };
    SteamMasterProcessor& processor;
    juce::Viewport rackViewport;
    juce::Component rackContent;
    std::vector<std::unique_ptr<Page>> modules;
    std::unique_ptr<juce::LookAndFeel_V4> theme;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
