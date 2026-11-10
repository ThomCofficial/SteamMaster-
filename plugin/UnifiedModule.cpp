#include "UnifiedModule.h"
#include <BinaryData.h>
#include <cmath>

namespace {
juce::Image decode(const char* data,int size) {
    return juce::ImageFileFormat::loadFrom(data,(size_t)size);
}
juce::Image panel(const juce::String& id) {
#define PANEL(ID) if(id==#ID) return decode(BinaryData::ID##_faceplate_png,BinaryData::ID##_faceplate_pngSize)
    PANEL(ocean); PANEL(furnace); PANEL(parametric); PANEL(pultec);
    PANEL(shadow); PANEL(exciter); PANEL(bax); PANEL(tape); PANEL(stereo_limiter);
#undef PANEL
    return {};
}
juce::Image sprites(const juce::String& id) {
#define ROTORS(ID) if(id==#ID) return decode(BinaryData::ID##_rotors_png,BinaryData::ID##_rotors_pngSize)
    ROTORS(ocean); ROTORS(furnace); ROTORS(parametric); ROTORS(pultec);
    ROTORS(shadow); ROTORS(exciter); ROTORS(bax); ROTORS(tape); ROTORS(stereo_limiter);
#undef ROTORS
    return {};
}
juce::String heading(const juce::String& id) {
    if(id=="stereo_limiter")return "STEREO IMAGE + LIMITER";
    return id.toUpperCase();
}
}

struct SteamMasterModule::KnobLook final : juce::LookAndFeel_V4 {
    juce::Image rotor;
    explicit KnobLook(const juce::Image& i):rotor(i) {}
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,
                          float start,float end,juce::Slider&) override {
        if(!rotor.isValid())return;
        const float imgW=(float)rotor.getWidth(),imgH=(float)rotor.getHeight();
        const float factor=juce::jmin((float)w/imgW,(float)h/imgH);
        auto t=juce::AffineTransform::translation(-imgW*.5f,-imgH*.5f)
                   .rotated(start+(end-start)*pos)
                   .scaled(factor)
                   .translated((float)x+w*.5f,(float)y+h*.5f);
        g.drawImageTransformed(rotor,t);
    }
};
struct SteamMasterModule::LampLook final : juce::LookAndFeel_V4 {
    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool hovered,bool) override {
        const auto r=b.getLocalBounds().toFloat();
        // The actual engraved metal switch and labels remain on the approved
        // faceplate. Only the working illuminated state is overlaid.
        if(b.getToggleState()) {
            auto cx=r.getCentreX(),cy=r.getCentreY();
            const float radius=juce::jmin(r.getWidth(),r.getHeight())*.21f;
            juce::ColourGradient glow(juce::Colour(0xffffc76d).withAlpha(.9f),cx,cy,
                                       juce::Colour(0xffffb744).withAlpha(0.f),
                                       cx+radius*2.f,cy,true);
            g.setGradientFill(glow);
            g.fillEllipse(cx-radius*2,cy-radius*2,radius*4,radius*4);
            g.setColour(juce::Colour(0xffffe0a0));
            g.fillEllipse(cx-radius*.65f,cy-radius*.65f,radius*1.3f,radius*1.3f);
        }
        if(hovered) {
            g.setColour(juce::Colours::white.withAlpha(.17f));
            g.drawRoundedRectangle(r.reduced(2.f),4.f,1.f);
        }
    }
};

