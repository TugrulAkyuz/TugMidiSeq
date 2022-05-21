/*
  ==============================================================================

    stellite.h
    Created: 21 May 2022 2:45:47pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/
#include <JuceHeader.h>
#include "PluginProcessor.h"
#pragma once

class Satellite   : public juce::Component , public juce::Timer
{
    public:
    Satellite(TugMidiSeqAudioProcessor&);
    void  paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
 
private:
    TugMidiSeqAudioProcessor& audioProcessor;
    int statellitePosition[32];
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Satellite)
};
