#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float pi = juce::MathConstants<float>::pi;
const juce::Colour cream(0xfff2d7a6), gold(0xffbb7c35), teal(0xff153339), ink(0xff3b2618);
void text(juce::Graphics& g, const juce::String& s, int x,int y,int w,int h,float size,juce::Colour c) {
    g.setColour(c); g.setFont(juce::Font(juce::FontOptions(size).withStyle("Bold")));
    g.drawText(s,x,y,w,h,juce::Justification::centred,false);
}
}

void SteamMasterEditor::MetalLookAndFeel::drawRotarySlider(
    juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&) {
    const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f;
    const float r=float(juce::jmin(w,h))*.45f;
    g.setColour(juce::Colour(0xff49311d));
    g.fillEllipse(cx-r-3,cy-r-3,2*r+6,2*r+6);
    g.setColour(juce::Colour(0xffd8ab65));
    g.fillEllipse(cx-r,cy-r,2*r,2*r);
    juce::ColourGradient grad(juce::Colour(0xfff8f6ea),cx-r,cy-r,
                              juce::Colour(0xff6d706f),cx+r,cy+r,false);
    grad.addColour(.5,juce::Colour(0xffced1ce));
    g.setGradientFill(grad);
    g.fillEllipse(cx-r+6,cy-r+6,2*r-12,2*r-12);
    g.setColour(juce::Colour(0xff47301c));
    const float a=start+(end-start)*pos;
    g.drawLine(cx,cy,cx+std::sin(a)*(r-12),cy-std::cos(a)*(r-12),4.0f);
}
void SteamMasterEditor::MetalLookAndFeel::drawButtonBackground(
    juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) {
    auto r=b.getLocalBounds().toFloat().reduced(2);
    const bool lit=b.getToggleState();
    g.setColour(juce::Colour(0xff211a14));
    g.fillRoundedRectangle(r.expanded(2),6);
    juce::ColourGradient grad(lit?juce::Colour(0xffffd47c):juce::Colour(0xff785535),
                              r.getX(),r.getY(),
                              lit?juce::Colour(0xffcf7019):juce::Colour(0xff28211c),
                              r.getRight(),r.getBottom(),false);
    if(over||down)grad.addColour(.55,juce::Colour(0xffc68b45));
    g.setGradientFill(grad);g.fillRoundedRectangle(r,5);
    g.setColour(gold);g.drawRoundedRectangle(r,5,2);
}

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
: juce::AudioProcessorEditor(&p),processor(p) {
    setOpaque(true);
    auto knob=[this](juce::Slider& s) {
        addAndMakeVisible(s);s.setLookAndFeel(&metalLook);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setRotaryParameters(1.25f*pi,2.75f*pi,true);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,92,22);
        s.setColour(juce::Slider::textBoxBackgroundColourId,cream);
        s.setColour(juce::Slider::textBoxTextColourId,ink);
        s.setColour(juce::Slider::textBoxOutlineColourId,gold);
    };
    knob(inputGain);knob(outputGain);
    inputAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.state,"global_input_db",inputGain);
    outputAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.state,"global_output_db",outputGain);
    auto button=[this](juce::TextButton& b,const juce::String& label) {
        b.setButtonText(label);b.setLookAndFeel(&metalLook);
        b.setColour(juce::TextButton::textColourOffId,cream);
        b.setColour(juce::TextButton::textColourOnId,ink);
        addAndMakeVisible(b);
    };
    button(globalIn,"IN");globalIn.setClickingTogglesState(true);
    globalInAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.state,"global_in",globalIn);
    const char* quality[]={"OFF","2x","4x","8x"};
    for(int i=0;i<4;++i) {
        button(oversamplingButtons[size_t(i)],quality[i]);
        oversamplingButtons[size_t(i)].onClick=[this,i]{chooseOversampling(i);};
    }
    for(int i=0;i<2;++i) {
        button(abButtons[size_t(i)],i==0?"A":"B");
        abButtons[size_t(i)].onClick=[this,i]{chooseAB(i);};
    }
    button(previousPreset,"<");button(nextPreset,">");button(savePresetButton,"SAVE");
    previousPreset.onClick=[this]{navigatePreset(-1);};
    nextPreset.onClick=[this]{navigatePreset(1);};
    savePresetButton.onClick=[this]{savePreset();};
    addAndMakeVisible(presetLabel);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setColour(juce::Label::textColourId,cream);
    presetLabel.setText(currentPresetName,juce::dontSendNotification);
    abSnapshots[0]=p.state.copyState();abSnapshots[1]=abSnapshots[0].createCopy();
    setResizable(true,true);setResizeLimits(820,173,1850,390);
    if(auto* c=getConstrainer())c->setFixedAspectRatio(double(canvasWidth)/canvasHeight);
    setSize(1280,270);startTimerHz(20);timerCallback();
}
SteamMasterEditor::~SteamMasterEditor() {
    stopTimer();
    inputGain.setLookAndFeel(nullptr);outputGain.setLookAndFeel(nullptr);
    globalIn.setLookAndFeel(nullptr);
    for(auto& b:oversamplingButtons)b.setLookAndFeel(nullptr);
    for(auto& b:abButtons)b.setLookAndFeel(nullptr);
    previousPreset.setLookAndFeel(nullptr);nextPreset.setLookAndFeel(nullptr);
    savePresetButton.setLookAndFeel(nullptr);
}
void SteamMasterEditor::chooseOversampling(int index) {
    if(auto* p=processor.state.getParameter("global_oversampling")) {
        p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(float(index)));
        p->endChangeGesture();
    }
    timerCallback();
}
void SteamMasterEditor::chooseAB(int i) {
    if(i==activeAB)return;
    abSnapshots[size_t(activeAB)]=processor.state.copyState();
    activeAB=i;processor.state.replaceState(abSnapshots[size_t(i)].createCopy());
    timerCallback();
}
juce::File SteamMasterEditor::presetDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("ThomC").getChildFile("SteamMaster").getChildFile("Presets");
}
void SteamMasterEditor::navigatePreset(int direction) {
    juce::Array<juce::File> files;
    auto folder=presetDirectory();
    if(!folder.exists())return;
    folder.findChildFiles(files,juce::File::findFiles,false,"*.stmpreset");
    if(files.isEmpty())return;
    std::sort(files.begin(),files.end(),[](const juce::File& a,const juce::File& b) {
        return a.getFileName().compareNatural(b.getFileName())<0;
    });
    int index=0;
    for(int i=0;i<files.size();++i)
        if(files.getReference(i).getFileNameWithoutExtension()==currentPresetName){index=i;break;}
    index=(index+direction+files.size())%files.size();
    auto file=files.getReference(index);
    if(auto xml=juce::XmlDocument::parse(file)) {
        if(xml->hasTagName(processor.state.state.getType().toString())) {
            processor.state.replaceState(juce::ValueTree::fromXml(*xml));
            currentPresetName=file.getFileNameWithoutExtension();
            presetLabel.setText(currentPresetName,juce::dontSendNotification);
        }
    }
}
void SteamMasterEditor::savePreset() {
    auto folder=presetDirectory();
    if(!folder.isDirectory()&&!folder.createDirectory())return;
    presetChooser=std::make_unique<juce::FileChooser>(
        "Save SteamMaster preset",folder.getChildFile(currentPresetName+".stmpreset"),"*.stmpreset");
    presetChooser->launchAsync(juce::FileBrowserComponent::saveMode |
        juce::FileBrowserComponent::canSelectFiles,[this](const juce::FileChooser& c) {
        auto file=c.getResult();if(file==juce::File())return;
        if(!file.hasFileExtension(".stmpreset"))file=file.withFileExtension(".stmpreset");
        if(auto xml=processor.state.copyState().createXml())
            if(file.replaceWithText(xml->toString())) {
                currentPresetName=file.getFileNameWithoutExtension();
                presetLabel.setText(currentPresetName,juce::dontSendNotification);
            }
    });
}
void SteamMasterEditor::timerCallback() {
    if(auto* v=processor.state.getRawParameterValue("global_oversampling")) {
        int chosen=juce::jlimit(0,3,juce::roundToInt(v->load()));
        for(int i=0;i<4;++i)oversamplingButtons[size_t(i)].setToggleState(
            i==chosen,juce::dontSendNotification);
    }
    for(int i=0;i<2;++i)abButtons[size_t(i)].setToggleState(
        i==activeAB,juce::dontSendNotification);
}
void SteamMasterEditor::paintFaceplate(juce::Graphics& g) const {
    g.fillAll(juce::Colour(0xff15191b));
    g.setColour(juce::Colour(0xff85512a));g.fillRoundedRectangle(4,7,2040,417,36);
    g.setColour(gold);g.drawRoundedRectangle(10,10,2028,412,33,6);
    g.setColour(teal);g.fillRoundedRectangle(32,29,1984,369,20);
    g.setColour(gold);g.drawRoundedRectangle(32,29,1984,369,20,3);
    g.setColour(cream);g.fillRoundedRectangle(80,173,460,207,17);
    g.fillRoundedRectangle(566,224,1045,156,17);
    g.setColour(gold);g.drawRoundedRectangle(80,173,460,207,17,3);
    g.drawRoundedRectangle(566,224,1045,156,17,3);
    g.setColour(juce::Colour(0xff16110e));
    g.fillRoundedRectangle(1643,123,178,256,12);
    g.fillRoundedRectangle(1831,95,148,284,12);
    g.setColour(gold);g.drawRoundedRectangle(1643,123,178,256,12,3);
    g.drawRoundedRectangle(1831,95,148,284,12,3);
    g.setColour(cream);g.fillRoundedRectangle(720,15,695,193,46);
    g.setColour(gold);g.drawRoundedRectangle(720,15,695,193,46,5);
}
void SteamMasterEditor::paintLabelsAndScales(juce::Graphics& g) const {
    text(g,"ThomC",900,23,335,50,33,ink);
    text(g,"STEAMMASTER",755,68,630,88,73,juce::Colour(0xff70421a));
    text(g,"MASTERING SUITE",856,155,421,43,31,ink);
    text(g,"GLOBAL IN",88,190,111,40,22,ink);
    text(g,"INPUT",207,184,151,36,25,ink);
    text(g,"OUTPUT",379,184,151,36,25,ink);
    text(g,"OVERSAMPLING",588,234,320,44,27,ink);
    text(g,"A / B",973,234,169,44,27,ink);
    text(g,"PRESETS",1225,234,315,44,27,ink);
    const auto drawScale=[&](int cx,const std::array<int,9>& values) {
        for(int i=0;i<9;++i){
            float a=(-135.f+270.f*float(i)/8.f)*pi/180.f;
            float sx=std::sin(a),cy=std::cos(a);
            g.setColour(ink);g.drawLine(cx+77*sx,310-77*cy,cx+88*sx,310-88*cy,2.f);
            auto v=values[size_t(i)];
            text(g,(v>0?"+":"")+juce::String(v),juce::roundToInt(cx+108*sx)-27,
                 juce::roundToInt(310-108*cy)-15,54,30,20,ink);
        }
    };
    drawScale(282,{{-24,-18,-12,-6,0,6,12,18,24}});
    drawScale(456,{{-24,-18,-12,-6,0,3,6,9,12}});
}
void SteamMasterEditor::paintMeterPlaceholders(juce::Graphics& g) const {
    text(g,"LUFS",1649,133,171,47,30,cream);
    text(g,"INT   --",1650,200,170,30,25,cream);
    text(g,"SRT   --",1650,260,170,30,25,cream);
    text(g,"TP    --",1650,319,170,30,25,cream);
    text(g,"L",1848,110,49,36,24,cream);
    text(g,"R",1910,110,49,36,24,cream);
    for(int bar=0;bar<2;++bar)for(int i=0;i<23;++i) {
        g.setColour(juce::Colour(0xff2c4845));g.fillRect(1850+bar*65,156+i*9,38,5);
    }
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    const float s=juce::jmin(float(getWidth())/canvasWidth,float(getHeight())/canvasHeight);
    g.addTransform(juce::AffineTransform::scale(s));
    paintFaceplate(g);paintLabelsAndScales(g);paintMeterPlaceholders(g);
}
void SteamMasterEditor::resized() {
    const float s=juce::jmin(float(getWidth())/canvasWidth,float(getHeight())/canvasHeight);
    auto place=[s](juce::Component& c,int x,int y,int w,int h) {
        c.setBounds(juce::roundToInt(x*s),juce::roundToInt(y*s),
                    juce::roundToInt(w*s),juce::roundToInt(h*s));
    };
    place(globalIn,109,255,68,84);
    place(inputGain,229,251,106,123);
    place(outputGain,405,251,106,123);
    for(int i=0;i<4;++i)place(oversamplingButtons[size_t(i)],586+i*81,290,79,65);
    place(abButtons[0],972,290,80,65);place(abButtons[1],1055,290,80,65);
    place(previousPreset,1184,290,55,65);
    place(presetLabel,1244,291,243,62);
    place(nextPreset,1491,290,55,65);
    place(savePresetButton,1550,290,61,65);
}
