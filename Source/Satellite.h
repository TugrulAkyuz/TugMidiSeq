/*
  ==============================================================================

    stellite.h
    Created: 21 May 2022 2:45:47pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Theme.h"
#pragma once

// Curated Eurorack lane palette (see Theme.h). Kept under the original name so
// every existing reference (Grids, GlobalPanel, Satellite) picks it up unchanged.
const juce::Colour colourarray [5] = {
    Theme::lane[0], Theme::lane[1], Theme::lane[2], Theme::lane[3], Theme::lane[4]
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
