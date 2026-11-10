#include "HeaderModule.h"
#include <BinaryData.h>
#include <cmath>
#include <functional>

namespace {
constexpr int cropTop = 120;
juce::Image load(const char* data, int size) {
    return juce::ImageFileFormat::loadFrom(data, (size_t)size);
}
juce::Image crop(juce::Image img) {
    if (!img.isValid() || img.getHeight() <= cropTop) return {};
    return img.getClippedImage({0,cropTop,
        juce::jmin(2048,img.getWidth()),juce::jmin(438,img.getHeight()-cropTop)});
}
}

struct SteamMasterHeader::ImageStateButton final : juce::Button {
    juce::Image inactive, active;
    explicit ImageStateButton(const juce::String& id,const char* off,int offLen,
                              const char* on,int onLen)
       : juce::Button(id),inactive(load(off,offLen)),active(load(on,onLen)) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setWantsKeyboardFocus(true);
    }
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        auto& img = getToggleState() ? active : inactive;
        if(img.isValid()) {
            g.setOpacity(isEnabled() ? (down ? .85f:1.f):.45f);
            g.drawImageWithin(img,0,0,getWidth(),getHeight(),
                              juce::RectanglePlacement::centred);
        } else {
            g.setColour(juce::Colours::darkgoldenrod);
            g.fillRoundedRectangle(getLocalBounds().toFloat(),3.f);
        }
        if(over) {
            g.setColour(juce::Colours::white.withAlpha(.16f));
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1),3.f,1.f);
        }
    }
};

struct SteamMasterHeader::DialLook final : juce::LookAndFeel_V4 {
    juce::Image inputArt,outputArt;
    DialLook() {
        inputArt = load(BinaryData::knob_input_silver_png,
                        BinaryData::knob_input_silver_pngSize);
        outputArt = load(BinaryData::knob_output_silver_png,
                         BinaryData::knob_output_silver_pngSize);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,
                          float start,float end,juce::Slider& slider) override {
        const auto& img = (slider.getComponentID()=="global_input_db")?inputArt:outputArt;
        if(!img.isValid())return;
        const auto size=(float)juce::jmin(w,h);
        const auto scale=size/(float)juce::jmax(img.getWidth(),img.getHeight());
        const float radians=start+position*(end-start);
        const auto transform=juce::AffineTransform::translation(-img.getWidth()*.5f,
                                                                 -img.getHeight()*.5f)
                                 .rotated(radians).scaled(scale)
                                 .translated((float)x+w*.5f,(float)y+h*.5f);
        g.drawImageTransformed(img,transform);
    }
};

SteamMasterHeader::ImageStateButton& SteamMasterHeader::button(
        const juce::String& name,const char* off,int offLen,const char* on,int onLen,
        float x,float y,float w,float h,std::function<void()> callback) {
    auto b=std::make_unique<ImageStateButton>(name,off,offLen,on,onLen);
    b->setTooltip(name);
    b->onClick=std::move(callback);
    auto* result=b.get();
    addAndMakeVisible(*b);
    placements.push_back({result,x,y,w,h});
    artButtons.push_back(std::move(b));
    return *result;
}

SteamMasterHeader::SteamMasterHeader(SteamMasterProcessor& p):processor(p) {
    faceplate=crop(load(BinaryData::header_original_faceplate_png,
                        BinaryData::header_original_faceplate_pngSize));
    dialLook=std::make_unique<DialLook>();
    const auto bindDial=[this](const char* id,int x,int y,std::unique_ptr<juce::Slider>& dial,
                               std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment){
        dial=std::make_unique<juce::Slider>();
        dial->setComponentID(id);
        dial->setTooltip(id);
        dial->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        dial->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        dial->setRotaryParameters(juce::MathConstants<float>::pi*1.25f,
                                  juce::MathConstants<float>::pi*2.75f,true);
        dial->setLookAndFeel(dialLook.get());
        addAndMakeVisible(*dial);
        attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    processor.state,id,*dial);
    };
    bindDial("global_input_db",219,353,input,inputBinding);
    bindDial("global_output_db",402,353,output,outputBinding);

