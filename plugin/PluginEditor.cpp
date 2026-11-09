#include "PluginEditor.h"

SteamMasterEditor::Page::Page(juce::AudioProcessor& processor,juce::AudioProcessorValueTreeState& state,const juce::String& section) {
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contents,false);
    viewport.setScrollBarsShown(true,false);
    for(auto* p : processor.getParameters()) {
        auto* withID=dynamic_cast<juce::AudioProcessorParameterWithID*>(p);
        if(!withID || !withID->getName(200).startsWith(section+" / "))continue;
        const auto id=withID->paramID;
        auto label=std::make_unique<juce::Label>();
        label->setText(withID->getName(200).fromFirstOccurrenceOf(" / ",false,false),juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        contents.addAndMakeVisible(*label);
        labels.push_back(std::move(label));
        if(auto* choice=dynamic_cast<juce::AudioParameterChoice*>(p)) {
            auto c=std::make_unique<juce::ComboBox>();
            c->addItemList(choice->choices,1);
            contents.addAndMakeVisible(*c);
            combos.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state,id,*c));
            controls.push_back(std::move(c));
        } else if(dynamic_cast<juce::AudioParameterBool*>(p)) {
            auto b=std::make_unique<juce::ToggleButton>();
            contents.addAndMakeVisible(*b);
            buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state,id,*b));
            controls.push_back(std::move(b));
        } else {
            auto s=std::make_unique<juce::Slider>();
            s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,20);
            contents.addAndMakeVisible(*s);
            sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,*s));
            controls.push_back(std::move(s));
        }
    }
}
void SteamMasterEditor::Page::resized() {
    viewport.setBounds(getLocalBounds());
    const int width=juce::jmax(480,getWidth()-18), cols=4, cellW=width/cols, cellH=130;
    const int rows=(int(controls.size())+cols-1)/cols;
    contents.setSize(width,juce::jmax(getHeight(),rows*cellH+12));
    for(int i=0;i<int(controls.size());++i){const int x=(i%cols)*cellW,y=(i/cols)*cellH;
        labels[size_t(i)]->setBounds(x+5,y+5,cellW-10,25);
        controls[size_t(i)]->setBounds(x+8,y+32,cellW-16,88);
    }
}
SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p):AudioProcessorEditor(&p),processor(p) {
    setOpaque(true);addAndMakeVisible(tabs);
    for(const auto* section:{"Global","Ocean","Furnace","Parametric","Pultec","Shadow","Exciter","Bax Mid","Bax Side","Tape","Stereo","Limiter"}) {
        auto* page=new Page(processor,processor.state,section);
        tabs.addTab(section,juce::Colour(0xff19232d),page,true);
    }
    setSize(880,580);
}
void SteamMasterEditor::resized(){tabs.setBounds(getLocalBounds().reduced(8));}
