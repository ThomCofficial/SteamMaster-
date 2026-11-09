#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout SteamMasterProcessor::makeLayout(){
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"global_in",1},"Global / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"global_input_db",1},"Global / Input dB",juce::NormalisableRange<float>(-24.0f,24.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"global_output_db",1},"Global / Output dB",juce::NormalisableRange<float>(-24.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"global_oversampling",1},"Global / Oversampling",juce::StringArray{"Off","2x","4x","8x"},0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"ocean_in",1},"Ocean / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ocean_mix",1},"Ocean / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ocean_threshold_db",1},"Ocean / Threshold dB",juce::NormalisableRange<float>(-40.0f,10.0f,0.001f),-18.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"ocean_ratio",1},"Ocean / Ratio",juce::StringArray{"1.2","1.5","2","3","4","6"},2));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ocean_attack_ms",1},"Ocean / Attack ms",juce::NormalisableRange<float>(25.0f,70.0f,0.001f),40.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"ocean_recovery",1},"Ocean / Recovery",juce::StringArray{"0.2","0.4","0.6","4","8"},1));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"ocean_sc_hpf",1},"Ocean / SC HPF",juce::StringArray{"Off","40","80","160","320","500"},0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"furnace_in",1},"Furnace / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"furnace_mix",1},"Furnace / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"furnace_threshold_db",1},"Furnace / Threshold dB",juce::NormalisableRange<float>(-15.0f,15.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"furnace_ratio",1},"Furnace / Ratio",juce::StringArray{"2","4","10"},1));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"furnace_attack",1},"Furnace / Attack",juce::StringArray{"0.1","0.3","1","3","10","30"},4));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"furnace_release",1},"Furnace / Release",juce::StringArray{"0.1","0.3","0.6","1.2","Auto"},1));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"furnace_sc_hpf",1},"Furnace / SC HPF",juce::StringArray{"Off","40","80","160","320","500"},0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"parametric_in",1},"Parametric / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_mix",1},"Parametric / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_output_db",1},"Parametric / Output dB",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"parametric_band_1_type",1},"Parametric / Band 1 Type",juce::StringArray{"Peak","Low Shelf","High Shelf"},1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_1_freq",1},"Parametric / Band 1 Freq",juce::NormalisableRange<float>(20.0f,20000.0f,0.001f),80.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_1_q",1},"Parametric / Band 1 Q",juce::NormalisableRange<float>(0.2f,8.0f,0.001f),0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_1_gain",1},"Parametric / Band 1 Gain",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"parametric_band_2_type",1},"Parametric / Band 2 Type",juce::StringArray{"Peak","Low Shelf","High Shelf"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_2_freq",1},"Parametric / Band 2 Freq",juce::NormalisableRange<float>(20.0f,20000.0f,0.001f),400.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_2_q",1},"Parametric / Band 2 Q",juce::NormalisableRange<float>(0.2f,8.0f,0.001f),0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_2_gain",1},"Parametric / Band 2 Gain",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"parametric_band_3_type",1},"Parametric / Band 3 Type",juce::StringArray{"Peak","Low Shelf","High Shelf"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_3_freq",1},"Parametric / Band 3 Freq",juce::NormalisableRange<float>(20.0f,20000.0f,0.001f),2000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_3_q",1},"Parametric / Band 3 Q",juce::NormalisableRange<float>(0.2f,8.0f,0.001f),0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_3_gain",1},"Parametric / Band 3 Gain",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"parametric_band_4_type",1},"Parametric / Band 4 Type",juce::StringArray{"Peak","Low Shelf","High Shelf"},2));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_4_freq",1},"Parametric / Band 4 Freq",juce::NormalisableRange<float>(20.0f,20000.0f,0.001f),10000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_4_q",1},"Parametric / Band 4 Q",juce::NormalisableRange<float>(0.2f,8.0f,0.001f),0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"parametric_band_4_gain",1},"Parametric / Band 4 Gain",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"pultec_in",1},"Pultec / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_mix",1},"Pultec / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"pultec_low_freq",1},"Pultec / Low Freq",juce::StringArray{"30","60","100"},1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_low_boost",1},"Pultec / Low Boost",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_low_atten",1},"Pultec / Low Atten",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"pultec_high_boost_freq",1},"Pultec / High Boost Freq",juce::StringArray{"8000","12000","16000"},1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_high_boost",1},"Pultec / High Boost",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_bandwidth",1},"Pultec / Bandwidth",juce::NormalisableRange<float>(0.0f,1.0f,0.001f),0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"pultec_high_atten_freq",1},"Pultec / High Atten Freq",juce::StringArray{"5000","10000","20000"},1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_high_atten",1},"Pultec / High Atten",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"pultec_output_db",1},"Pultec / Output dB",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"shadow_in",1},"Shadow / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_mix",1},"Shadow / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"shadow_optical_in",1},"Shadow / Optical IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_optical_threshold",1},"Shadow / Optical Threshold",juce::NormalisableRange<float>(-40.0f,10.0f,0.001f),-18.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_optical_makeup",1},"Shadow / Optical Makeup",juce::NormalisableRange<float>(-12.0f,18.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"shadow_discrete_in",1},"Shadow / Discrete IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_discrete_threshold",1},"Shadow / Discrete Threshold",juce::NormalisableRange<float>(-40.0f,10.0f,0.001f),-12.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_discrete_gain",1},"Shadow / Discrete Gain",juce::NormalisableRange<float>(-12.0f,18.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"shadow_discrete_ratio",1},"Shadow / Discrete Ratio",juce::StringArray{"1.2","2","3","4","6","20"},2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"shadow_discrete_attack",1},"Shadow / Discrete Attack",juce::StringArray{"0.1","0.3","1","3","10","30"},3));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"shadow_discrete_recovery",1},"Shadow / Discrete Recovery",juce::StringArray{"0.1","0.2","0.4","0.6","0.8","1.2","Dual"},2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"shadow_sc_hpf",1},"Shadow / SC HPF",juce::StringArray{"Off","40","80","160","320","500"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"shadow_output_db",1},"Shadow / Output dB",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"exciter_in",1},"Exciter / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_mix",1},"Exciter / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"exciter_mode",1},"Exciter / Mode",juce::StringArray{"Tube","Tape","Transformer"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_low_drive",1},"Exciter / Low Drive",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_low_freq",1},"Exciter / Low Freq",juce::NormalisableRange<float>(20.0f,800.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_high_drive",1},"Exciter / High Drive",juce::NormalisableRange<float>(0.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_high_freq",1},"Exciter / High Freq",juce::NormalisableRange<float>(1000.0f,18000.0f,0.001f),5000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_crossover",1},"Exciter / Crossover",juce::NormalisableRange<float>(100.0f,5000.0f,0.001f),800.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"exciter_output_db",1},"Exciter / Output dB",juce::NormalisableRange<float>(-12.0f,12.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bax_mid_in",1},"Bax Mid / IN",true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_mid_hpf",1},"Bax Mid / HPF",juce::StringArray{"Out","12","18","24","30","36","43","54"},0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_mid_low_freq",1},"Bax Mid / Low Freq",juce::StringArray{"74","84","98","116","131","166","230","361"},3));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_mid_low_gain",1},"Bax Mid / Low Gain",juce::NormalisableRange<float>(-5.0f,5.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_mid_high_freq",1},"Bax Mid / High Freq",juce::StringArray{"1600","1800","2100","2500","3400","4800","7100","18000"},4));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_mid_high_gain",1},"Bax Mid / High Gain",juce::NormalisableRange<float>(-5.0f,5.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_mid_lpf",1},"Bax Mid / LPF",juce::StringArray{"Out","70k","28k","18k","12.6k","11.1k","9k","7.5k"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_mid_output_db",1},"Bax Mid / Output dB",juce::NormalisableRange<float>(-4.0f,4.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bax_side_in",1},"Bax Side / IN",true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_side_hpf",1},"Bax Side / HPF",juce::StringArray{"Out","12","18","24","30","36","43","54"},0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_side_low_freq",1},"Bax Side / Low Freq",juce::StringArray{"74","84","98","116","131","166","230","361"},3));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_side_low_gain",1},"Bax Side / Low Gain",juce::NormalisableRange<float>(-5.0f,5.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_side_high_freq",1},"Bax Side / High Freq",juce::StringArray{"1600","1800","2100","2500","3400","4800","7100","18000"},4));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_side_high_gain",1},"Bax Side / High Gain",juce::NormalisableRange<float>(-5.0f,5.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"bax_side_lpf",1},"Bax Side / LPF",juce::StringArray{"Out","70k","28k","18k","12.6k","11.1k","9k","7.5k"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"bax_side_output_db",1},"Bax Side / Output dB",juce::NormalisableRange<float>(-4.0f,4.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"stereo_routing",1},"Stereo / Routing",juce::StringArray{"Mid Side","Left Right"},0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"stereo_link",1},"Stereo / Link",false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"stereo_mono_maker_hz",1},"Stereo / Mono Maker Hz",juce::NormalisableRange<float>(0.0f,300.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"stereo_width_percent",1},"Stereo / Width Percent",juce::NormalisableRange<float>(0.0f,200.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"stereo_width_freq",1},"Stereo / Width Freq",juce::NormalisableRange<float>(800.0f,5000.0f,0.001f),2000.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"tape_in",1},"Tape / IN",true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tape_mix",1},"Tape / Mix",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"tape_type",1},"Tape / Type",juce::StringArray{"Studio","Vintage","Modern"},0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"tape_speed",1},"Tape / Speed",juce::StringArray{"7.5","15","30"},1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tape_drive_db",1},"Tape / Drive dB",juce::NormalisableRange<float>(-10.0f,10.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tape_bias",1},"Tape / Bias",juce::NormalisableRange<float>(-5.0f,5.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tape_hf_trim_db",1},"Tape / HF Trim dB",juce::NormalisableRange<float>(-6.0f,6.0f,0.001f),0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"limiter_in",1},"Limiter / IN",true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"limiter_mode",1},"Limiter / Mode",juce::StringArray{"Clean","Punch","Modern"},0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"limiter_threshold_db",1},"Limiter / Threshold dB",juce::NormalisableRange<float>(-24.0f,0.0f,0.001f),-6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"limiter_ceiling_dbtp",1},"Limiter / Ceiling dBTP",juce::NormalisableRange<float>(-2.0f,0.0f,0.001f),-1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"limiter_release_ms",1},"Limiter / Release ms",juce::NormalisableRange<float>(10.0f,1000.0f,0.001f),150.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"limiter_auto_release",1},"Limiter / Auto Release",false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"limiter_lookahead_ms",1},"Limiter / Lookahead ms",juce::NormalisableRange<float>(0.1f,5.0f,0.001f),3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"limiter_stereo_link",1},"Limiter / Stereo Link",juce::NormalisableRange<float>(0.0f,100.0f,0.001f),100.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"limiter_true_peak",1},"Limiter / True Peak",true));
    return layout;
}
SteamMasterProcessor::SteamMasterProcessor():juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"PARAMETERS",makeLayout()),rack(std::make_unique<steammaster::MasteringRack>()){
    const auto& params=getParameters();
    jassert(params.size()==int(monitored_.size()));
    for(size_t i=0;i<monitored_.size() && i<size_t(params.size());++i)monitored_[i]=params[int(i)];
}
bool SteamMasterProcessor::isBusesLayoutSupported(const BusesLayout& l) const {return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo() && l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void SteamMasterProcessor::prepareToPlay(double fs,int){rack->prepare(fs);snapshotValid_=false;updateDSP();setLatencySamples(int(rack->latencySamples()));}
void SteamMasterProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
    juce::ScopedNoDenormals noDenormals;
    if(buffer.getNumChannels()<2)return;
    updateDSP();
    auto* left=buffer.getWritePointer(0);auto* right=buffer.getWritePointer(1);
    for(int i=0;i<buffer.getNumSamples();++i){const auto y=rack->process({double(left[i]),double(right[i])});left[i]=float(y.l);right[i]=float(y.r);}
    for(int c=2;c<buffer.getNumChannels();++c)buffer.clear(c,0,buffer.getNumSamples());
}
juce::AudioProcessorEditor* SteamMasterProcessor::createEditor(){return new SteamMasterEditor(*this);}
void SteamMasterProcessor::getStateInformation(juce::MemoryBlock& dest){if(auto xml=state.copyState().createXml())copyXmlToBinary(*xml,dest);}
void SteamMasterProcessor::setStateInformation(const void* data,int bytes){if(auto xml=getXmlFromBinary(data,bytes))if(xml->hasTagName(state.state.getType().toString()))state.replaceState(juce::ValueTree::fromXml(*xml));}
void SteamMasterProcessor::updateDSP() noexcept {
    bool changed=!snapshotValid_;
    for(size_t i=0;i<monitored_.size();++i){
        if(!monitored_[i])continue;
        const float current=monitored_[i]->getValue();
        if(!snapshotValid_ || current!=previous_[i]){changed=true;previous_[i]=current;}
    }
    snapshotValid_=true;
    if(!changed)return;
    using namespace steammaster;
    GlobalParameters g;
    g.in=v("global_in")>.5f;g.inputDb=v("global_input_db");g.outputDb=v("global_output_db");g.oversampling=OversamplingMode(juce::jlimit(0,3,int(v("global_oversampling"))));
    g.monoMakerHz=v("stereo_mono_maker_hz");g.stereoWidth=v("stereo_width_percent")/100.;g.widthFrequencyHz=v("stereo_width_freq");rack->setGlobal(g);
    OceanParameters o;o.common.in=v("ocean_in")>.5f;o.common.mix=v("ocean_mix")/100.;
    o.thresholdDb=v("ocean_threshold_db");o.ratio=OceanRatio(int(v("ocean_ratio")));o.attackMs=v("ocean_attack_ms");o.recovery=OceanRecovery(int(v("ocean_recovery")));o.scHpfHz=std::array<int,6>{0,40,80,160,320,500}[juce::jlimit(0,5,int(v("ocean_sc_hpf")))];
    rack->firstSix.ocean.set(o);
    FurnaceParameters fu;fu.common.in=v("furnace_in")>.5f;fu.common.mix=v("furnace_mix")/100.;
    fu.thresholdDb=v("furnace_threshold_db");fu.ratio=FurnaceRatio(int(v("furnace_ratio")));fu.attack=FurnaceAttack(int(v("furnace_attack")));fu.release=FurnaceRelease(int(v("furnace_release")));fu.scHpfHz=std::array<int,6>{0,40,80,160,320,500}[juce::jlimit(0,5,int(v("furnace_sc_hpf")))];
    rack->firstSix.furnace.set(fu);
    ParametricParameters eq;eq.common.in=v("parametric_in")>.5f;eq.common.mix=v("parametric_mix")/100.;
    eq.outputDb=v("parametric_output_db");
    eq.bands[0].frequencyHz=v("parametric_band_1_freq");eq.bands[0].q=v("parametric_band_1_q");eq.bands[0].gainDb=v("parametric_band_1_gain");eq.bands[0].mode=(int(v("parametric_band_1_type"))==1?TptEQ::Mode::LowShelf:(int(v("parametric_band_1_type"))==2?TptEQ::Mode::HighShelf:TptEQ::Mode::Peak));
    eq.bands[1].frequencyHz=v("parametric_band_2_freq");eq.bands[1].q=v("parametric_band_2_q");eq.bands[1].gainDb=v("parametric_band_2_gain");eq.bands[1].mode=(int(v("parametric_band_2_type"))==1?TptEQ::Mode::LowShelf:(int(v("parametric_band_2_type"))==2?TptEQ::Mode::HighShelf:TptEQ::Mode::Peak));
    eq.bands[2].frequencyHz=v("parametric_band_3_freq");eq.bands[2].q=v("parametric_band_3_q");eq.bands[2].gainDb=v("parametric_band_3_gain");eq.bands[2].mode=(int(v("parametric_band_3_type"))==1?TptEQ::Mode::LowShelf:(int(v("parametric_band_3_type"))==2?TptEQ::Mode::HighShelf:TptEQ::Mode::Peak));
    eq.bands[3].frequencyHz=v("parametric_band_4_freq");eq.bands[3].q=v("parametric_band_4_q");eq.bands[3].gainDb=v("parametric_band_4_gain");eq.bands[3].mode=(int(v("parametric_band_4_type"))==1?TptEQ::Mode::LowShelf:(int(v("parametric_band_4_type"))==2?TptEQ::Mode::HighShelf:TptEQ::Mode::Peak));
    rack->firstSix.parametric.set(eq);
    PultecParameters pu;pu.common.in=v("pultec_in")>.5f;pu.common.mix=v("pultec_mix")/100.;
    pu.lowFrequencyHz=std::array<int,3>{30,60,100}[juce::jlimit(0,2,int(v("pultec_low_freq")))];pu.lowBoost=v("pultec_low_boost");pu.lowAttenuation=v("pultec_low_atten");pu.highBoostFrequencyHz=std::array<int,3>{8000,12000,16000}[juce::jlimit(0,2,int(v("pultec_high_boost_freq")))];pu.highBoost=v("pultec_high_boost");pu.bandwidth=v("pultec_bandwidth");pu.highAttenuationFrequencyHz=std::array<int,3>{5000,10000,20000}[juce::jlimit(0,2,int(v("pultec_high_atten_freq")))];pu.highAttenuation=v("pultec_high_atten");pu.outputDb=v("pultec_output_db");
    rack->firstSix.pultec.set(pu);
    ShadowParameters sh;sh.common.in=v("shadow_in")>.5f;sh.common.mix=v("shadow_mix")/100.;
    sh.opticalIn=v("shadow_optical_in")>.5f;
    sh.opticalThresholdDb=v("shadow_optical_threshold");
    sh.opticalMakeupDb=v("shadow_optical_makeup");
    sh.discreteIn=v("shadow_discrete_in")>.5f;
    sh.discreteThresholdDb=v("shadow_discrete_threshold");
    sh.discreteGainDb=v("shadow_discrete_gain");
    sh.outputDb=v("shadow_output_db");
    sh.discreteRatio=DiscreteRatio(int(v("shadow_discrete_ratio")));sh.discreteAttack=DiscreteAttack(int(v("shadow_discrete_attack")));sh.discreteRecovery=DiscreteRecovery(int(v("shadow_discrete_recovery")));sh.scHpfHz=std::array<int,6>{0,40,80,160,320,500}[juce::jlimit(0,5,int(v("shadow_sc_hpf")))];
    rack->firstSix.shadow.set(sh);
    ExciterParameters ex;ex.common.in=v("exciter_in")>.5f;ex.common.mix=v("exciter_mix")/100.;
    ex.mode=ExciterMode(int(v("exciter_mode")));ex.lowDrive=v("exciter_low_drive");ex.lowFrequencyHz=v("exciter_low_freq");ex.highDrive=v("exciter_high_drive");ex.highFrequencyHz=v("exciter_high_freq");ex.crossoverHz=v("exciter_crossover");ex.outputDb=v("exciter_output_db");
    rack->firstSix.exciter.set(ex);
    TapeParameters ta;ta.common.in=v("tape_in")>.5f;ta.common.mix=v("tape_mix")/100.;
    ta.type=TapeType(int(v("tape_type")));ta.speed=TapeSpeed(int(v("tape_speed")));ta.driveDb=v("tape_drive_db");ta.bias=v("tape_bias");ta.hfTrimDb=v("tape_hf_trim_db");
    rack->tape.set(ta);
    LimiterParameters li;li.in=v("limiter_in")>.5f;
    li.mode=LimiterMode(int(v("limiter_mode")));li.thresholdDb=v("limiter_threshold_db");li.ceilingDbTP=v("limiter_ceiling_dbtp");li.releaseMs=v("limiter_release_ms");li.autoRelease=v("limiter_auto_release")>.5f;li.lookaheadMs=v("limiter_lookahead_ms");li.stereoLink=v("limiter_stereo_link")/100.;li.truePeak=v("limiter_true_peak")>.5f;
    rack->limiter.set(li);
    BaxandallParameters bax;
    bax.rows[0].in=v("bax_mid_in")>.5f;
    bax.rows[0].lowGainDb=v("bax_mid_low_gain");
    bax.rows[0].highGainDb=v("bax_mid_high_gain");
    bax.rows[0].outputDb=v("bax_mid_output_db");
    bax.rows[0].hpf=BaxHPF(int(v("bax_mid_hpf")));
    bax.rows[0].lowFrequency=BaxLowShelf(int(v("bax_mid_low_freq")));
    bax.rows[0].highFrequency=BaxHighShelf(int(v("bax_mid_high_freq")));
    bax.rows[0].lpf=BaxLPF(int(v("bax_mid_lpf")));
    bax.rows[1].in=v("bax_side_in")>.5f;
    bax.rows[1].lowGainDb=v("bax_side_low_gain");
    bax.rows[1].highGainDb=v("bax_side_high_gain");
    bax.rows[1].outputDb=v("bax_side_output_db");
    bax.rows[1].hpf=BaxHPF(int(v("bax_side_hpf")));
    bax.rows[1].lowFrequency=BaxLowShelf(int(v("bax_side_low_freq")));
    bax.rows[1].highFrequency=BaxHighShelf(int(v("bax_side_high_freq")));
    bax.rows[1].lpf=BaxLPF(int(v("bax_side_lpf")));
    bax.routing=BaxRouting(int(v("stereo_routing")));bax.link=v("stereo_link")>.5f;rack->baxandall.set(bax);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SteamMasterProcessor();}