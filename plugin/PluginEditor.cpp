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


class SteamMasterEditor::RackPanel final : public juce::Component {
public:
    class Tile final : public juce::Component {
    public:
        explicit Tile(juce::AudioProcessorValueTreeState& state,juce::String id) {
            auto* p=state.getParameter(id);
            jassert(p!=nullptr);
            if(p==nullptr)return;
            label.setText(p->getName(100).fromLastOccurrenceOf(" / ",false,false).toUpperCase(),juce::dontSendNotification);
            label.setFont(juce::Font(juce::FontOptions(13.f).withStyle("Bold")));
            label.setJustificationType(juce::Justification::centred);
            label.setColour(juce::Label::textColourId,juce::Colour(0xff55391d));
            addAndMakeVisible(label);
            if(auto* c=dynamic_cast<juce::AudioParameterChoice*>(p)){
                combo=std::make_unique<juce::ComboBox>();
                for(int i=0;i<c->choices.size();++i)combo->addItem(c->choices[i],i+1);
                combo->setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xffe3cfae));
                combo->setColour(juce::ComboBox::textColourId,juce::Colour(0xff36251b));
                addAndMakeVisible(*combo);
                ca=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state,id,*combo);
            }else if(dynamic_cast<juce::AudioParameterBool*>(p)){
                toggle=std::make_unique<juce::TextButton>("IN");
                toggle->setClickingTogglesState(true);
                addAndMakeVisible(*toggle);
                ba=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state,id,*toggle);
            }else {
                knob=std::make_unique<juce::Slider>();
                knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                knob->setRotaryParameters(3.926991f,8.63938f,true);
                knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,85,23);
                knob->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xfff1e2c3));
                knob->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xff33261a));
                addAndMakeVisible(*knob);
                sa=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,*knob);
            }
            setTooltip(p->getName(100));
        }
        void resized() override {
            auto b=getLocalBounds().reduced(2);
            label.setBounds(b.removeFromTop(29));
            if(knob)knob->setBounds(b);
            if(combo)combo->setBounds(b.withSizeKeepingCentre(juce::jmax(1,b.getWidth()-4),31));
            if(toggle)toggle->setBounds(b.withSizeKeepingCentre(72,42));
        }
    private:
        juce::Label label;
        std::unique_ptr<juce::Slider> knob;
        std::unique_ptr<juce::ComboBox> combo;
        std::unique_ptr<juce::TextButton> toggle;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sa;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ca;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ba;
    };
    RackPanel(SteamMasterProcessor& p,juce::String name,juce::String sub,
              std::initializer_list<const char*> first,std::initializer_list<const char*> second,int style)
     : title_(std::move(name)),subtitle_(std::move(sub)),style_(style) {
        for(const char* id:first){auto tile=std::make_unique<Tile>(p.state,id);addAndMakeVisible(*tile);a.push_back(std::move(tile));}
        for(const char* id:second){auto tile=std::make_unique<Tile>(p.state,id);addAndMakeVisible(*tile);b.push_back(std::move(tile));}
        if(!b.empty()){
            ocean.setButtonText("OCEAN");furnace.setButtonText("FURNACE");
            addAndMakeVisible(ocean);addAndMakeVisible(furnace);
            ocean.onClick=[this]{show(0);};furnace.onClick=[this]{show(1);};
        }
        show(0);
    }
    int idealHeight() const {return 85+156*juce::jmax(1,(int(juce::jmax(a.size(),b.size()))+6)/7);}
    void paint(juce::Graphics& g) override {
        auto box=getLocalBounds().toFloat().reduced(4);
        const bool dark=style_==3||style_==7||style_==8;
        g.setColour(juce::Colour(0xff513019));g.fillRoundedRectangle(box,13);
        g.setColour(dark?juce::Colour(0xff527175):juce::Colour(0xffe9d3ad));
        g.fillRoundedRectangle(box.reduced(5),10);
        g.setColour(juce::Colour(0xffbd8d47));
        g.drawRoundedRectangle(box.reduced(7),9,2);
        g.setColour(dark?juce::Colour(0xfff2d6a5):juce::Colour(0xff643f20));
        g.setFont(juce::Font(juce::FontOptions(26.f).withStyle("Bold")));
        g.drawText(title_,24,11,getWidth()-48,31,juce::Justification::centred);
        g.setFont(juce::Font(juce::FontOptions(12.f).withStyle("Bold")));
        g.drawText(subtitle_,24,42,getWidth()-48,18,juce::Justification::centred);
    }
    void resized() override {
        const int space=6, pad=20, width=juce::jmax(1,(getWidth()-40-space*6)/7);
        auto place=[&](auto& group){for(int i=0;i<int(group.size());++i)group[size_t(i)]->setBounds(pad+(i%7)*(width+space),72+(i/7)*156,width,149);};
        place(a);place(b);
        if(!b.empty()){ocean.setBounds(24,16,100,38);furnace.setBounds(128,16,100,38);}
    }