#define A(NAME) BinaryData::NAME##_png, BinaryData::NAME##_pngSize
    globalButton=&button("Global IN",A(global_toggle_off),A(global_toggle_on),
                          97,307,68,146,{});
    globalButton->setClickingTogglesState(true);
    globalBinding=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                     processor.state,"global_in",*globalButton);

    oversamplingButtons[0]=&button("Oversampling Off",A(button_off_off),A(button_off_on),
                                  586,429,78,66,[this]{changeOversampling(0);});
    oversamplingButtons[1]=&button("Oversampling 2x",A(button_2x_off),A(button_2x_on),
                                  667,429,78,66,[this]{changeOversampling(1);});
    oversamplingButtons[2]=&button("Oversampling 4x",A(button_4x_off),A(button_4x_on),
                                  748,429,78,66,[this]{changeOversampling(2);});
    oversamplingButtons[3]=&button("Oversampling 8x",A(button_8x_off),A(button_8x_on),
                                  829,429,78,66,[this]{changeOversampling(3);});

    slotA=processor.state.copyState();
    slotB=slotA.createCopy();
    aButton=&button("Snapshot A",A(button_a_off),A(button_a_on),
                    962,429,83,66,[this]{switchSnapshot(false);});
    bButton=&button("Snapshot B",A(button_b_off),A(button_b_on),
                    1045,429,83,66,[this]{switchSnapshot(true);});
    button("Previous preset",A(button_previous_off),A(button_previous_on),
           1181,429,57,66,[this]{previousOrNextPreset(-1);});
    button("Next preset",A(button_next_off),A(button_next_on),
           1468,429,57,66,[this]{previousOrNextPreset(1);});
    button("Save preset",A(button_save_off),A(button_save_on),
           1527,429,62,66,[this]{savePreset();});
#undef A
    startTimerHz(24);
}

SteamMasterHeader::~SteamMasterHeader() {
    stopTimer();
    inputBinding.reset();
    outputBinding.reset();
    globalBinding.reset();
    input->setLookAndFeel(nullptr);
    output->setLookAndFeel(nullptr);
}
juce::Rectangle<int> SteamMasterHeader::mapped(float x,float y,float w,float h) const {
    const float sx=(float)getWidth()/sourceWidth;
    const float sy=(float)getHeight()/sourceHeight;
    return {juce::roundToInt(x*sx),juce::roundToInt((y-cropTop)*sy),
            juce::jmax(1,juce::roundToInt(w*sx)),juce::jmax(1,juce::roundToInt(h*sy))};
}
void SteamMasterHeader::resized() {
    input->setBounds(mapped(219,353,128,128));
    output->setBounds(mapped(402,353,128,128));
    for(const auto& p:placements)
        p.button->setBounds(mapped(p.x,p.y,p.w,p.h));
}

void SteamMasterHeader::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff10181b));
    if(faceplate.isValid())
        g.drawImage(faceplate,getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit);
    else {
        g.setColour(juce::Colours::gold);
        g.drawText("STEAMMASTER — GLOBAL HEADER",getLocalBounds(),
                   juce::Justification::centred);
    }
}