SteamMasterModule::SteamMasterModule(SteamMasterProcessor& p,const juce::String& name)
    :processor(p),id(name) {
    plate=panel(id);
    rotorAtlas=sprites(id);
    if(id=="tape") reelAtlas=decode(BinaryData::tape_reels_png,BinaryData::tape_reels_pngSize);
    lampLook=std::make_unique<LampLook>();
    const auto json=juce::String::fromUTF8(BinaryData::ui_layout_json,BinaryData::ui_layout_jsonSize);
    const auto root=juce::JSON::parse(json);
    const auto mods=root.getProperty("modules",juce::var());
    const auto spec=mods.getProperty(id,juce::var());
    sourceWidth=(int)spec.getProperty("width",1536);
    sourceHeight=(int)spec.getProperty("height",512);
    if(!plate.isValid() || !rotorAtlas.isValid()) {
        jassertfalse; return;
    }
    const auto list=spec.getProperty("controls",juce::var());
    auto* items=list.getArray();
    if(items==nullptr)return;
    int rotorIndex=0;
    for(const auto& item:*items) {
        auto ctrl=std::make_unique<Control>();
        ctrl->parameterId=item.getProperty("id",juce::var()).toString();
        ctrl->kind=item.getProperty("kind",juce::var()).toString();
        ctrl->x=(float)(double)item.getProperty("x",juce::var(0.0));
        ctrl->y=(float)(double)item.getProperty("y",juce::var(0.0));
        ctrl->r=(float)(double)item.getProperty("r",juce::var(25.0));
        auto* param=processor.state.getParameter(ctrl->parameterId);
        if(param==nullptr) {jassertfalse; continue;}
        if(ctrl->kind=="knob") {
            auto slider=std::make_unique<juce::Slider>();
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
            slider->setRotaryParameters(juce::MathConstants<float>::pi*1.20f,
                                        juce::MathConstants<float>::pi*2.80f,true);
            slider->setTooltip(param->getName(120));
            slider->setComponentID(ctrl->parameterId);
            const int sx=rotorIndex%8,sy=rotorIndex/8;
            auto sprite=rotorAtlas.getClippedImage({sx*160,sy*160,160,160});
            ctrl->knobLook=std::make_unique<KnobLook>(sprite);
            slider->setLookAndFeel(ctrl->knobLook.get());
            ctrl->sliderBinding=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                                   processor.state,ctrl->parameterId,*slider);
            addAndMakeVisible(*slider);
            ctrl->widget=std::move(slider);
            ++rotorIndex;
        } else if(ctrl->kind=="button") {
            auto b=std::make_unique<juce::ToggleButton>();
            b->setButtonText("");
            b->setTooltip(param->getName(120));
            b->setLookAndFeel(lampLook.get());
            ctrl->buttonBinding=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                                    processor.state,ctrl->parameterId,*b);
            addAndMakeVisible(*b);
            ctrl->widget=std::move(b);
        } else {
            auto b=std::make_unique<juce::TextButton>();
            b->setTooltip(param->getName(120));
            b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff30261a));
            b->setColour(juce::TextButton::textColourOffId,juce::Colour(0xffffdba0));
            const auto pid=ctrl->parameterId;
            b->onClick=[this,pid]{
                auto* param=processor.state.getParameter(pid);
                auto* choice=dynamic_cast<juce::AudioParameterChoice*>(param);
                if(param==nullptr || choice==nullptr)return;
                const int next=((int)std::round(param->convertFrom0to1(param->getValue()))+1)%
                                juce::jmax(1,choice->choices.size());
                param->setValueNotifyingHost(param->convertTo0to1((float)next));
            };
            addAndMakeVisible(*b);
            ctrl->widget=std::move(b);
        }
        controls.push_back(std::move(ctrl));
    }
    startTimerHz(20);
}
SteamMasterModule::~SteamMasterModule() {
    stopTimer();
    for(auto& c:controls) {
        c->buttonBinding.reset();
        c->sliderBinding.reset();
        if(auto* s=dynamic_cast<juce::Slider*>(c->widget.get()))s->setLookAndFeel(nullptr);
        if(auto* b=dynamic_cast<juce::ToggleButton*>(c->widget.get()))b->setLookAndFeel(nullptr);
    }
    controls.clear();
}

