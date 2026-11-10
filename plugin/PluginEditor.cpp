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
    std::map<juce::String, std::pair<juce::Image, juce::Image>> headerButtonsArt;
    std::map<int, juce::Image> oversamplingArt;
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
#ifdef STEAMMASTER_HAS_ARTWORK
        // Exact alpha-component bounds audited against the uploaded 1448x1086 sheets.
        // The previous 4x3 grid cut knobs in half; these are twelve true isolated sprites.
        const auto ocean = juce::ImageFileFormat::loadFrom(
            BinaryData::Ocean_Furnace_Knobs_Sprite_Sheet_png,
            (size_t)BinaryData::Ocean_Furnace_Knobs_Sprite_Sheet_pngSize);
        const auto pultec = juce::ImageFileFormat::loadFrom(
            BinaryData::Pultec_EQ_Knobs_and_IN_Button_Sheet_png,
            (size_t)BinaryData::Pultec_EQ_Knobs_and_IN_Button_Sheet_pngSize);
        auto cut = [](const juce::Image& sheet, int x, int y, int w, int h) {
            return sheet.isValid() ? sheet.getClippedImage({x,y,w,h}) : juce::Image();
        };
        if (ocean.isValid()) {
            const juce::Image oBig=cut(ocean,100,64,401,434), oLever=cut(ocean,611,123,294,303),
                              oSmall=cut(ocean,1044,126,303,332), fBig=cut(ocean,97,573,405,437),
                              fLever=cut(ocean,610,633,295,307), fSmall=cut(ocean,1041,634,304,333);
            knobs["ocean_threshold_db"]=oBig; knobs["ocean_ratio"]=oLever;
            knobs["ocean_attack_ms"]=oSmall; knobs["ocean_recovery"]=oLever;
            knobs["ocean_sc_hpf"]=oLever; knobs["ocean_mix"]=oSmall;
            knobs["furnace_threshold_db"]=fBig; knobs["furnace_ratio"]=fLever;
            knobs["furnace_attack"]=fLever; knobs["furnace_release"]=fLever;
            knobs["furnace_sc_hpf"]=fLever; knobs["furnace_mix"]=fSmall;
        }
        if (pultec.isValid()) {
            knobs["pultec_low_freq"]=cut(pultec,132,74,301,303);
            knobs["pultec_high_boost_freq"]=cut(pultec,572,74,301,303);
            knobs["pultec_high_atten_freq"]=cut(pultec,1046,126,225,228);
            knobs["pultec_low_boost"]=cut(pultec,91,447,253,259);
            knobs["pultec_low_atten"]=cut(pultec,427,447,253,259);
            knobs["pultec_high_boost"]=cut(pultec,764,447,253,259);
            knobs["pultec_high_atten"]=cut(pultec,1102,447,253,259);
            knobs["pultec_bandwidth"]=cut(pultec,68,795,194,199);
            knobs["pultec_output_db"]=cut(pultec,316,795,194,199);
            knobs["pultec_mix"]=cut(pultec,565,795,195,200);
            switches["pultec_in"] = {cut(pultec,831,807,258,190),
                                      cut(pultec,1131,807,260,191)};
        }
#endif
#ifdef STEAMMASTER_HAS_ARTWORK
        const auto buttonsSheet=juce::ImageFileFormat::loadFrom(
            BinaryData::steampunk_brass_ui_button_sprite_sheet_png,
            (size_t)BinaryData::steampunk_brass_ui_button_sprite_sheet_pngSize);
        const auto inSheet=juce::ImageFileFormat::loadFrom(
            BinaryData::steampunk_global_in_toggle_set_png,
            (size_t)BinaryData::steampunk_global_in_toggle_set_pngSize);
        if (buttonsSheet.isValid()) {
            auto cutButton=[&](int x,int y,int w,int h) {
                return buttonsSheet.getClippedImage({x,y,w,h});
            };
            headerButtonsArt["slotA"]={cutButton(97,453,262,156),cutButton(411,453,262,156)};
            headerButtonsArt["slotB"]={cutButton(773,453,262,156),cutButton(1089,453,262,156)};
            headerButtonsArt["previous"]={cutButton(126,648,242,184),cutButton(424,648,238,184)};
            headerButtonsArt["next"]={cutButton(786,648,240,184),cutButton(1084,648,239,183)};
            headerButtonsArt["save"]={cutButton(202,871,430,184),cutButton(810,871,430,185)};
            oversamplingArt[0]=cutButton(401,47,275,165);
            oversamplingArt[1]=cutButton(1081,47,277,165);
            oversamplingArt[2]=cutButton(401,247,278,168);
            oversamplingArt[3]=cutButton(1083,247,280,168);
        }
        if(inSheet.isValid()) {
            switches["global_in"]={
                inSheet.getClippedImage({66,152,386,763}),
                inSheet.getClippedImage({531,152,385,763})};
        }