private:
    void show(int i) {
        for(auto& t:a)t->setVisible(i==0);
        for(auto& t:b)t->setVisible(i==1);
        ocean.setToggleState(i==0,juce::dontSendNotification);
        furnace.setToggleState(i==1,juce::dontSendNotification);
    }
    juce::String title_,subtitle_;
    int style_;
    juce::TextButton ocean,furnace;
    std::vector<std::unique_ptr<Tile>> a,b;
};

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

    addAndMakeVisible(rackViewport);
    rackCanvas.setLookAndFeel(&metalLook);
    rackViewport.setViewedComponent(&rackCanvas,false);
    rackViewport.setScrollBarsShown(true,false);
    auto addPanel=[this,&p](const char* title,const char* sub,std::initializer_list<const char*> a,
                            std::initializer_list<const char*> b,int style){
        auto panel=std::make_unique<RackPanel>(p,title,sub,a,b,style);
        rackCanvas.addAndMakeVisible(*panel);
        rackPanels.push_back(std::move(panel));
    };
addPanel("OCEAN / FURNACE","VARI-MU / SSL",{"ocean_in","ocean_mix","ocean_threshold_db","ocean_ratio","ocean_attack_ms","ocean_recovery","ocean_sc_hpf"},{"furnace_in","furnace_mix","furnace_threshold_db","furnace_ratio","furnace_attack","furnace_release","furnace_sc_hpf"},0);
addPanel("PARAMETRIC EQ","4-BAND EQUALIZER",{"parametric_in","parametric_mix","parametric_output_db","parametric_band_1_type","parametric_band_1_freq","parametric_band_1_q","parametric_band_1_gain","parametric_band_2_type","parametric_band_2_freq","parametric_band_2_q","parametric_band_2_gain","parametric_band_3_type","parametric_band_3_freq","parametric_band_3_q","parametric_band_3_gain","parametric_band_4_type","parametric_band_4_freq","parametric_band_4_q","parametric_band_4_gain"},{},1);
addPanel("PULTEC EQ","PROGRAM EQUALIZER",{"pultec_in","pultec_mix","pultec_low_freq","pultec_low_boost","pultec_low_atten","pultec_high_boost_freq","pultec_high_boost","pultec_bandwidth","pultec_high_atten_freq","pultec_high_atten","pultec_output_db"},{},2);
addPanel("SHADOW COMPRESSOR","OPTICAL / DISCRETE",{"shadow_in","shadow_mix","shadow_optical_in","shadow_optical_threshold","shadow_optical_makeup","shadow_discrete_in","shadow_discrete_threshold","shadow_discrete_gain","shadow_discrete_ratio","shadow_discrete_attack","shadow_discrete_recovery","shadow_sc_hpf","shadow_output_db"},{},3);
addPanel("EXCITER","HARMONIC ENHANCER",{"exciter_in","exciter_mix","exciter_mode","exciter_low_drive","exciter_low_freq","exciter_high_drive","exciter_high_freq","exciter_crossover","exciter_output_db"},{},4);
addPanel("BAXANDALL EQ","MID / SIDE",{"bax_mid_in","bax_mid_hpf","bax_mid_low_freq","bax_mid_low_gain","bax_mid_high_freq","bax_mid_high_gain","bax_mid_lpf","bax_mid_output_db","bax_side_in","bax_side_hpf","bax_side_low_freq","bax_side_low_gain","bax_side_high_freq","bax_side_high_gain","bax_side_lpf","bax_side_output_db","stereo_routing","stereo_link"},{},5);
addPanel("TAPE","ANALOG MASTERING MACHINE",{"tape_in","tape_mix","tape_type","tape_speed","tape_drive_db","tape_bias","tape_hf_trim_db"},{},6);
addPanel("STEREO IMAGE","STEREO FIELD",{"stereo_mono_maker_hz","stereo_width_percent","stereo_width_freq"},{},7);
addPanel("LIMITER","TRUE PEAK MASTER",{"limiter_in","limiter_mode","limiter_threshold_db","limiter_ceiling_dbtp","limiter_release_ms","limiter_auto_release","limiter_lookahead_ms","limiter_stereo_link","limiter_true_peak"},{},8);
    abSnapshots[0]=p.state.copyState();abSnapshots[1]=abSnapshots[0].createCopy();
    setResizable(true,true);setResizeLimits(820,520,1850,1300);
    setSize(1280,870);startTimerHz(30);timerCallback();
}
SteamMasterEditor::~SteamMasterEditor() {
    stopTimer();
    rackCanvas.setLookAndFeel(nullptr);
    rackViewport.setViewedComponent(nullptr,false);
    rackPanels.clear();
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
    repaint(0,0,getWidth(),juce::roundToInt(canvasHeight*float(getWidth())/canvasWidth));
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
    const auto m=processor.getMeterSnapshot();
    const auto db=[](float x){return 20.f*std::log10(juce::jmax(0.000001f,x));};
    text(g,"METERS",1649,133,171,47,30,cream);
    text(g,"TP "+juce::String(db(m.truePeak),1),1650,200,170,30,24,cream);
    text(g,"GR "+juce::String(m.limiterReductionDb,1),1650,260,170,30,24,cream);
    text(g,"COR "+juce::String(m.correlation,2),1650,319,170,30,24,cream);
    text(g,"L",1848,110,49,36,24,cream);text(g,"R",1910,110,49,36,24,cream);
    const float peaks[]={m.leftPeak,m.rightPeak};
    for(int bar=0;bar<2;++bar) {
        const int lit=juce::jlimit(0,23,int((db(peaks[bar])+60.f)*23.f/60.f));
        for(int i=0;i<23;++i) {
            g.setColour(i<lit?(i>20?juce::Colour(0xffe74a28):i>17?
                        juce::Colour(0xffffbf37):juce::Colour(0xff4dca7d)):juce::Colour(0xff2c4845));
            g.fillRect(1850+bar*65,156+(22-i)*9,38,5);
        }
    }
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    const float s=float(getWidth())/float(canvasWidth);
    g.addTransform(juce::AffineTransform::scale(s));
    paintFaceplate(g);paintLabelsAndScales(g);paintMeterPlaceholders(g);
}
void SteamMasterEditor::resized() {
    const float s=float(getWidth())/float(canvasWidth);
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
    const int headerHeight=juce::roundToInt(canvasHeight*s);
    rackViewport.setBounds(0,headerHeight,getWidth(),juce::jmax(0,getHeight()-headerHeight));
    int y=0;
    for(auto& panel:rackPanels){const int h=juce::roundToInt(panel->idealHeight()*s*1.25f);panel->setBounds(0,y,getWidth(),juce::jmax(180,h));y+=panel->getHeight();}
    rackCanvas.setSize(juce::jmax(1,getWidth()),juce::jmax(1,y));
}
