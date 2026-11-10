#include "PluginEditor.h"
#include <cmath>
#ifdef STEAMMASTER_HAS_ARTWORK
#include <BinaryData.h>
#endif

namespace {
constexpr auto brass = juce::Colour(0xffbb9353);
constexpr auto ivory = juce::Colour(0xfff0dec0);
constexpr auto steel = juce::Colour(0xff263742);
constexpr auto deep = juce::Colour(0xff10191d);
class SteamLook final : public juce::LookAndFeel_V4 {
public:
 SteamLook() {
    setColour(juce::Slider::textBoxTextColourId, ivory);
    setColour(juce::Slider::textBoxBackgroundColourId, deep);
    setColour(juce::Slider::textBoxOutlineColourId, brass.withAlpha(0.45f));
    setColour(juce::ComboBox::backgroundColourId, steel);
    setColour(juce::ComboBox::textColourId, ivory);
    setColour(juce::ComboBox::outlineColourId, brass);
    setColour(juce::Label::textColourId, ivory);
    setColour(juce::ToggleButton::textColourId, ivory);
    setColour(juce::TabbedButtonBar::tabOutlineColourId, brass);
    setColour(juce::TabbedButtonBar::frontOutlineColourId, brass);
 }
 void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,
                      float start,float end,juce::Slider&) override {
    auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(7.f);
    const float r=juce::jmin(bounds.getWidth(),bounds.getHeight())*0.43f;
    const auto c=bounds.getCentre();
    auto rim=juce::Rectangle<float>(c.x-r,c.y-r,2*r,2*r);
    g.setColour(brass.darker(0.65f));g.fillEllipse(rim.expanded(4.f));
    g.setColour(brass);g.drawEllipse(rim.expanded(3.f),2.f);
    juce::ColourGradient gradient(juce::Colour(0xff64717b),c.x-r,c.y-r,
                                  juce::Colour(0xff161e25),c.x+r,c.y+r,false);
    gradient.addColour(0.48,juce::Colour(0xffc6b9a1));
    g.setGradientFill(gradient);g.fillEllipse(rim);
    g.setColour(ivory.withAlpha(.42f));g.drawEllipse(rim.reduced(3.f),1.f);
    const float theta=start+position*(end-start);
    auto tip=c+juce::Point<float>(std::sin(theta),-std::cos(theta))*(r*.79f);
    auto base=c+juce::Point<float>(std::sin(theta),-std::cos(theta))*(r*.20f);
    g.setColour(juce::Colours::black);g.drawLine(base.x+1,base.y+1,tip.x+1,tip.y+1,4.f);
    g.setColour(juce::Colour(0xffffe8b2));g.drawLine(base.x,base.y,tip.x,tip.y,3.f);
    g.setColour(brass);g.fillEllipse(juce::Rectangle<float>(c.x-4,c.y-4,8,8));
 }
 void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool over,bool down) override {
    auto r=b.getLocalBounds().toFloat().reduced(3);
    const bool on=b.getToggleState();
    g.setColour(juce::Colour(0xff080e12));g.fillRoundedRectangle(r,5.f);
    g.setColour(on?brass:brass.withAlpha(.35f));g.drawRoundedRectangle(r,5.f,1.5f);
    g.setColour(on?juce::Colour(0xffffd17c):juce::Colour(0xff526169));
    g.fillEllipse(r.getX()+7,r.getCentreY()-4,8,8);
    g.setColour(on?ivory:ivory.withAlpha(.72f));
    g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
    g.drawText(b.getButtonText(),r.withTrimmedLeft(19),juce::Justification::centredLeft,true);
    if(over||down){g.setColour(juce::Colours::white.withAlpha(.08f));g.fillRoundedRectangle(r,5.f);}
 }
};
void paintPanel(juce::Graphics& g,juce::Rectangle<int> b,const juce::String& title) {
 auto r=b.toFloat().reduced(3);
 juce::ColourGradient bg(juce::Colour(0xff344653),r.getX(),r.getY(),
                         juce::Colour(0xff131e26),r.getRight(),r.getBottom(),false);
 g.setGradientFill(bg);g.fillRoundedRectangle(r,10);
 g.setColour(brass);g.drawRoundedRectangle(r,10,2);
 g.setColour(brass.withAlpha(.45f));g.drawRoundedRectangle(r.reduced(5),7,1);
 g.setColour(ivory);g.setFont(juce::Font(juce::FontOptions(22.f,juce::Font::bold)));
 g.drawFittedText(title.toUpperCase(),b.reduced(24).removeFromTop(40),juce::Justification::centredLeft,1);
 g.setColour(brass.withAlpha(.65f));
 g.drawLine((float)b.getX()+20,(float)b.getY()+49,(float)b.getRight()-20,(float)b.getY()+49,1);
}
}

