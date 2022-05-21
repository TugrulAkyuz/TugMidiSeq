/*
  ==============================================================================

    Grids.h
    Created: 13 May 2022 3:35:51pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MyLookanAndFeels.h"
#pragma once

class Grids : public juce::Component,  public juce::Timer
{
public:
    Grids(TugMidiSeqAudioProcessor&,int line);
    void  paint (juce::Graphics& g) override;
    void resized() override;

    
private:
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<juce::TextButton> buttons;
    CustomRoratySlider gridNumberSlider;
    CustomRoratySlider gridVelSlider;
    juce::ComboBox gridSpeedCombo;
    juce::ComboBox gridDurationCombo;
    bool dirt = false;
    int step;
    int myLine;
    void timerCallback() override
    {
        int st = audioProcessor.getSteps(myLine);
        if(step != st)
        {
            step = st;
            resized();
        }
 
    }
    bool stepArray [32];
   juce::OwnedArray    <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachmentArray;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxSpeedAtaachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxDurationAtaachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridNumberSliderAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grids)
    
};
