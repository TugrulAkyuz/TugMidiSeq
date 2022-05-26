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
#include "Grids.h"
#include "MyLookanAndFeels.h"


#pragma once

class GlobalPanel   : public juce::Component , juce::Timer  
{
    public:
    GlobalPanel(TugMidiSeqAudioProcessor&  );
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
        getComponentID();
    }
    
    void setGridComp(Grids *g , int i)
    {
        otherg[i] = g;
    }
    
private:
    Grids *otherg[5] = {};
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    
    CustomRoratySlider gridAllNumberSlider;
    CustomRoratySlider gridAllVelSlider;
    juce::ComboBox gridAllSpeedCombo;
    juce::ComboBox gridAllDurationCombo;
    juce::ComboBox presetCombo;
    
    Slider loopBarlenghtSlider;
    Label loopBarCounterLabel;
    TextButton resetButton;
    TextButton velUsageButton;
    
    Label globalNameLabel;
    Label loopBarlenghtSliderLabel;
    
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> loopBarlenghtSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> velUsageButtonAttachment;
 
    TextButton writeButton;
    TextButton deleteButton;
    
    juce::OwnedArray<juce::TextButton> randomButton;
    int preset_index_sil = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};


 
    //==============================================================================
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)  : DocumentWindow (name,
                                                          juce::Colours::lightgrey,
                                                          DocumentWindow::allButtons)
        {
            centreWithSize (300, 200);
            setVisible (true);
        }
 
        void closeButtonPressed() override
        {
           
        }
 
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };
