/*
  ==============================================================================

    GlobalPanel.h
    Created: 23 May 2022 9:19:45pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MyLookanAndFeels.h"
#pragma once

class GlobalPanel   : public juce::Component
{
    public:
    GlobalPanel(TugMidiSeqAudioProcessor&);
    void  paint (juce::Graphics& g) override;
    void resized() override;
   ~GlobalPanel()
    {
       setLookAndFeel (nullptr);
    }
private:
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    
    CustomRoratySlider gridAllNumberSlider;
    CustomRoratySlider gridAllVelSlider;
    juce::ComboBox gridAllSpeedCombo;
    juce::ComboBox gridAllDurationCombo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};
