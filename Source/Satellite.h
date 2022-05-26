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

const juce::Colour colourarray [5] = {
    juce::Colours::red,
    juce::Colours::lightgreen,
    juce::Colours::lightblue,
    juce::Colours::yellow,
    juce::Colours::magenta
};

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
    int counter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Satellite)
};
