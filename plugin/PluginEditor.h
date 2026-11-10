#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

/**
 * SteamMaster UI-development milestone 1: a native, proportionally resizable
 * Global Header. The subsequent rack modules are deliberately NOT represented
 * by fake generic controls. Their approved high-resolution artwork is still
 * awaiting production-grade integration.
 *
 * Never change processor parameter IDs or DSP algorithms to accommodate UI.
 */
class SteamMasterEditor final : public juce::AudioProcessorEditor,
                                private juce::Timer {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class MetalLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics&, int, int, int, int,
                              float, float, float, juce::Slider&) override;
        void drawButtonBackground(juce::Graphics&, juce::Button&,
                                  const juce::Colour&, bool, bool) override;
    };

    class RackPanel;
    juce::Viewport rackViewport;
    juce::Component rackCanvas;
    std::vector<std::unique_ptr<RackPanel>> rackPanels;
    SteamMasterProcessor& processor;
    MetalLookAndFeel metalLook;

    juce::Slider inputGain, outputGain;
    juce::TextButton globalIn;
    std::array<juce::TextButton, 4> oversamplingButtons;
    std::array<juce::TextButton, 2> abButtons;
    juce::TextButton previousPreset, nextPreset, savePresetButton;
    juce::Label presetLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> globalInAttachment;

    std::array<juce::ValueTree, 2> abSnapshots;
    int activeAB = 0;
    juce::String currentPresetName = "Default";
    std::unique_ptr<juce::FileChooser> presetChooser;

    void timerCallback() override;
    void chooseOversampling(int);
    void chooseAB(int);
    void navigatePreset(int);
    void savePreset();
    juce::File presetDirectory() const;
    void paintFaceplate(juce::Graphics&) const;
    void paintLabelsAndScales(juce::Graphics&) const;
    void paintMeterPlaceholders(juce::Graphics&) const;

    static constexpr int canvasWidth = 2048;
    static constexpr int canvasHeight = 432;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
