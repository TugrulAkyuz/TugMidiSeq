/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Grids.h"




//==============================================================================
/**
*/
class TugMidiSeqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor&);
    ~TugMidiSeqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;



private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<juce::TextButton> buttons;
 
    juce::OwnedArray< Grids> grids;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessorEditor)
};
