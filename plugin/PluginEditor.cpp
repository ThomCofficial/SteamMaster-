#include "PluginEditor.h"

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
    :AudioProcessorEditor(&p),header(p) {
    setOpaque(true);
    addAndMakeVisible(header);
    const char* ids[]={"ocean","furnace","parametric",
                       "pultec","shadow","exciter",
                       "bax","tape","stereo_limiter"};
    auto addNavigation=[this](const juce::String& title,const juce::String& id) {
        auto b=std::make_unique<juce::TextButton>(title);
        b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff231d15));
        b->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xffa47636));
        b->setColour(juce::TextButton::textColourOffId,juce::Colour(0xfff6dfb1));
        b->setColour(juce::TextButton::textColourOnId,juce::Colour(0xff211409));
        b->onClick=[this,id]{select(id);};
        addAndMakeVisible(*b);
        navigation.push_back(std::move(b));
    };
    addNavigation("ALL MODULES","");
    for(auto id:ids) {
        auto m=std::make_unique<SteamMasterModule>(p,id);
        m->setFocusRequest([this](const juce::String& name){select(name);});
        addAndMakeVisible(*m);
        modules.push_back(std::move(m));
        const juce::String pretty=juce::String(id).replace("_"," ").toUpperCase();
        addNavigation(pretty,id);
    }
    setResizable(true,true);
    setResizeLimits(1000,720,2600,2000);
    setSize(1650,1140);
    select("");
}
SteamMasterEditor::~SteamMasterEditor()=default;
void SteamMasterEditor::select(const juce::String& id) {
    selected=id;
    for(auto& m:modules) {
        const bool active=selected.isEmpty()||m->getModuleId()==selected;
        m->setVisible(active);
        m->setFocused(!selected.isEmpty()&&m->getModuleId()==selected);
    }
    for(size_t i=0;i<navigation.size();++i)
        navigation[i]->setToggleState(i==0?selected.isEmpty():
            modules[i-1]->getModuleId()==selected,juce::dontSendNotification);
    resized();
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0e1416));
    g.setColour(juce::Colour(0xffb38b4b));
    g.drawRect(getLocalBounds(),2);
}
void SteamMasterEditor::resized() {
    const auto area=getLocalBounds().reduced(8);
    // Fit original header at the exact 2048:438 artwork aspect ratio.
    const int hdr=juce::jmin(juce::roundToInt((float)area.getWidth()*438.f/2048.f),
                            juce::jmax(170,getHeight()/3));
    header.setBounds(area.getX(),area.getY(),area.getWidth(),hdr);
    const int toolbarY=area.getY()+hdr+8;
    const int barHeight=juce::jlimit(28,46,getHeight()/27);
    const int gap=3;
    const int buttons=(int)navigation.size();
    const int baseWidth=(area.getWidth()-(buttons-1)*gap)/buttons;
    for(int i=0;i<buttons;++i)navigation[(size_t)i]->setBounds(
        area.getX()+i*(baseWidth+gap),toolbarY,baseWidth,barHeight);
    auto body=juce::Rectangle<int>(area.getX(),toolbarY+barHeight+8,
                                   area.getWidth(),juce::jmax(1,area.getBottom()-(toolbarY+barHeight+8)));
    if(!selected.isEmpty()) {
        for(auto& module:modules)if(module->getModuleId()==selected) {
            module->setBounds(body.reduced(8));
        }
        return;
    }
    constexpr int cols=3,rows=3;
    const int pad=6,cellW=body.getWidth()/cols,cellH=body.getHeight()/rows;
    for(int i=0;i<(int)modules.size();++i) {
        auto cell=juce::Rectangle<int>(body.getX()+(i%cols)*cellW,
                                       body.getY()+(i/cols)*cellH,
                                       cellW,cellH).reduced(pad);
        modules[(size_t)i]->setBounds(cell);
    }
}
