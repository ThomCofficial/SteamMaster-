#include "PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
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
}
