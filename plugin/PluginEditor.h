#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "HeaderModule.h"

// Standalone first-module test editor. SteamMasterHeader itself is independently
// embeddable in the eventual single-window, non-scrollable ten-module editor.
class SteamMasterEditor final : public juce::AudioProcessorEditor {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override = default;
    void resized() override;
    void paint(juce::Graphics&) override;
private:
    SteamMasterHeader header;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
