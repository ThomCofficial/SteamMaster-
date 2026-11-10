#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "HeaderModule.h"
#include "UnifiedModule.h"
#include <array>
#include <memory>

// One resizable, non-scrolling SteamMaster window. Nine live hardware modules
// stay visible in Overview; select any module for a full-width Focus view.
class SteamMasterEditor final : public juce::AudioProcessorEditor {
public:
    explicit SteamMasterEditor(SteamMasterProcessor&);
    ~SteamMasterEditor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
private:
    SteamMasterHeader header;
    std::vector<std::unique_ptr<SteamMasterModule>> modules;
    std::vector<std::unique_ptr<juce::TextButton>> navigation;
    juce::String selected;
    void select(const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteamMasterEditor)
};