juce::Rectangle<float> SteamMasterModule::artRect() const {
    const float x=(float)getWidth(),y=(float)getHeight();
    const float k=juce::jmin(x/(float)sourceWidth,y/(float)sourceHeight);
    const float w=sourceWidth*k,h=sourceHeight*k;
    return {(x-w)*.5f,(y-h)*.5f,w,h};
}
juce::Point<float> SteamMasterModule::pointAt(float x,float y) const {
    const auto r=artRect();
    return {r.getX()+x*r.getWidth()/sourceWidth,r.getY()+y*r.getHeight()/sourceHeight};
}
void SteamMasterModule::resized() {
    const auto a=artRect();
    const float scale=a.getWidth()/(float)sourceWidth;
    for(auto& ctrl:controls) {
        auto center=pointAt(ctrl->x,ctrl->y);
        const bool slider=ctrl->kind=="knob";
        // The rotor is a 160x160 source-space atlas cell with an alpha-masked
        // original hardware core; scale the entire cell by source-image scale.
        const float diameter=slider?160.f:ctrl->r*3.f;
        int w=juce::jmax(5,juce::roundToInt(diameter*scale));
        int h=w;
        if(ctrl->kind=="cycle"){w=juce::jmax(23,juce::roundToInt(ctrl->r*2.0f*scale));h=juce::jmax(12,juce::roundToInt(28.f*scale));}
        ctrl->widget->setBounds(juce::roundToInt(center.x-w/2.f),
                                juce::roundToInt(center.y-h/2.f),w,h);
        // Source reference artwork is never recoloured or resized by generative tools.
    }
}
void SteamMasterModule::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff10171b));
    if(plate.isValid())
        g.drawImage(plate,artRect(),juce::RectanglePlacement::stretchToFit);
    if(id=="tape" && reelAtlas.isValid()) {
        const float scale=artRect().getWidth()/sourceWidth;
        for(int i=0;i<2;++i) {
            auto reel=reelAtlas.getClippedImage({i*256,0,256,256});
            const auto center=pointAt(i==0?395.f:1151.f,141.f);
            const float theta=(i==0?-1.f:1.f)*reelPhase;
            auto matrix=juce::AffineTransform::translation(-128.f,-128.f)
                            .rotated(theta).scaled(scale)
                            .translated(center.x,center.y);
            g.drawImageTransformed(reel,matrix);
        }
    }

    else {
        g.setColour(juce::Colour(0xffe1b775));
        g.drawText(heading(id),getLocalBounds(),juce::Justification::centred);
    }
    g.setColour(focused?juce::Colour(0xffeacb89):juce::Colour(0xff8b6f47));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.f),5.f,focused?2.f:1.f);
}
void SteamMasterModule::paintMeter(juce::Graphics& g,juce::String label,float value,float low,
                                    float high,float x,float y) const {
    auto center=pointAt(x,y);
    const float s=artRect().getWidth()/sourceWidth;
    const float w=juce::jmax(24.f,102.f*s),h=juce::jmax(8.f,24.f*s);
    juce::Rectangle<float> box(center.x-w*.5f,center.y-h*.5f,w,h);
    g.setColour(juce::Colour(0xff24180f).withAlpha(.92f));
    g.fillRoundedRectangle(box,4.f);
    const auto normalized=juce::jlimit(0.f,1.f,(value-low)/(high-low));
    auto track=box.reduced(2.f).withTrimmedLeft(w*.29f);
    g.setColour(juce::Colour(0xffe6af58));
    g.fillRect(track.withWidth(track.getWidth()*normalized));
    g.setColour(juce::Colour(0xffffe5ba));
    g.setFont(juce::Font(juce::FontOptions(juce::jmax(7.f,12.f*s))));
    g.drawFittedText(label,box.withWidth(w*.30f).toNearestInt(),juce::Justification::centred,1);
}
void SteamMasterModule::paintOverChildren(juce::Graphics& g) {
    if(!focused)return;
    const auto read=[](const std::atomic<float>& f){return f.load(std::memory_order_relaxed);};
    if(id=="ocean") paintMeter(g,"GR",-read(processor.meterOceanGr),0.f,20.f,700,50);
    else if(id=="furnace") paintMeter(g,"GR",-read(processor.meterFurnaceGr),0.f,20.f,700,50);
    else if(id=="shadow") {
        paintMeter(g,"OPTO",-read(processor.meterShadowOpticalGr),0.f,20.f,700,130);
        paintMeter(g,"DISC",-read(processor.meterShadowDiscreteGr),0.f,20.f,700,165);
    } else if(id=="stereo_limiter") {
        paintMeter(g,"CORR",read(processor.meterCorrelation),-1.f,1.f,1294,133);
        paintMeter(g,"GR",read(processor.meterLimiterGr),0.f,20.f,1336,365);
    }
}
void SteamMasterModule::timerCallback(){
    if(id=="tape") {
        const auto* enabled=processor.state.getRawParameterValue("tape_in");
        const auto* speed=processor.state.getRawParameterValue("tape_speed");
        const bool running=enabled!=nullptr && enabled->load(std::memory_order_relaxed)>.5f &&
            processor.meterOutputDb.load(std::memory_order_relaxed)>-70.f;
        if(running) {
            const float choice=speed?speed->load(std::memory_order_relaxed):1.f;
            reelPhase=std::fmod(reelPhase+(.025f+.021f*choice),juce::MathConstants<float>::twoPi);
            repaint();
        }
    }
    for(auto& c:controls) {
        if(c->kind!="cycle")continue;
        auto* b=dynamic_cast<juce::TextButton*>(c->widget.get());
        auto* p=dynamic_cast<juce::AudioParameterChoice*>(processor.state.getParameter(c->parameterId));
        if(b && p) {
            const int index=juce::jlimit(0,p->choices.size()-1,
                              (int)std::round(p->convertFrom0to1(p->getValue())));
            b->setButtonText(p->choices[index]);
        }
    }
    if(focused)repaint();
}
void SteamMasterModule::setFocusRequest(std::function<void(const juce::String&)> fn){
    focusRequest=std::move(fn);
}
void SteamMasterModule::mouseDoubleClick(const juce::MouseEvent&) {
    if(focusRequest)focusRequest(id);
}
void SteamMasterModule::setFocused(bool v) {focused=v;repaint();}
