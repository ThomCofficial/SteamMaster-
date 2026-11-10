#include "PluginEditor.h"
#include <cmath>
#include <map>
#ifdef STEAMMASTER_HAS_ARTWORK
#include <BinaryData.h>
#endif

namespace {
const juce::Colour brass (0xffbb9353);
const juce::Colour ivory (0xfff0dec0);
const juce::Colour steel (0xff263742);
const juce::Colour deep (0xff10191d);
class SteamLook final : public juce::LookAndFeel_V4 {
    std::map<juce::String, juce::Image> knobs;
    std::map<juce::String, std::pair<juce::Image, juce::Image>> switches;
public:
    SteamLook() {
#ifdef STEAMMASTER_HAS_ARTWORK
        auto load = [this](const char* id, const void* bytes, size_t count) {
            auto image = juce::ImageFileFormat::loadFrom(bytes, count);
            if (image.isValid()) knobs.emplace(id, image);
        };
        load("global_input_db", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("global_output_db", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("ocean_threshold_db", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("ocean_ratio", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("ocean_attack_ms", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("ocean_recovery", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("ocean_sc_hpf", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("ocean_mix", BinaryData::Continuous_Small_Brass_png, (size_t) BinaryData::Continuous_Small_Brass_pngSize);
        load("furnace_threshold_db", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("furnace_ratio", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("furnace_attack", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("furnace_release", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("furnace_sc_hpf", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("furnace_mix", BinaryData::Continuous_Small_Brass_png, (size_t) BinaryData::Continuous_Small_Brass_pngSize);
        load("parametric_output_db", BinaryData::Output_png, (size_t) BinaryData::Output_pngSize);
        load("parametric_mix", BinaryData::Mix_png, (size_t) BinaryData::Mix_pngSize);
        load("pultec_low_freq", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("pultec_low_boost", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("pultec_low_atten", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("pultec_high_boost_freq", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("pultec_high_boost", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("pultec_high_atten_freq", BinaryData::Stepped_Lever_Brass_png, (size_t) BinaryData::Stepped_Lever_Brass_pngSize);
        load("pultec_high_atten", BinaryData::Continuous_Large_Brass_png, (size_t) BinaryData::Continuous_Large_Brass_pngSize);
        load("pultec_bandwidth", BinaryData::Continuous_Small_Brass_png, (size_t) BinaryData::Continuous_Small_Brass_pngSize);
        load("pultec_output_db", BinaryData::Continuous_Small_Brass_png, (size_t) BinaryData::Continuous_Small_Brass_pngSize);
        load("pultec_mix", BinaryData::Continuous_Small_Brass_png, (size_t) BinaryData::Continuous_Small_Brass_pngSize);
        load("shadow_optical_threshold", BinaryData::Optical_Threshold_png, (size_t) BinaryData::Optical_Threshold_pngSize);
        load("shadow_optical_makeup", BinaryData::Optical_Gain_png, (size_t) BinaryData::Optical_Gain_pngSize);
        load("shadow_discrete_threshold", BinaryData::Discrete_Threshold_png, (size_t) BinaryData::Discrete_Threshold_pngSize);
        load("shadow_discrete_gain", BinaryData::Discrete_Gain_png, (size_t) BinaryData::Discrete_Gain_pngSize);
        load("shadow_discrete_ratio", BinaryData::Discrete_Ratio_png, (size_t) BinaryData::Discrete_Ratio_pngSize);
        load("shadow_discrete_attack", BinaryData::Discrete_Attack_png, (size_t) BinaryData::Discrete_Attack_pngSize);
        load("shadow_discrete_recovery", BinaryData::Discrete_Recovery_png, (size_t) BinaryData::Discrete_Recovery_pngSize);
        load("shadow_sc_hpf", BinaryData::Sidechain_HPF_png, (size_t) BinaryData::Sidechain_HPF_pngSize);
        load("shadow_output_db", BinaryData::Output_png, (size_t) BinaryData::Output_pngSize);
        load("shadow_mix", BinaryData::Mix_png, (size_t) BinaryData::Mix_pngSize);
        load("exciter_low_drive", BinaryData::Low_Drive_Brass_Transparent_png, (size_t) BinaryData::Low_Drive_Brass_Transparent_pngSize);
        load("exciter_low_freq", BinaryData::Low_Frequency_Brass_Transparent_png, (size_t) BinaryData::Low_Frequency_Brass_Transparent_pngSize);
        load("exciter_high_drive", BinaryData::High_Drive_Brass_Transparent_png, (size_t) BinaryData::High_Drive_Brass_Transparent_pngSize);
        load("exciter_high_freq", BinaryData::High_Frequency_Brass_Transparent_png, (size_t) BinaryData::High_Frequency_Brass_Transparent_pngSize);
        load("exciter_crossover", BinaryData::Crossover_Brass_Transparent_png, (size_t) BinaryData::Crossover_Brass_Transparent_pngSize);
        load("exciter_output_db", BinaryData::Output_Brass_Transparent_png, (size_t) BinaryData::Output_Brass_Transparent_pngSize);
        load("exciter_mix", BinaryData::Mix_Brass_Transparent_png, (size_t) BinaryData::Mix_Brass_Transparent_pngSize);
        load("tape_type", BinaryData::Type_FrontOn_png, (size_t) BinaryData::Type_FrontOn_pngSize);
        load("tape_speed", BinaryData::Speed_FrontOn_png, (size_t) BinaryData::Speed_FrontOn_pngSize);
        load("tape_drive_db", BinaryData::Drive_FrontOn_png, (size_t) BinaryData::Drive_FrontOn_pngSize);
        load("tape_bias", BinaryData::Bias_FrontOn_png, (size_t) BinaryData::Bias_FrontOn_pngSize);
        load("tape_hf_trim_db", BinaryData::HF_Trim_FrontOn_png, (size_t) BinaryData::HF_Trim_FrontOn_pngSize);
        load("tape_mix", BinaryData::Mix_FrontOn_png, (size_t) BinaryData::Mix_FrontOn_pngSize);
        load("stereo_mono_maker_hz", BinaryData::Mono_Maker_FrontFacing_Transparent_png, (size_t) BinaryData::Mono_Maker_FrontFacing_Transparent_pngSize);
        load("stereo_width_percent", BinaryData::Stereo_Width_FrontFacing_Transparent_png, (size_t) BinaryData::Stereo_Width_FrontFacing_Transparent_pngSize);
        load("stereo_width_freq", BinaryData::Width_Frequency_FrontFacing_Transparent_png, (size_t) BinaryData::Width_Frequency_FrontFacing_Transparent_pngSize);
        load("limiter_threshold_db", BinaryData::Threshold_FrontFacing_Transparent_png, (size_t) BinaryData::Threshold_FrontFacing_Transparent_pngSize);
        load("limiter_ceiling_dbtp", BinaryData::Ceiling_FrontFacing_Transparent_png, (size_t) BinaryData::Ceiling_FrontFacing_Transparent_pngSize);
        load("limiter_release_ms", BinaryData::Release_FrontFacing_Transparent_png, (size_t) BinaryData::Release_FrontFacing_Transparent_pngSize);
        load("limiter_lookahead_ms", BinaryData::Lookahead_FrontFacing_Transparent_png, (size_t) BinaryData::Lookahead_FrontFacing_Transparent_pngSize);
        load("limiter_stereo_link", BinaryData::Stereo_Link_FrontFacing_Transparent_png, (size_t) BinaryData::Stereo_Link_FrontFacing_Transparent_pngSize);
        load("parametric_band_1_gain", BinaryData::Band1_Gain_png, (size_t) BinaryData::Band1_Gain_pngSize);
        load("parametric_band_1_freq", BinaryData::Band1_Frequency_png, (size_t) BinaryData::Band1_Frequency_pngSize);
        load("parametric_band_1_q", BinaryData::Band1_Q_png, (size_t) BinaryData::Band1_Q_pngSize);
        load("parametric_band_2_gain", BinaryData::Band2_Gain_png, (size_t) BinaryData::Band2_Gain_pngSize);
        load("parametric_band_2_freq", BinaryData::Band2_Frequency_png, (size_t) BinaryData::Band2_Frequency_pngSize);
        load("parametric_band_2_q", BinaryData::Band2_Q_png, (size_t) BinaryData::Band2_Q_pngSize);
        load("parametric_band_3_gain", BinaryData::Band3_Gain_png, (size_t) BinaryData::Band3_Gain_pngSize);
        load("parametric_band_3_freq", BinaryData::Band3_Frequency_png, (size_t) BinaryData::Band3_Frequency_pngSize);
        load("parametric_band_3_q", BinaryData::Band3_Q_png, (size_t) BinaryData::Band3_Q_pngSize);
        load("parametric_band_4_gain", BinaryData::Band4_Gain_png, (size_t) BinaryData::Band4_Gain_pngSize);
        load("parametric_band_4_freq", BinaryData::Band4_Frequency_png, (size_t) BinaryData::Band4_Frequency_pngSize);
        load("parametric_band_4_q", BinaryData::Band4_Q_png, (size_t) BinaryData::Band4_Q_pngSize);
        load("bax_mid_hpf", BinaryData::Bax_HPF_png, (size_t) BinaryData::Bax_HPF_pngSize);
        load("bax_mid_low_freq", BinaryData::Bax_Low_Shelf_Frequency_png, (size_t) BinaryData::Bax_Low_Shelf_Frequency_pngSize);
        load("bax_mid_low_gain", BinaryData::Bax_Low_Shelf_Gain_png, (size_t) BinaryData::Bax_Low_Shelf_Gain_pngSize);
        load("bax_mid_high_freq", BinaryData::Bax_High_Shelf_Frequency_png, (size_t) BinaryData::Bax_High_Shelf_Frequency_pngSize);
        load("bax_mid_high_gain", BinaryData::Bax_High_Shelf_Gain_png, (size_t) BinaryData::Bax_High_Shelf_Gain_pngSize);
        load("bax_mid_lpf", BinaryData::Bax_LPF_png, (size_t) BinaryData::Bax_LPF_pngSize);
        load("bax_mid_output_db", BinaryData::Bax_Output_png, (size_t) BinaryData::Bax_Output_pngSize);
        load("bax_side_hpf", BinaryData::Bax_HPF_png, (size_t) BinaryData::Bax_HPF_pngSize);
        load("bax_side_low_freq", BinaryData::Bax_Low_Shelf_Frequency_png, (size_t) BinaryData::Bax_Low_Shelf_Frequency_pngSize);
        load("bax_side_low_gain", BinaryData::Bax_Low_Shelf_Gain_png, (size_t) BinaryData::Bax_Low_Shelf_Gain_pngSize);
        load("bax_side_high_freq", BinaryData::Bax_High_Shelf_Frequency_png, (size_t) BinaryData::Bax_High_Shelf_Frequency_pngSize);
        load("bax_side_high_gain", BinaryData::Bax_High_Shelf_Gain_png, (size_t) BinaryData::Bax_High_Shelf_Gain_pngSize);
        load("bax_side_lpf", BinaryData::Bax_LPF_png, (size_t) BinaryData::Bax_LPF_pngSize);
        load("bax_side_output_db", BinaryData::Bax_Output_png, (size_t) BinaryData::Bax_Output_pngSize);
#endif
#ifdef STEAMMASTER_HAS_ARTWORK
        switches.emplace("shadow_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Global_IN_ON_png, (size_t)BinaryData::Global_IN_ON_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Global_IN_OFF_png, (size_t)BinaryData::Global_IN_OFF_pngSize)));
        switches.emplace("shadow_optical_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Optical_IN_ON_png, (size_t)BinaryData::Optical_IN_ON_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Optical_IN_OFF_png, (size_t)BinaryData::Optical_IN_OFF_pngSize)));
        switches.emplace("shadow_discrete_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Discrete_IN_ON_png, (size_t)BinaryData::Discrete_IN_ON_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Discrete_IN_OFF_png, (size_t)BinaryData::Discrete_IN_OFF_pngSize)));
        switches.emplace("exciter_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::IN_ON_png, (size_t)BinaryData::IN_ON_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::IN_OFF_png, (size_t)BinaryData::IN_OFF_pngSize)));
        switches.emplace("tape_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::IN_On_FrontOn_png, (size_t)BinaryData::IN_On_FrontOn_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::IN_Off_FrontOn_png, (size_t)BinaryData::IN_Off_FrontOn_pngSize)));
        switches.emplace("limiter_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::IN_On_png, (size_t)BinaryData::IN_On_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::IN_Off_png, (size_t)BinaryData::IN_Off_pngSize)));
        switches.emplace("limiter_true_peak", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::True_Peak_On_png, (size_t)BinaryData::True_Peak_On_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::True_Peak_Off_png, (size_t)BinaryData::True_Peak_Off_pngSize)));
        switches.emplace("limiter_auto_release", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Auto_Release_On_png, (size_t)BinaryData::Auto_Release_On_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Auto_Release_Off_png, (size_t)BinaryData::Auto_Release_Off_pngSize)));
        switches.emplace("bax_mid_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Channel_IN_Lit_png, (size_t)BinaryData::Bax_Channel_IN_Lit_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Channel_IN_Unlit_png, (size_t)BinaryData::Bax_Channel_IN_Unlit_pngSize)));
        switches.emplace("bax_side_in", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Channel_IN_Lit_png, (size_t)BinaryData::Bax_Channel_IN_Lit_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Channel_IN_Unlit_png, (size_t)BinaryData::Bax_Channel_IN_Unlit_pngSize)));
        switches.emplace("stereo_link", std::make_pair(
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Link_On_Lit_png, (size_t)BinaryData::Bax_Link_On_Lit_pngSize),
            juce::ImageFileFormat::loadFrom(BinaryData::Bax_Link_Off_Unlit_png, (size_t)BinaryData::Bax_Link_Off_Unlit_pngSize)));
#endif
        setColour(juce::Slider::textBoxTextColourId, ivory);
        setColour(juce::Slider::textBoxBackgroundColourId, deep);
        setColour(juce::Slider::textBoxOutlineColourId, brass.withAlpha(0.45f));
        setColour(juce::ComboBox::backgroundColourId, steel);
        setColour(juce::ComboBox::textColourId, ivory);
        setColour(juce::ComboBox::outlineColourId, brass);
        setColour(juce::Label::textColourId, ivory);
        setColour(juce::ToggleButton::textColourId, ivory);
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float position, float start, float end, juce::Slider& slider) override {
#ifdef STEAMMASTER_HAS_ARTWORK
        const auto it = knobs.find(slider.getComponentID());
        if (it != knobs.end() && it->second.isValid()) {
            const auto& sprite = it->second;
            const float diameter = (float) juce::jmin(w, h) * 0.72f;
            const float scale = diameter / (float) juce::jmax(sprite.getWidth(), sprite.getHeight());
            const float angle = start + position * (end - start);
            const float imageW = (float) sprite.getWidth(), imageH = (float) sprite.getHeight();
            juce::Graphics::ScopedSaveState saved(g);
            auto transform = juce::AffineTransform::rotation(angle, imageW / 2.f, imageH / 2.f)
                   .scaled(scale)
                   .translated((float)x + w / 2.f - imageW * scale / 2.f,
                               (float)y + h / 2.f - imageH * scale / 2.f);
            // SteamVox lesson: full-colour sprites must NOT be drawn as alpha masks.
            g.drawImageTransformed(sprite, transform, false);
            return;
        }
#endif
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(7.f);
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.43f;
        const auto centre = bounds.getCentre();
        auto rim = juce::Rectangle<float>(centre.x-radius, centre.y-radius, 2*radius, 2*radius);
        g.setColour(brass.darker(0.65f)); g.fillEllipse(rim.expanded(4.f));
        g.setColour(brass); g.drawEllipse(rim.expanded(3.f), 2.f);
        juce::ColourGradient gradient(juce::Colour(0xff64717b), centre.x-radius, centre.y-radius,
                                      juce::Colour(0xff161e25), centre.x+radius, centre.y+radius, false);
        gradient.addColour(0.48, juce::Colour(0xffc6b9a1));
        g.setGradientFill(gradient); g.fillEllipse(rim);
        const float theta = start + position*(end-start);
        auto tip = centre + juce::Point<float>(std::sin(theta),-std::cos(theta))*(radius*.79f);
        auto base = centre + juce::Point<float>(std::sin(theta),-std::cos(theta))*(radius*.20f);
        g.setColour(juce::Colour(0xffffe8b2));
        g.drawLine(base.x,base.y,tip.x,tip.y,3.f);
    }

 void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool over,bool down) override {
#ifdef STEAMMASTER_HAS_ARTWORK
    auto it = switches.find(b.getComponentID());
    if (it != switches.end()) {
        const auto& sprite = b.getToggleState() ? it->second.first : it->second.second;
        if (sprite.isValid()) {
            g.drawImageWithin(sprite, 0, 0, b.getWidth(), b.getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, false);
            return;
        }
    }
#endif
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


juce::Point<float> approvedPosition(const juce::String& id) {
    struct Placement {const char* id;float x,y;};
    static constexpr Placement positions[] = {
       {"global_in",130.f,384.f},
       {"global_input_db",283.f,417.f},
       {"global_output_db",466.f,417.f},
       {"global_oversampling",750.f,462.f},
       {"ocean_in",214.f,282.f},
       {"ocean_mix",586.f,292.f},
       {"ocean_threshold_db",129.f,176.f},
       {"ocean_ratio",289.f,176.f},
       {"ocean_attack_ms",430.f,176.f},
       {"ocean_recovery",586.f,176.f},
       {"ocean_sc_hpf",344.f,292.f},
       {"furnace_in",210.f,283.f},
       {"furnace_mix",587.f,293.f},
       {"furnace_threshold_db",109.f,177.f},
       {"furnace_ratio",268.f,176.f},
       {"furnace_attack",429.f,176.f},
       {"furnace_release",584.f,176.f},
       {"furnace_sc_hpf",343.f,293.f},
       {"parametric_in",88.f,414.f},
       {"parametric_mix",1377.f,412.f},
       {"parametric_output_db",607.f,411.f},
       {"pultec_in",224.f,567.f},
       {"pultec_mix",1960.f,555.f},
       {"pultec_low_freq",237.f,330.f},
       {"pultec_low_boost",538.f,328.f},
       {"pultec_low_atten",841.f,328.f},
       {"pultec_high_boost_freq",1143.f,328.f},
       {"pultec_high_boost",1360.f,328.f},
       {"pultec_bandwidth",1555.f,328.f},
       {"pultec_high_atten_freq",1770.f,328.f},
       {"pultec_high_atten",1970.f,327.f},
       {"pultec_output_db",1651.f,557.f},
       {"shadow_in",232.f,646.f},
       {"shadow_mix",1277.f,641.f},
       {"shadow_optical_in",280.f,245.f},
       {"shadow_optical_threshold",159.f,376.f},
       {"shadow_optical_makeup",421.f,375.f},
       {"shadow_discrete_in",1417.f,229.f},
       {"shadow_discrete_threshold",1045.f,288.f},
       {"shadow_discrete_gain",1366.f,451.f},
       {"shadow_discrete_ratio",1262.f,287.f},
       {"shadow_discrete_attack",957.f,455.f},
       {"shadow_discrete_recovery",1156.f,454.f},
       {"shadow_sc_hpf",546.f,641.f},
       {"shadow_output_db",909.f,641.f},
       {"exciter_in",123.f,339.f},
       {"exciter_mix",1450.f,339.f},
       {"exciter_mode",123.f,250.f},
       {"exciter_low_drive",286.f,251.f},
       {"exciter_low_freq",428.f,251.f},
       {"exciter_high_drive",858.f,250.f},
       {"exciter_high_freq",998.f,250.f},
       {"exciter_crossover",1198.f,249.f},
       {"exciter_output_db",1321.f,335.f},
       {"bax_mid_in",115.f,500.f},
       {"bax_mid_hpf",290.f,466.f},
       {"bax_mid_low_freq",476.f,471.f},
       {"bax_mid_low_gain",668.f,473.f},
       {"bax_mid_high_freq",1007.f,478.f},
       {"bax_mid_high_gain",845.f,473.f},
       {"bax_mid_lpf",1186.f,475.f},
       {"bax_mid_output_db",1393.f,474.f},
       {"bax_side_in",115.f,835.f},
       {"bax_side_hpf",289.f,807.f},
       {"bax_side_low_freq",479.f,808.f},
       {"bax_side_low_gain",675.f,808.f},
       {"bax_side_high_freq",1011.f,810.f},
       {"bax_side_high_gain",845.f,805.f},
       {"bax_side_lpf",1189.f,809.f},
       {"bax_side_output_db",1395.f,811.f},
       {"tape_in",102.f,339.f},
       {"tape_mix",1392.f,362.f},
       {"tape_type",289.f,377.f},
       {"tape_speed",512.f,377.f},
       {"tape_drive_db",726.f,361.f},
       {"tape_bias",949.f,361.f},
       {"tape_hf_trim_db",1174.f,363.f},
       {"stereo_mono_maker_hz",507.f,151.f},
       {"stereo_width_percent",758.f,152.f},
       {"stereo_width_freq",1006.f,153.f},
       {"limiter_in",354.f,378.f},
       {"limiter_mode",1107.f,417.f},
       {"limiter_threshold_db",477.f,383.f},
       {"limiter_ceiling_dbtp",638.f,382.f},
       {"limiter_release_ms",807.f,383.f},
       {"limiter_auto_release",861.f,438.f},
       {"limiter_lookahead_ms",991.f,325.f},
       {"limiter_stereo_link",1138.f,325.f},
       {"limiter_true_peak",962.f,415.f},
       {"parametric_band_1_type",165.f,358.f},
       {"parametric_band_1_freq",221.f,225.f},
       {"parametric_band_1_q",345.f,225.f},
       {"parametric_band_1_gain",100.f,225.f},
       {"parametric_band_2_type",563.f,358.f},
       {"parametric_band_2_freq",619.f,225.f},
       {"parametric_band_2_q",743.f,225.f},
       {"parametric_band_2_gain",498.f,225.f},
       {"parametric_band_3_type",919.f,358.f},
       {"parametric_band_3_freq",975.f,225.f},
       {"parametric_band_3_q",1099.f,225.f},
       {"parametric_band_3_gain",854.f,225.f},
       {"parametric_band_4_type",1270.f,358.f},
       {"parametric_band_4_freq",1326.f,225.f},
       {"parametric_band_4_q",1450.f,225.f},
       {"parametric_band_4_gain",1205.f,225.f},
       {"stereo_routing",616.f,228.f},
       {"stereo_link",838.f,226.f},
    };
    for(const auto& p:positions) if(id==p.id)return {p.x,p.y};
    return {-1.f,-1.f};
}

SteamMasterEditor::Page::Page(juce::AudioProcessor& processor,juce::AudioProcessorValueTreeState& state,
                               const juce::String& s):source(static_cast<SteamMasterProcessor&>(processor)),section(s) {
    // Decode the approved source art once, on the message thread, never in paint/audio callbacks.
#ifdef STEAMMASTER_HAS_ARTWORK
    const char* asset = nullptr;
    int assetSize = 0;
    if (section == "Global") { asset=BinaryData::abyssal_steampunk_mastering_faceplate_png; assetSize=BinaryData::abyssal_steampunk_mastering_faceplate_pngSize; }
    else if(section == "Ocean") { asset=BinaryData::Ocean_Furnace_Clean_Faceplate_png; assetSize=BinaryData::Ocean_Furnace_Clean_Faceplate_pngSize; }
    else if(section == "Furnace") { asset=BinaryData::Ocean_Furnace_Clean_Faceplate_png; assetSize=BinaryData::Ocean_Furnace_Clean_Faceplate_pngSize; }
    else if(section == "Parametric") { asset=BinaryData::Parametric_EQ_Clean_Faceplate_png; assetSize=BinaryData::Parametric_EQ_Clean_Faceplate_pngSize; }
    else if(section == "Pultec") { asset=BinaryData::Pultec_EQ_Faceplate_png; assetSize=BinaryData::Pultec_EQ_Faceplate_pngSize; }
    else if(section == "Shadow") { asset=BinaryData::Shadow_Clean_Faceplate_EmptySockets_4x_png; assetSize=BinaryData::Shadow_Clean_Faceplate_EmptySockets_4x_pngSize; }
    else if(section == "Exciter") { asset=BinaryData::Exciter_Empty_Socket_Faceplate_4x_png; assetSize=BinaryData::Exciter_Empty_Socket_Faceplate_4x_pngSize; }
    else if(section == "Bax") { asset=BinaryData::Baxandall_Rack_Faceplate_Cropped_png; assetSize=BinaryData::Baxandall_Rack_Faceplate_Cropped_pngSize; }
    else if(section == "Tape") { asset=BinaryData::TapeMachine_Faceplate_Clean_4x_png; assetSize=BinaryData::TapeMachine_Faceplate_Clean_4x_pngSize; }
    else if(section == "Stereo + Limiter") { asset=BinaryData::StereoImage_Limiter_Clean_Redesigned_png; assetSize=BinaryData::StereoImage_Limiter_Clean_Redesigned_pngSize; }
    if (asset != nullptr && assetSize > 0) {
       contents.art = juce::ImageFileFormat::loadFrom(asset, (size_t)assetSize);
    }
#endif
    addAndMakeVisible(contents);
    for(auto* p:processor.getParameters()) {
        auto* withID=dynamic_cast<juce::AudioProcessorParameterWithID*>(p);
        if(!withID)continue;
        const auto fullName=withID->getName(200);
        const bool groupBax=section=="Bax" &&
            (fullName.startsWith("Bax Mid / ") || fullName.startsWith("Bax Side / ") ||
             fullName=="Stereo / Routing" || fullName=="Stereo / Link");
        const bool groupStereo=section=="Stereo + Limiter" &&
            (fullName.startsWith("Limiter / ") ||
             (fullName.startsWith("Stereo / ") &&
              fullName!="Stereo / Routing" && fullName!="Stereo / Link"));
        if(!groupBax && !groupStereo && !fullName.startsWith(section+" / "))continue;
        const auto id=withID->paramID;
        ids.push_back(id);
        auto label=std::make_unique<juce::Label>();
        label->setText(withID->getName(200).fromFirstOccurrenceOf(" / ",false,false),juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::Font(juce::FontOptions(13.f)));
        contents.addAndMakeVisible(*label);
        labels.push_back(std::move(label));
        if(auto* choice=dynamic_cast<juce::AudioParameterChoice*>(p)) {
            // Rotary detents for frequency selectors and vintage switch banks.
            // Multi-mode buttons remain a readable popup selector.
            const bool popup=id=="global_oversampling" || id=="exciter_mode" ||
                             id=="limiter_mode" || id=="stereo_routing";
            if(popup) {
                auto box=std::make_unique<juce::ComboBox>();
                box->addItemList(choice->choices,1);
                contents.addAndMakeVisible(*box);
                combos.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state,id,*box));
                controls.push_back(std::move(box));
            } else {
                auto slider=std::make_unique<juce::Slider>();
                slider->setComponentID(id);
            slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
                slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,108,23);
                slider->setRange(0,juce::jmax(0,choice->choices.size()-1),1);
                slider->setRotaryParameters(juce::MathConstants<float>::pi*1.25f,
                                             juce::MathConstants<float>::pi*2.75f,true);
                const auto choices=choice->choices;
                slider->textFromValueFunction=[choices](double v) {
                    return choices[juce::jlimit(0,choices.size()-1,juce::roundToInt(v))];
                };
                slider->valueFromTextFunction=[choices](const juce::String& text) {
                    return (double)juce::jmax(0,choices.indexOf(text));
                };
                contents.addAndMakeVisible(*slider);
                sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,*slider));
                controls.push_back(std::move(slider));
            }
        } else if(dynamic_cast<juce::AudioParameterBool*>(p)) {
            auto b=std::make_unique<juce::ToggleButton>();
            b->setComponentID(id);
            b->setButtonText(id.endsWith("_in")?"IN":id.contains("link")?"LINK":id.contains("auto")?"AUTO":id.contains("peak")?"TP":"ON");
            contents.addAndMakeVisible(*b);
            buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state,id,*b));
            controls.push_back(std::move(b));
        } else {
            auto slider=std::make_unique<juce::Slider>();
            slider->setComponentID(id);
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
    if(section=="Global"){
        slotA=source.state.copyState();
        slotB=slotA.createCopy();
        auto addButton=[this](juce::String title,juce::String name,std::function<void()> callback) {
            auto button=std::make_unique<juce::TextButton>(title);
            button->setName(name);
            button->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff4a3524));
            button->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xffc18c39));
            button->setColour(juce::TextButton::textColourOffId,juce::Colour(0xffffe2a6));
            button->onClick=std::move(callback);
            contents.addAndMakeVisible(*button);
            headerButtons.push_back(std::move(button));
        };
        addButton("A","slotA",[this]{switchSlot(false);});
        addButton("B","slotB",[this]{switchSlot(true);});
        addButton("<","previous",[this]{navigatePreset(-1);});
        addButton(">","next",[this]{navigatePreset(1);});
        addButton("SAVE","save",[this]{savePreset();});
        presetLabel=std::make_unique<juce::Label>();
        presetLabel->setJustificationType(juce::Justification::centred);
        presetLabel->setFont(juce::Font(juce::FontOptions(17.f,juce::Font::italic)));
        presetLabel->setColour(juce::Label::backgroundColourId,juce::Colour(0xff292319));
        presetLabel->setColour(juce::Label::textColourId,juce::Colour(0xffedd6a0));
        presetLabel->setText(presetTitle,juce::dontSendNotification);
        contents.addAndMakeVisible(*presetLabel);
    }
    startTimerHz(30);
}

juce::File SteamMasterEditor::Page::presetDirectory() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("ThomC").getChildFile("SteamMaster").getChildFile("Presets");
}
void SteamMasterEditor::Page::switchSlot(bool toB) {
    if(usingB==toB)return;
    if(usingB) slotB=source.state.copyState();
    else slotA=source.state.copyState();
    auto state=toB?slotB:slotA;
    if(state.isValid()) source.state.replaceState(state.createCopy());
    usingB=toB;
    if(presetLabel) presetLabel->setText(presetTitle+(toB?" [B]":" [A]"),juce::dontSendNotification);
}
void SteamMasterEditor::Page::savePreset() {
    const auto folder=presetDirectory();
    if(folder.createDirectory().failed()) {
       if(presetLabel)presetLabel->setText("Cannot write preset folder",juce::dontSendNotification);
       return;
    }
    const auto stamp=juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
    const auto destination=folder.getChildFile("SteamMaster_"+stamp+".xml");
    auto xml=source.state.copyState().createXml();
    if(xml!=nullptr && xml->writeTo(destination)) {
       presetTitle=destination.getFileNameWithoutExtension();
       if(presetLabel)presetLabel->setText(presetTitle,juce::dontSendNotification);
    } else if(presetLabel)presetLabel->setText("Preset save failed",juce::dontSendNotification);
}
void SteamMasterEditor::Page::navigatePreset(int direction) {
    const auto dir=presetDirectory();
    auto files=dir.findChildFiles(juce::File::findFiles,false,"*.xml");
    if(files.isEmpty()){
       if(presetLabel)presetLabel->setText("No saved presets",juce::dontSendNotification);
       return;
    }
    int index=-1;
    for(int i=0;i<files.size();++i)
       if(files.getReference(i).getFileNameWithoutExtension()==presetTitle){index=i;break;}
    const int nextIndex=(index+direction+files.size())%files.size();
    auto xml=juce::XmlDocument::parse(files.getReference(nextIndex));
    if(xml!=nullptr && xml->hasTagName(source.state.state.getType().toString())) {
       source.state.replaceState(juce::ValueTree::fromXml(*xml));
       presetTitle=files.getReference(nextIndex).getFileNameWithoutExtension();
       if(usingB)slotB=source.state.copyState();else slotA=source.state.copyState();
       if(presetLabel)presetLabel->setText(presetTitle,juce::dontSendNotification);
    } else if(presetLabel) presetLabel->setText("Invalid preset",juce::dontSendNotification);
}

void SteamMasterEditor::Page::timerCallback(){repaint(juce::Rectangle<int>(getWidth()-295,0,290,30));}
void SteamMasterEditor::Page::paint(juce::Graphics& g) {
    g.fillAll(deep);
    g.setColour(brass);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(3),8.f,1.6f);
    g.setFont(juce::Font(juce::FontOptions(17.f,juce::Font::bold)));
    g.setColour(ivory);
    g.drawText(section.toUpperCase(),getLocalBounds().removeFromTop(27).reduced(12,0),
               juce::Justification::centredLeft);
    // Tiny independent steampunk instrument: no audio thread paints directly.
    // These measurements represent real processor telemetry, not knob positions.
    auto meter = [this,&g](const juce::String& label,float value,float low,float high,int x) {
       if(getWidth()<800)return;
       const float normalized=juce::jlimit(0.f,1.f,(value-low)/(high-low));
       juce::Rectangle<float> plate((float)x,4.f,128.f,21.f);
       g.setColour(juce::Colour(0xff3d2d23));g.fillRoundedRectangle(plate,4.f);
       g.setColour(brass);g.drawRoundedRectangle(plate,4.f,1.f);
       g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
       g.setColour(ivory);g.drawText(label,plate.withWidth(49.f),juce::Justification::centred,true);
       auto active=plate.withTrimmedLeft(51.f).reduced(4.f);
       g.setColour(juce::Colour(0xff131a1c));g.fillRect(active);
       g.setColour(juce::Colour(0xffd7ac69));
       g.fillRect(active.withWidth(active.getWidth()*normalized));
       g.setColour(juce::Colour(0xffffe4a3));
       const float needleX=active.getX()+active.getWidth()*normalized;
       g.drawLine(needleX,active.getY()-2.f,needleX,active.getBottom()+2.f,1.5f);
    };
    const auto read=[](const std::atomic<float>& p){return p.load(std::memory_order_relaxed);};
    int pos=getWidth()-279;
    if(section=="Ocean")meter("GR dB",-read(source.meterOceanGr),0.f,20.f,pos+145);
    else if(section=="Furnace")meter("GR dB",-read(source.meterFurnaceGr),0.f,20.f,pos+145);
    else if(section=="Shadow") {
       meter("OPTO",-read(source.meterShadowOpticalGr),0.f,20.f,pos);
       meter("DISC",-read(source.meterShadowDiscreteGr),0.f,20.f,pos+145);
    }
    else if(section=="Stereo + Limiter") {
       meter("CORR",read(source.meterCorrelation),-1.f,1.f,pos);
       meter("GR dB",read(source.meterLimiterGr),0.f,20.f,pos+145);
    }
    else if(section=="Global")meter("dBFS",read(source.meterOutputDb),-60.f,0.f,pos+145);
}
void SteamMasterEditor::Page::resized() {
    auto area=getLocalBounds().reduced(10);
    area.removeFromTop(28);
    contents.setBounds(area);
    const bool artValid=contents.art.isValid();
    const int artWidth=juce::jmax(500,area.getWidth()-15);
    const int artHeight=artValid
        ?juce::jmax(140,juce::roundToInt(artWidth*(double)contents.art.getHeight()/contents.art.getWidth()))
        :260;
    const float sx=artValid?(float)artWidth/contents.art.getWidth():1.f;
    const float sy=artValid?(float)artHeight/contents.art.getHeight():1.f;
    int overflow=0;
    for(int i=0;i<(int)controls.size();++i)
       if(approvedPosition(ids[(size_t)i]).x<0)++overflow;
    const int cellW=140,cellH=105;
    const int cols=juce::jmax(1,artWidth/cellW);
    const int extraRows=(overflow+cols-1)/cols;
    const int totalHeight=artHeight+(extraRows?extraRows*cellH+28:0);
    contents.artHeight=artHeight;
    contents.setBounds(area.getX(),area.getY(),artWidth,totalHeight);
    if(section=="Global" && contents.art.isValid() && headerButtons.size()==5) {
       // Original uploaded header art uses untrimmed 2048x682 source coordinates.
       const auto scaled=[sx,sy](int x,int y,int w,int h) {
          return juce::Rectangle<int>(juce::roundToInt(x*sx),juce::roundToInt(y*sy),
                                      juce::jmax(18,juce::roundToInt(w*sx)),
                                      juce::jmax(18,juce::roundToInt(h*sy)));
       };
       headerButtons[0]->setBounds(scaled(966,423,78,62));
       headerButtons[1]->setBounds(scaled(1046,423,78,62));
       headerButtons[2]->setBounds(scaled(1177,424,52,62));
       headerButtons[3]->setBounds(scaled(1463,424,52,62));
       headerButtons[4]->setBounds(scaled(1531,424,54,62));
       if(presetLabel)presetLabel->setBounds(scaled(1238,430,220,51));
    }

    int extra=0;
    for(int i=0;i<(int)controls.size();++i) {
        auto p=approvedPosition(ids[(size_t)i]);
        const bool embedded=artValid&&p.x>=0&&p.y>=0;
        int x,y,w,h;
        if(embedded) {
            const bool isToggle=dynamic_cast<juce::ToggleButton*>(controls[(size_t)i].get())!=nullptr;
            const bool isCombo=dynamic_cast<juce::ComboBox*>(controls[(size_t)i].get())!=nullptr;
            w=juce::jlimit(36,112,juce::roundToInt((isToggle?87.f:(isCombo?110.f:128.f))*sx));
            h=juce::jlimit(32,115,juce::roundToInt((isToggle?55.f:(isCombo?48.f:130.f))*sy));
            x=juce::roundToInt(p.x*sx)-w/2;
            y=juce::roundToInt(p.y*sy)-h/2;
            x=juce::jlimit(0,juce::jmax(0,artWidth-w),x);
            y=juce::jlimit(0,juce::jmax(0,artHeight-h),y);
            labels[(size_t)i]->setBounds(x,y-18,w,17);
            labels[(size_t)i]->setVisible(false);
        } else {
            x=(extra%cols)*cellW+2;
            y=artHeight+20+(extra/cols)*cellH;
            w=cellW-6;h=76;
            labels[(size_t)i]->setBounds(x,y,w,22);
            labels[(size_t)i]->setVisible(true);
            y+=22;
            ++extra;
        }
        auto* control=controls[(size_t)i].get();
        control->setBounds(x,y,w,h);
        control->setName(labels[(size_t)i]->getText());
    }
}
int SteamMasterEditor::Page::desiredHeight(int width) const {
    const int artW=juce::jmax(500,width-35);
    const auto& art=contents.art;
    const int artH=art.isValid()
      ?juce::jmax(140,juce::roundToInt(artW*(double)art.getHeight()/art.getWidth()))
      :260;
    int missing=0;
    for(const auto& id:ids) if(approvedPosition(id).x<0) ++missing;
    const int cols=juce::jmax(1,artW/140);
    return artH+62+(missing?((missing+cols-1)/cols)*105+28:0);
}
SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
:AudioProcessorEditor(&p),processor(p) {
    setOpaque(true);
    theme=std::make_unique<SteamLook>();
    setLookAndFeel(theme.get());
    addAndMakeVisible(rackViewport);
    rackViewport.setViewedComponent(&rackContent,false);
    rackViewport.setScrollBarsShown(true,false);
    for(const auto* section:{"Global","Ocean","Furnace","Parametric","Pultec","Shadow",
                              "Exciter","Bax","Tape","Stereo + Limiter"}) {
        auto page=std::make_unique<Page>(processor,processor.state,section);
        rackContent.addAndMakeVisible(*page);
        modules.push_back(std::move(page));
    }
    setResizable(true,true);
    setResizeLimits(760,520,1850,1300);
    setSize(1120,780);
}
SteamMasterEditor::~SteamMasterEditor(){
    rackViewport.setViewedComponent(nullptr,false);
    modules.clear();
    setLookAndFeel(nullptr);
}
void SteamMasterEditor::paint(juce::Graphics& g) {
    g.fillAll(deep);
    auto r=getLocalBounds().toFloat().reduced(3.f);
    g.setColour(brass);
    g.drawRoundedRectangle(r,8.f,2.f);
}
void SteamMasterEditor::resized(){
    auto r=getLocalBounds().reduced(9);
    rackViewport.setBounds(r);
    const int width=juce::jmax(540,r.getWidth()-18);
    int y=0;
    for(auto& module:modules){
       const int height=module->desiredHeight(width);
       module->setBounds(0,y,width,height);
       y+=height+8;
    }
    rackContent.setSize(width,y);
}