#endif
        setColour(juce::Slider::textBoxTextColourId, ivory);
        setColour(juce::Slider::textBoxBackgroundColourId, deep);
        setColour(juce::Slider::textBoxOutlineColourId, brass.withAlpha(0.45f));
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ComboBox::textColourId, ivory);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, ivory);
        setColour(juce::ToggleButton::textColourId, ivory);
    }
    void drawComboBox(juce::Graphics&, int, int, bool,
                      int, int, int, int, juce::ComboBox&) override {
        // The faceplate provides the switch housing. No JUCE rectangle or arrow.
    }
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool over, bool down) override {
#ifdef STEAMMASTER_HAS_ARTWORK
        auto it=headerButtonsArt.find(button.getComponentID());
        if(it!=headerButtonsArt.end()) {
            const bool selected=button.getToggleState()||down||over;
            const auto& picture=selected?it->second.first:it->second.second;
            if(picture.isValid()) {
                g.drawImageWithin(picture,0,0,button.getWidth(),button.getHeight(),
                    juce::RectanglePlacement::stretchToFit,false);
                return;
            }
        }
#endif
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool over, bool down) override {
        if(headerButtonsArt.count(button.getComponentID())!=0)return;
        juce::LookAndFeel_V4::drawButtonText(g,button,over,down);
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
    // Fallback bypass LED, not a gold/black rectangular JUCE widget.
    const bool on=b.getToggleState();
    const float radius=juce::jmax(2.f,juce::jmin(b.getWidth(),b.getHeight())*0.14f);
    auto c=b.getLocalBounds().toFloat().getCentre();
    g.setColour(on ? juce::Colour(0xffffd78b) : juce::Colour(0xff536064));
    g.fillEllipse(c.x-radius,c.y-radius,radius*2.f,radius*2.f);
    g.setColour(juce::Colour(0xff28170b));
    g.drawEllipse(c.x-radius,c.y-radius,radius*2.f,radius*2.f,1.f);
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


// Audited native 1145-pixel reference coordinate map. Each module scales
// uniformly with the faceplate; never multiply old screenshots by guesses.
juce::Point<float> approvedPosition(const juce::String& id) {
    struct Placement { const char* id; float x,y; };
    static constexpr Placement positions[] = {
        {"global_in", 82.f, 414.f},
        {"global_input_db", 145.f, 422.f},
        {"global_output_db", 268.f, 422.f},
        {"global_oversampling", 476.f, 465.f},
        {"ocean_in", 93.f, 95.f},
        {"ocean_threshold_db", 144.f, 78.f},
        {"ocean_ratio", 212.f, 78.f},
        {"ocean_attack_ms", 281.f, 78.f},
        {"ocean_recovery", 351.f, 78.f},
        {"ocean_sc_hpf", 418.f, 78.f},
        {"ocean_mix", 476.f, 78.f},
        {"furnace_in", 669.f, 95.f},
        {"furnace_threshold_db", 715.f, 78.f},
        {"furnace_ratio", 783.f, 78.f},
        {"furnace_attack", 851.f, 78.f},
        {"furnace_release", 918.f, 78.f},
        {"furnace_sc_hpf", 986.f, 78.f},
        {"furnace_mix", 1053.f, 78.f},
        {"parametric_in", 132.f, 102.f},
        {"parametric_mix", 1084.f, 71.f},
        {"parametric_output_db", 1014.f, 78.f},
        {"pultec_in", 109.f, 91.f},
        {"pultec_low_freq", 245.f, 76.f},
        {"pultec_low_boost", 328.f, 76.f},
        {"pultec_low_atten", 413.f, 76.f},
        {"pultec_high_boost_freq", 496.f, 76.f},
        {"pultec_high_boost", 578.f, 76.f},
        {"pultec_high_atten_freq", 664.f, 76.f},
        {"pultec_high_atten", 746.f, 76.f},
        {"pultec_bandwidth", 827.f, 76.f},
        {"pultec_output_db", 910.f, 76.f},
        {"pultec_mix", 990.f, 76.f},
        {"shadow_in", 1028.f, 123.f},
        {"shadow_mix", 949.f, 125.f},
        {"shadow_optical_in", 158.f, 72.f},
        {"shadow_optical_threshold", 229.f, 83.f},
        {"shadow_optical_makeup", 313.f, 83.f},
        {"shadow_discrete_in", 588.f, 72.f},
        {"shadow_discrete_threshold", 648.f, 83.f},
        {"shadow_discrete_gain", 706.f, 83.f},
        {"shadow_discrete_ratio", 778.f, 84.f},
        {"shadow_discrete_attack", 845.f, 85.f},
        {"shadow_discrete_recovery", 913.f, 85.f},
        {"shadow_sc_hpf", 608.f, 126.f},
        {"shadow_output_db", 875.f, 126.f},
        {"exciter_in", 167.f, 54.f},
        {"exciter_mix", 1050.f, 50.f},
        {"exciter_mode", 260.f, 54.f},
        {"exciter_low_drive", 351.f, 50.f},
        {"exciter_low_freq", 419.f, 50.f},
        {"exciter_high_drive", 716.f, 50.f},
        {"exciter_high_freq", 787.f, 50.f},
        {"exciter_crossover", 864.f, 50.f},
        {"exciter_output_db", 941.f, 50.f},
        {"bax_mid_in", 210.f, 79.f},
        {"bax_mid_hpf", 361.f, 89.f},
        {"bax_mid_low_freq", 470.f, 89.f},
        {"bax_mid_low_gain", 575.f, 89.f},
        {"bax_mid_high_freq", 778.f, 89.f},
        {"bax_mid_high_gain", 677.f, 89.f},
        {"bax_mid_lpf", 879.f, 89.f},
        {"bax_mid_output_db", 978.f, 89.f},
        {"bax_side_in", 210.f, 159.f},
        {"bax_side_hpf", 361.f, 161.f},
        {"bax_side_low_freq", 470.f, 161.f},
        {"bax_side_low_gain", 575.f, 161.f},
        {"bax_side_high_freq", 778.f, 161.f},
        {"bax_side_high_gain", 677.f, 161.f},
        {"bax_side_lpf", 879.f, 161.f},
        {"bax_side_output_db", 978.f, 161.f},
        {"stereo_routing", 75.f, 78.f},
        {"stereo_link", 75.f, 159.f},
        {"tape_in", 360.f, 56.f},
        {"tape_mix", 779.f, 73.f},
        {"tape_type", 405.f, 73.f},
        {"tape_speed", 484.f, 73.f},
        {"tape_drive_db", 594.f, 73.f},
        {"tape_bias", 655.f, 73.f},
        {"tape_hf_trim_db", 715.f, 73.f},
        {"stereo_mono_maker_hz", 113.f, 90.f},
        {"stereo_width_percent", 197.f, 90.f},
        {"stereo_width_freq", 279.f, 90.f},
        {"limiter_in", 538.f, 98.f},
        {"limiter_mode", 678.f, 155.f},
        {"limiter_threshold_db", 599.f, 78.f},
        {"limiter_ceiling_dbtp", 678.f, 78.f},
        {"limiter_release_ms", 759.f, 78.f},
        {"limiter_auto_release", 912.f, 168.f},
        {"limiter_lookahead_ms", 840.f, 78.f},
        {"limiter_stereo_link", 918.f, 78.f},
        {"limiter_true_peak", 829.f, 168.f},
        {"parametric_band_1_gain", 198.f, 72.f},
        {"parametric_band_1_freq", 261.f, 72.f},
        {"parametric_band_1_q", 319.f, 72.f},
        {"parametric_band_1_type", 261.f, 113.f},
        {"parametric_band_2_gain", 397.f, 72.f},
        {"parametric_band_2_freq", 461.f, 72.f},
        {"parametric_band_2_q", 519.f, 72.f},
        {"parametric_band_2_type", 461.f, 113.f},
        {"parametric_band_3_gain", 602.f, 72.f},
        {"parametric_band_3_freq", 664.f, 72.f},
        {"parametric_band_3_q", 723.f, 72.f},
        {"parametric_band_3_type", 664.f, 113.f},
        {"parametric_band_4_gain", 804.f, 72.f},
        {"parametric_band_4_freq", 869.f, 72.f},
        {"parametric_band_4_q", 927.f, 72.f},
        {"parametric_band_4_type", 869.f, 113.f},
    };
    for (const auto& p:positions) if(id==p.id) return {p.x,p.y};
    return {-1.f,-1.f};
}

SteamMasterEditor::Page::Page(juce::AudioProcessor& processor,juce::AudioProcessorValueTreeState& state,
                               const juce::String& s):source(static_cast<SteamMasterProcessor&>(processor)),section(s) {
    // Decode the approved source art once, on the message thread, never in paint/audio callbacks.
#ifdef STEAMMASTER_HAS_ARTWORK
    const char* asset = nullptr;
    int assetSize = 0;
    if (section == "Global") { asset=BinaryData::abyssal_steampunk_mastering_faceplate_png; assetSize=BinaryData::abyssal_steampunk_mastering_faceplate_pngSize; }
    else if(section == "Ocean + Furnace") { asset=BinaryData::Ocean_Furnace_Clean_Faceplate_png; assetSize=BinaryData::Ocean_Furnace_Clean_Faceplate_pngSize; }
    else if(section == "Parametric") { asset=BinaryData::Parametric_EQ_Clean_Faceplate_png; assetSize=BinaryData::Parametric_EQ_Clean_Faceplate_pngSize; }
    else if(section == "Pultec") { asset=BinaryData::Pultec_EQ_Faceplate_png; assetSize=BinaryData::Pultec_EQ_Faceplate_pngSize; }
    else if(section == "Shadow") { asset=BinaryData::Shadow_Clean_Faceplate_EmptySockets_4x_png; assetSize=BinaryData::Shadow_Clean_Faceplate_EmptySockets_4x_pngSize; }
    else if(section == "Exciter") { asset=BinaryData::Exciter_Empty_Socket_Faceplate_4x_png; assetSize=BinaryData::Exciter_Empty_Socket_Faceplate_4x_pngSize; }
    else if(section == "Bax") { asset=BinaryData::Baxandall_Rack_Faceplate_Cropped_png; assetSize=BinaryData::Baxandall_Rack_Faceplate_Cropped_pngSize; }
    else if(section == "Tape") { asset=BinaryData::TapeMachine_Faceplate_Clean_4x_png; assetSize=BinaryData::TapeMachine_Faceplate_Clean_4x_pngSize; }
    else if(section == "Stereo + Limiter") { asset=BinaryData::StereoImage_Limiter_Clean_Redesigned_png; assetSize=BinaryData::StereoImage_Limiter_Clean_Redesigned_pngSize; }
    if (asset != nullptr && assetSize > 0) {
        contents.art = juce::ImageFileFormat::loadFrom(asset, (size_t)assetSize);
        if (contents.art.isValid()) {
            // Crop native artwork rather than stretching large empty PNG margins.
            if (section == "Ocean + Furnace")
                contents.art=contents.art.getClippedImage({0,224,2172,276});
            else if (section == "Pultec")
                contents.art=contents.art.getClippedImage({0,252,2172,232});
            else if (section == "Stereo + Limiter")
                contents.art=contents.art.getClippedImage({0,142,2172,406});
        }
    }
    contents.section=section;
    contents.processor=&source;
    auto loadImage=[](const void* p,size_t bytes) {
        return juce::ImageFileFormat::loadFrom(p,bytes);
    };
    if(section=="Global") {
        contents.headerMeterAtlas=loadImage(
            BinaryData::nautical_steampunk_meter_and_display_sprite_sheet_png,
            (size_t)BinaryData::nautical_steampunk_meter_and_display_sprite_sheet_pngSize);
    }
    if(section=="Tape") {
        contents.leftReel=loadImage(BinaryData::Reel_Left_FrontOn_png,
                                   (size_t)BinaryData::Reel_Left_FrontOn_pngSize);
        contents.rightReel=loadImage(BinaryData::Reel_Right_FrontOn_png,
                                    (size_t)BinaryData::Reel_Right_FrontOn_pngSize);
    }
    if(section=="Shadow") {
        contents.shadowNeedle=loadImage(BinaryData::GainReduction_Needle_png,
                                        (size_t)BinaryData::GainReduction_Needle_pngSize);
    }
    if(section=="Exciter") {
        contents.exciterNeedle=loadImage(BinaryData::VU_Animated_Needle_Brass_png,
                                         (size_t)BinaryData::VU_Animated_Needle_Brass_pngSize);
    }
    if(section=="Stereo + Limiter") {
        contents.correlationNeedle=loadImage(BinaryData::Correlation_Needle_Separate_png,
                                              (size_t)BinaryData::Correlation_Needle_Separate_pngSize);
        contents.limiterNeedle=loadImage(BinaryData::Gain_Reduction_Needle_Separate_png,
                                          (size_t)BinaryData::Gain_Reduction_Needle_Separate_pngSize);
    }
#endif
    addAndMakeVisible(contents);
    for(auto* p:processor.getParameters()) {
        auto* withID=dynamic_cast<juce::AudioProcessorParameterWithID*>(p);
        if(!withID)continue;
        const auto fullName=withID->getName(200);
        const bool groupCompressors=section=="Ocean + Furnace" &&
            (fullName.startsWith("Ocean / ") || fullName.startsWith("Furnace / "));
        const bool groupBax=section=="Bax" &&
            (fullName.startsWith("Bax Mid / ") || fullName.startsWith("Bax Side / ") ||
             fullName=="Stereo / Routing" || fullName=="Stereo / Link");
        const bool groupStereo=section=="Stereo + Limiter" &&
            (fullName.startsWith("Limiter / ") ||
             (fullName.startsWith("Stereo / ") &&
              fullName!="Stereo / Routing" && fullName!="Stereo / Link"));
        if(!groupCompressors && !groupBax && !groupStereo && !fullName.startsWith(section+" / "))continue;
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
                slider->setPopupDisplayEnabled(true, true, nullptr);
                slider->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
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
                slider->setPopupDisplayEnabled(true, true, nullptr);
            slider->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
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
            button->setComponentID(name);
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
        headerButtons[0]->setToggleState(true,juce::dontSendNotification);
        presetLabel=std::make_unique<juce::Label>();
        presetLabel->setJustificationType(juce::Justification::centred);
        presetLabel->setFont(juce::Font(juce::FontOptions(17.f,juce::Font::italic)));
        presetLabel->setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);
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
    if(headerButtons.size()>=2) {
        headerButtons[0]->setToggleState(!toB,juce::dontSendNotification);
        headerButtons[1]->setToggleState(toB,juce::dontSendNotification);
    }
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

void SteamMasterEditor::Page::timerCallback(){
    if (section=="Tape") contents.reelAngle += 0.025f;
    contents.repaint();
    if (section=="Global") repaint(juce::Rectangle<int>(getWidth()-150,0,150,20));
}
void SteamMasterEditor::Page::ArtCanvas::drawMeterNeedle(
    juce::Graphics& g,const juce::Image& needle,float cx,float cy,
    float radius,float fraction) {
    if (!needle.isValid()) return;
    const float scale=radius*0.72f/(float)juce::jmax(needle.getWidth(),needle.getHeight());
    const float angle=juce::jmap(juce::jlimit(0.f,1.f,fraction),-0.85f,0.85f);
    const float sw=(float)needle.getWidth(), sh=(float)needle.getHeight();
    auto transform=juce::AffineTransform::rotation(angle,sw*0.5f,sh*0.82f)
        .scaled(scale).translated(cx-sw*scale*0.5f,cy-sh*scale*0.82f);
    g.drawImageTransformed(needle,transform,false);
}
void SteamMasterEditor::Page::ArtCanvas::paintHeaderInstruments(juce::Graphics& g) {
    if (!processor || !art.isValid()) return;
    // Original header faceplate supplies the brass bezels. These are live,
    // lightweight readouts and meters inside the existing empty instruments.
    const float sx=(float)getWidth()/1145.f;
    const float sy=(float)artHeight/((float)art.getHeight()*1145.f/art.getWidth());
    juce::Graphics::ScopedSaveState saved(g);
    g.addTransform(juce::AffineTransform::scale(sx,sy));
    const auto gold=juce::Colour(0xffffd995);
    const auto cream=juce::Colour(0xfff2e5c6);
    const auto dim=juce::Colour(0xffc29d65);
    auto text=[&](const juce::String& t,float x,float y,float w,float h,float size,
                  juce::Colour colour=juce::Colour(0xfff2e5c6)) {
        g.setColour(colour);
        g.setFont(juce::Font(juce::FontOptions(size)));
        g.drawFittedText(t,juce::Rectangle<int>((int)x,(int)y,(int)w,(int)h),
                         juce::Justification::centred,1);
    };
    auto read=[&](const char* id)->float {
        if(auto* value=processor->state.getRawParameterValue(id))
            return value->load(std::memory_order_relaxed);
        return 0.f;
    };
    auto meter=[&](float x,float y,float w,float value,float minimum,float maximum) {
        const float fraction=juce::jlimit(0.f,1.f,(value-minimum)/(maximum-minimum));
        g.setColour(juce::Colour(0xff142a2c));
        g.fillRoundedRectangle(x,y,w,12.f,3.f);
        g.setColour(juce::Colour(0xff7c592e));
        g.drawRoundedRectangle(x,y,w,12.f,3.f,1.f);
        g.setColour(juce::Colour(0xffd6a454));
        g.fillRoundedRectangle(x+2.f,y+2.f,(w-4.f)*fraction,8.f,2.f);
    };
    // Two large brass-rimmed display apertures in the supplied faceplate.
    const float input=read("global_input_db");
    const float output=processor->meterOutputDb.load(std::memory_order_relaxed);
    text("INPUT TRIM",405,332,243,20,15,dim);
    text(juce::String(input,1)+" dB",418,353,215,35,27,gold);
    meter(438,389,175,input,-24.f,24.f);
    text("OUTPUT LEVEL",713,332,232,20,15,dim);
    text(juce::String(output,1)+" dBFS",727,353,201,35,27,gold);
    meter(738,389,180,output,-60.f,6.f);
    // Lower three instrument windows: mode and two compressor GR meters.
    text("OVERSAMPLING",404,441,151,17,12,dim);
    text(juce::String(juce::roundToInt(read("global_oversampling"))==0?"OFF":
          juce::roundToInt(read("global_oversampling"))==1?"2x":
          juce::roundToInt(read("global_oversampling"))==2?"4x":"8x"),
          411,462,138,28,23,gold);
    const float ocean=processor->meterOceanGr.load(std::memory_order_relaxed);
    const float furnace=processor->meterFurnaceGr.load(std::memory_order_relaxed);
    text("OCEAN GR",578,441,170,17,12,dim);
    text(juce::String(std::abs(ocean),1)+" dB",591,461,142,28,22,cream);
    text("FURNACE GR",788,441,166,17,12,dim);
    text(juce::String(std::abs(furnace),1)+" dB",798,461,144,28,22,cream);
    // Exact control identifiers remain in the JUCE overlay, not painted as
    // decorative, nonfunctional widgets.
    text("INPUT",119,372,75,20,14,cream);
    text("OUTPUT",212,372,75,20,14,cream);
    text("IN",51,385,36,20,14,gold);
}
void SteamMasterEditor::Page::ArtCanvas::paintMovingParts(juce::Graphics& g) {
    if (!processor || !art.isValid()) return;
    const float sx=(float)getWidth()/art.getWidth();
    const float sy=(float)artHeight/art.getHeight();
    const auto read=[](const std::atomic<float>& v){
        return v.load(std::memory_order_relaxed);
    };
    auto drawReel=[&](const juce::Image& sprite,float x,float y,float diameter) {
        if(!sprite.isValid())return;
        const float scale=diameter/(float)juce::jmax(sprite.getWidth(),sprite.getHeight());
        const float iw=(float)sprite.getWidth(), ih=(float)sprite.getHeight();
        auto t=juce::AffineTransform::rotation(reelAngle,iw/2.f,ih/2.f)
               .scaled(scale).translated(x-diameter/2.f,y-diameter/2.f);
        g.drawImageTransformed(sprite,t,false);
    };
    if(section=="Tape") {
        drawReel(leftReel,174.f*sx,70.f*sy,136.f*sx);
        drawReel(rightReel,969.f*sx,70.f*sy,136.f*sx);
    } else if(section=="Shadow") {
        const float reduction=juce::jmax(0.f,-read(processor->meterShadowOpticalGr))
                            +juce::jmax(0.f,-read(processor->meterShadowDiscreteGr));
        drawMeterNeedle(g,shadowNeedle,570.f*sx,109.f*sy,78.f*sx,
                        juce::jlimit(0.f,1.f,reduction/20.f));
    } else if(section=="Exciter") {
        drawMeterNeedle(g,exciterNeedle,565.f*sx,76.f*sy,70.f*sx,
                        juce::jlimit(0.f,1.f,(read(processor->meterOutputDb)+60.f)/60.f));
    } else if(section=="Stereo + Limiter") {
        drawMeterNeedle(g,correlationNeedle,363.f*sx,133.f*sy,75.f*sx,
                        (read(processor->meterCorrelation)+1.f)*0.5f);
        drawMeterNeedle(g,limiterNeedle,1040.f*sx,133.f*sy,75.f*sx,
                        juce::jlimit(0.f,1.f,read(processor->meterLimiterGr)/20.f));
    }
}
void SteamMasterEditor::Page::paint(juce::Graphics& g) {
    // No independent title bar, border or meter boxes. Artwork is one rack.
    g.fillAll(deep);
}
void SteamMasterEditor::Page::resized() {
    auto area=getLocalBounds();
    const bool valid=contents.art.isValid();
    const int sourceW=valid?contents.art.getWidth():1145;
    const int sourceH=valid?contents.art.getHeight():140;
    const int width=area.getWidth();
    const int height=juce::roundToInt((double)width*sourceH/sourceW);
    contents.artHeight=height;
    contents.setBounds(0,0,width,height);
    const float sx=(float)width/1145.f;
    const float sy=(float)height/((float)sourceH*1145.f/sourceW);
    if(section=="Global" && headerButtons.size()==5) {
        auto button=[&](int index,int x,int y,int w,int h) {
            headerButtons[(size_t)index]->setBounds(juce::roundToInt(x*sx),
                juce::roundToInt(y*sy),juce::roundToInt(w*sx),juce::roundToInt(h*sy));
        };
        button(0,655,420,45,30);button(1,707,420,45,30);
        button(2,794,420,35,30);button(3,997,420,35,30);
        button(4,1037,420,45,30);
        if(presetLabel) presetLabel->setBounds(juce::roundToInt(834*sx),
            juce::roundToInt(421*sy),juce::roundToInt(155*sx),juce::roundToInt(28*sy));
    }
    for(size_t i=0;i<controls.size();++i) {
        const auto id=ids[i];
        const auto p=approvedPosition(id);
        auto* control=controls[i].get();
        if(p.x<0.f || p.y<0.f) {
            // Never create a second row of controls outside the faceplate.
            control->setVisible(false);
            labels[i]->setVisible(false);
            continue;
        }
        const bool toggle=dynamic_cast<juce::ToggleButton*>(control)!=nullptr;
        const bool combo=dynamic_cast<juce::ComboBox*>(control)!=nullptr;
        const bool small=toggle || combo || id.endsWith("_type") ||
                         id=="limiter_mode" || id=="exciter_mode";
        const float diameter=small?(toggle?25.f:(combo?53.f:30.f)):38.f;
        const int w=juce::jmax(14,juce::roundToInt(diameter*sx));
        const int h=juce::jmax(14,juce::roundToInt((combo?24.f:diameter)*sy));
        const int x=juce::roundToInt(p.x*sx)-w/2;
        const int y=juce::roundToInt(p.y*sy)-h/2;
        control->setBounds(x,y,w,h);
        control->setVisible(true);
        labels[i]->setVisible(false);
        control->setName(labels[i]->getText());
    }
}
int SteamMasterEditor::Page::desiredHeight(int width) const {
    const auto& image=contents.art;
    if(!image.isValid()) return 140;
    return juce::jmax(1,juce::roundToInt((double)width*image.getHeight()/image.getWidth()));
}

SteamMasterEditor::SteamMasterEditor(SteamMasterProcessor& p)
:AudioProcessorEditor(&p),processor(p) {
    setOpaque(true);
    theme=std::make_unique<SteamLook>();
    setLookAndFeel(theme.get());
    addAndMakeVisible(rackViewport);
    rackViewport.setViewedComponent(&rackContent,false);
    rackViewport.setScrollBarsShown(true,false);
    for(const auto* section:{"Global","Ocean + Furnace","Parametric","Pultec","Shadow",
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
    // Rack edges are supplied by the artwork; avoid double borders.
}
void SteamMasterEditor::resized(){
    auto r=getLocalBounds();
    rackViewport.setBounds(r);
    const int width=juce::jmax(540,r.getWidth());
    int y=0;
    for(auto& module:modules){
       const int height=module->desiredHeight(width);
       module->setBounds(0,y,width,height);
       y+=height+8;
    }
    rackContent.setSize(width,y);
}
