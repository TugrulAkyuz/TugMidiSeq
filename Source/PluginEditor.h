/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Grids.h"
#include "Satellite.h"
#include "GlobalPanel.h"


const std::vector <std::string> topLabel={"midi in","Octave","G R I D S","#Grid","Speed","Duration","Vel","Event","Shuffle","Delay","Route","Note Satellite"};

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
 
 
    juce::OwnedArray< Grids> grids;
    Satellite satellite;
    GlobalPanel globalPanel;
    juce::OwnedArray< juce::Label > topInLabel;
    
    
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessorEditor)
};