SteamMasterEditor::Page::Page(juce::AudioProcessor& processor,juce::AudioProcessorValueTreeState& state,
                               const juce::String& s):section(s) {
    // Decode the approved source art once, on the message thread, never in paint/audio callbacks.
#ifdef STEAMMASTER_HAS_ARTWORK
    const char* asset = nullptr;
    int assetSize = 0;
    if (section == "Global") { asset=BinaryData::global_header_png; assetSize=BinaryData::global_header_pngSize; }
    else if(section == "Ocean") { asset=BinaryData::ocean_png; assetSize=BinaryData::ocean_pngSize; }
    else if(section == "Furnace") { asset=BinaryData::furnace_png; assetSize=BinaryData::furnace_pngSize; }
    else if(section == "Parametric") { asset=BinaryData::parametric_png; assetSize=BinaryData::parametric_pngSize; }
    else if(section == "Pultec") { asset=BinaryData::pultec_png; assetSize=BinaryData::pultec_pngSize; }
    else if(section == "Shadow") { asset=BinaryData::shadow_png; assetSize=BinaryData::shadow_pngSize; }
    else if(section == "Exciter") { asset=BinaryData::exciter_png; assetSize=BinaryData::exciter_pngSize; }
    else if(section == "Bax Mid" || section == "Bax Side") { asset=BinaryData::bax_png; assetSize=BinaryData::bax_pngSize; }
    else if(section == "Tape") { asset=BinaryData::tape_png; assetSize=BinaryData::tape_pngSize; }
    else if(section == "Stereo" || section == "Limiter") { asset=BinaryData::stereo_limiter_png; assetSize=BinaryData::stereo_limiter_pngSize; }
    if (asset != nullptr && assetSize > 0) {
       art = juce::ImageFileFormat::loadFrom(asset, (size_t)assetSize);
    }
#endif
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&contents,false);
    viewport.setScrollBarsShown(true,false);
    for(auto* p:processor.getParameters()) {
        auto* withID=dynamic_cast<juce::AudioProcessorParameterWithID*>(p);
        if(!withID||!withID->getName(200).startsWith(section+" / "))continue;
        const auto id=withID->paramID;
        auto label=std::make_unique<juce::Label>();
        label->setText(withID->getName(200).fromFirstOccurrenceOf(" / ",false,false),juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::Font(juce::FontOptions(13.f)));
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
            b->setButtonText("ACTIVE");
            contents.addAndMakeVisible(*b);
            buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state,id,*b));
            controls.push_back(std::move(b));
        } else {
            auto slider=std::make_unique<juce::Slider>();
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,92,23);
            slider->setRotaryParameters(juce::MathConstants<float>::pi*1.25f,
                      juce::MathConstants<float>::pi*2.75f,true);
            // APVTS attachments own the true parameter range; do not use the normalized default as a value.
            contents.addAndMakeVisible(*slider);
            sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,*slider));
            controls.push_back(std::move(slider));
        }
    }
}
void SteamMasterEditor::Page::paint(juce::Graphics& g) {
    g.fillAll(deep);
    paintPanel(g,getLocalBounds(),section);
    if(art.isValid()){
       auto area = getLocalBounds().reduced(23);
       area.removeFromTop(55);
       area.setHeight(juce::jmin(180,juce::jmax(95,getHeight()/3)));
       g.setColour(brass.darker(0.55f));
       g.fillRoundedRectangle(area.toFloat().expanded(3),6.f);
       g.drawImageWithin(art,area.getX(),area.getY(),area.getWidth(),area.getHeight(),juce::RectanglePlacement::centred);
    }
}
void SteamMasterEditor::Page::resized() {
    auto area=getLocalBounds().reduced(16);
    area.removeFromTop(art.isValid() ? juce::jmin(247,juce::jmax(160,getHeight()/3+55)) : 55);
    viewport.setBounds(area);
    const int cellW=158,cellH=145;
    const int cols=juce::jmax(1,(area.getWidth()-20)/cellW);
    const int rows=(int(controls.size())+cols-1)/cols;
    const int contentW=juce::jmax(area.getWidth()-18,cols*cellW);
    contents.setSize(contentW,juce::jmax(area.getHeight(),rows*cellH+14));
    for(int i=0;i<int(controls.size());++i){
       const int x=(i%cols)*cellW,y=(i/cols)*cellH;
       labels[(size_t)i]->setBounds(x+3,y+2,cellW-6,27);
       controls[(size_t)i]->setBounds(x+9,y+31,cellW-18,105);
    }
}
SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p):AudioProcessorEditor(&p),processor(p) {
    setOpaque(true);
    theme=std::make_unique<SteamLook>();
    setLookAndFeel(theme.get());
    addAndMakeVisible(tabs);
    for(const auto* section:{"Global","Ocean","Furnace","Parametric","Pultec","Shadow",
                              "Exciter","Bax Mid","Bax Side","Tape","Stereo","Limiter"}) {
       auto* page=new Page(processor,processor.state,section);
       tabs.addTab(section,juce::Colour(0xff273540),page,true);
    }
    setResizable(true,true);
    setResizeLimits(760,500,1800,1100);
    setSize(1120,730);
}
SteamMasterEditor::~SteamMasterEditor(){
    tabs.clearTabs();
    setLookAndFeel(nullptr);
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    g.fillAll(deep);
    auto r=getLocalBounds().toFloat().reduced(3.f);
    g.setColour(brass);
    g.drawRoundedRectangle(r,8.f,2.f);
}
void SteamMasterEditor::resized(){tabs.setBounds(getLocalBounds().reduced(9));}
