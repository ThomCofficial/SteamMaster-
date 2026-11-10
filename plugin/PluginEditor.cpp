#include "PluginEditor.h"

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
    : AudioProcessorEditor(&p), header(p) {
    setOpaque(true);
    addAndMakeVisible(header);
    setResizable(true,true);
    setResizeLimits(850,182,2000,428);
    if(auto* c=getConstrainer())c->setFixedAspectRatio(2048.0/438.0);
    setSize(1400,300);
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff10181b));
}
void SteamMasterEditor::resized() {
    header.setBounds(getLocalBounds());
}