void SteamMasterHeader::paintOverChildren(juce::Graphics& g) {
    const auto text=[this,&g](const juce::String& s,float x,float y,float w,float h,
                              float originalFontSize,juce::Justification just) {
        auto r=mapped(x,y,w,h).toFloat();
        g.setColour(juce::Colour(0xff2b2118));
        g.setFont(juce::Font(juce::FontOptions(
            juce::jmax(8.f,originalFontSize*getWidth()/sourceWidth))));
        g.drawFittedText(s,r.toNearestInt(),just,1);
    };
    // The original engraved dial labels and tick scales are preserved.
    // Current gain values are available from host automation and slider tooltips,
    // not pasted on top of the physical source artwork.
    text(presetName+(usingB?" [B]":" [A]"),1240,432,225,60,33.f,
         juce::Justification::centred);

    // The design's LUFS values were static mockup numbers; no fictitious
    // loudness values are displayed in the running plugin.
    const auto meter=[this,&g](float db,float x) {
        auto zone=mapped(x,242,34,244).toFloat();
        g.setColour(juce::Colour(0xff081114));
        g.fillRoundedRectangle(zone,2.f);
        const int segments=28;
        const float gap=juce::jmax(.5f,zone.getHeight()*.006f);
        const float bh=(zone.getHeight()-gap*(segments-1))/segments;
        const float active=juce::jlimit(0.f,1.f,(db+48.f)/48.f)*segments;
        for(int i=0;i<segments;++i) {
            const float y=zone.getBottom()-(i+1)*bh-i*gap;
            const float frac=(float)i/segments;
            auto colour=frac>.83f?juce::Colour(0xfff55c3e):
                        frac>.60f?juce::Colour(0xffffd45a):
                        frac>.27f?juce::Colour(0xff65dd73):
                                    juce::Colour(0xff31c8a9);
            g.setColour(colour.withAlpha(i<active?1.f:.13f));
            g.fillRoundedRectangle(zone.getX(),y,zone.getWidth(),bh,1.f);
        }
    };
    meter(processor.meterLeftDb.load(std::memory_order_relaxed),1869);
    meter(processor.meterRightDb.load(std::memory_order_relaxed),1938);
}

void SteamMasterHeader::timerCallback() {
    const auto* choice=processor.state.getRawParameterValue("global_oversampling");
    const int active=choice?juce::jlimit(0,3,(int)std::round(choice->load())):0;
    for(int i=0;i<4;++i)
        oversamplingButtons[(size_t)i]->setToggleState(active==i,juce::dontSendNotification);
    aButton->setToggleState(!usingB,juce::dontSendNotification);
    bButton->setToggleState(usingB,juce::dontSendNotification);
    repaint();
}
void SteamMasterHeader::changeOversampling(int index) {
    auto* p=processor.state.getParameter("global_oversampling");
    if(p) p->setValueNotifyingHost(p->convertTo0to1((float)index));
    timerCallback();
}
void SteamMasterHeader::switchSnapshot(bool toB) {
    if(toB==usingB)return;
    if(usingB)slotB=processor.state.copyState();else slotA=processor.state.copyState();
    processor.state.replaceState((toB?slotB:slotA).createCopy());
    usingB=toB;
    timerCallback();
}
juce::File SteamMasterHeader::presetDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
         .getChildFile("ThomC").getChildFile("SteamMaster").getChildFile("Presets");
}
void SteamMasterHeader::savePreset() {
    auto dir=presetDirectory();
    if(dir.createDirectory().failed()){presetName="SAVE ERROR";return;}
    const auto filename="SteamMaster_"+juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S")+".xml";
    const auto destination=dir.getChildFile(filename);
    auto xml=processor.state.copyState().createXml();
    if(xml && xml->writeTo(destination))presetName=destination.getFileNameWithoutExtension();
    else presetName="SAVE ERROR";
    repaint();
}
void SteamMasterHeader::previousOrNextPreset(int dir) {
    auto files=presetDirectory().findChildFiles(juce::File::findFiles,false,"*.xml");
    if(files.isEmpty()){presetName="NO SAVED PRESETS";repaint();return;}
    int i=-1;
    for(int n=0;n<files.size();++n)if(files[n].getFileNameWithoutExtension()==presetName){i=n;break;}
    const auto next=(i+dir+files.size())%files.size();
    auto xml=juce::XmlDocument::parse(files[next]);
    if(xml && xml->hasTagName(processor.state.state.getType().toString())) {
        processor.state.replaceState(juce::ValueTree::fromXml(*xml));
        presetName=files[next].getFileNameWithoutExtension();
        if(usingB)slotB=processor.state.copyState();else slotA=processor.state.copyState();
    }else presetName="INVALID PRESET";
    repaint();
}
