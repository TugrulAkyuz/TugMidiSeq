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

class GlobalPanel   : public juce::Component , juce::Timer
{
    public:
    GlobalPanel(TugMidiSeqAudioProcessor&);
    void  paint (juce::Graphics& g) override;
    void resized() override;
    void ButtonClick(TextButton *b) {};
   ~GlobalPanel()
    {
       setLookAndFeel (nullptr);
    }
    void timerCallback() override
    {
        loopBarCounterLabel.setText(std::to_string(audioProcessor.getLoopMeasure()),juce::NotificationType::dontSendNotification) ;
    }
private:
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    
    CustomRoratySlider gridAllNumberSlider;
    CustomRoratySlider gridAllVelSlider;
    juce::ComboBox gridAllSpeedCombo;
    juce::ComboBox gridAllDurationCombo;
    
    Slider loopBarlenghtSlider;
    Label loopBarCounterLabel;
    TextButton resetButton;
    TextButton velUsageButton;
    
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> loopBarlenghtSliderAttachment;
 
    juce::OwnedArray<juce::TextButton> randomButton;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};
