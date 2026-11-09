#include "PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <string>

void checkExciterHostSignalPath() {
    // End-to-end parameter->DSP test: all unrelated modules bypassed, then
    // compare the actual JUCE processor output with Exciter controls at zero.
    const auto render=[](int mode,float lowDrive,float highDrive,
                         float lowFreq,float highFreq) {
        auto p=std::make_unique<SteamMasterProcessor>();
        p->prepareToPlay(48000.,512);
        const auto set=[&](const char* id,float value) {
            auto* param=p->state.getParameter(id);
            if(!param)throw std::runtime_error(std::string("missing parameter: ")+id);
            param->setValueNotifyingHost(param->convertTo0to1(value));
        };
        for(const char* id:{"ocean_in","furnace_in","parametric_in",
                             "pultec_in","shadow_in","bax_mid_in","bax_side_in",
                             "tape_in","limiter_in"})set(id,0.f);
        set("exciter_in",1.f);
        set("exciter_mix",100.f);
        set("exciter_mode",float(mode));
        set("exciter_low_drive",lowDrive);
        set("exciter_high_drive",highDrive);
        set("exciter_low_freq",lowFreq);
        set("exciter_high_freq",highFreq);
        set("exciter_crossover",1100.f);
        set("global_oversampling",2.f);
        juce::AudioBuffer<float> audio(2,512);
        juce::MidiBuffer midi;
        std::vector<float> out;
        out.reserve(8192);
        for(int block=0;block<32;++block) {
            for(int i=0;i<512;++i) {
                const double n=double(block*512+i);
                const float x=float(.22*std::sin(2.*3.141592653589793*155.*n/48000.)
                                   +.18*std::sin(2.*3.141592653589793*1800.*n/48000.)
                                   +.10*std::sin(2.*3.141592653589793*8300.*n/48000.));
                audio.setSample(0,i,x);
                audio.setSample(1,i,x);
            }
            p->processBlock(audio,midi);
            if(block>=16)for(int i=0;i<512;++i) {
                const float v=audio.getSample(0,i);
                if(!std::isfinite(v))throw std::runtime_error("Exciter emitted nonfinite audio");
                out.push_back(v);
            }
        }
        return out;
    };
    const auto difference=[](const std::vector<float>& a,const std::vector<float>& b) {
        if(a.size()!=b.size())throw std::runtime_error("Exciter render length mismatch");
        double sum=0.;
        for(std::size_t i=0;i<a.size();++i) {
            const double d=double(a[i])-double(b[i]);
            sum+=d*d;
        }
        return std::sqrt(sum/double(a.size()));
    };
    const auto neutral=render(0,0.f,0.f,100.f,5000.f);
    for(int mode=0;mode<3;++mode) {
        const auto driven=render(mode,9.f,9.f,250.f,3200.f);
        const auto shifted=render(mode,9.f,9.f,60.f,15000.f);
        const double driveDelta=difference(neutral,driven);
        const double frequencyDelta=difference(driven,shifted);
        std::cout<<"Exciter host mode "<<mode<<" drive RMS delta "<<driveDelta
                 <<", frequency-control RMS delta "<<frequencyDelta<<"\\n";
        if(driveDelta<.003)throw std::runtime_error("Exciter host Drive is inaudible or unmapped");
        if(frequencyDelta<.001)throw std::runtime_error("Exciter host tone controls do not alter response");
    }
}

int main(){
  juce::ScopedJuceInitialiser_GUI juceInitialiser;
  std::mt19937 rng(0x534d);
  for(double fs:{44100.,48000.,96000.}){
    auto p=std::make_unique<SteamMasterProcessor>();
    p->prepareToPlay(fs,512);
    if(p->getLatencySamples()<=0)throw std::runtime_error("latency not reported");
    if(!p->hasEditor())throw std::runtime_error("missing editor");
    {
      std::unique_ptr<juce::AudioProcessorEditor> editor(p->createEditor());
      if(!editor || editor->getWidth()<300)throw std::runtime_error("editor invalid");
    }
    juce::AudioBuffer<float> audio(2,512);
    juce::MidiBuffer midi;
    const auto& params=p->getParameters();
    for(int block=0;block<80;++block){
      for(auto* param:params){
        if(!param)continue;
        const float norm=(block%3==0?0.f:block%3==1?1.f:std::generate_canonical<float,24>(rng));
        param->setValueNotifyingHost(norm);
      }
      for(int i=0;i<512;++i){audio.setSample(0,i,(i%127==0)?1.f:std::sin(float(i+block*512)*.061f)*.3f);audio.setSample(1,i,(i%151==0)?-1.f:std::cos(float(i+block*512)*.047f)*.3f);}
      p->processBlock(audio,midi);
      for(int ch=0;ch<2;++ch)for(int i=0;i<512;++i){const auto v=audio.getSample(ch,i);if(!std::isfinite(v)||std::abs(v)>4.01f)throw std::runtime_error("invalid output");}
    }
    juce::MemoryBlock state;p->getStateInformation(state);
    if(state.getSize()==0)throw std::runtime_error("state not saved");
    p->setStateInformation(state.getData(),int(state.getSize()));
    p->releaseResources();
    std::cout<<"host smoke pass "<<fs<<" Hz\n";
  }
  checkExciterHostSignalPath();
}
