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


const std::string midiNotes[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

class Grids : public juce::Component,  public juce::Timer
{
public:
    Grids(TugMidiSeqAudioProcessor&,int line);
    ~Grids()
    {
        setLookAndFeel (nullptr);
    }
    void  paint (juce::Graphics& g) override;
    void resized() override;

    
private:
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<juce::TextButton> buttons;
    CustomRoratySlider gridNumberSlider;
    CustomRoratySlider gridVelSlider;
    juce::ComboBox gridSpeedCombo;
    juce::ComboBox gridDurationCombo;
    
    juce::TextButton midiInNote;
    juce::Slider octaveSlider;
    
    bool dirt = false;
    int step;
    int myLine;
    int myMidiNote;
    void timerCallback() override
    {
        int st = audioProcessor.getSteps(myLine);
        if(step != st)
        {
            step = st;
            resized();
        }
        int midi = audioProcessor.getMidi(myLine);
        if(myMidiNote != midi)
            setMidiName(midi);
     

        myMidiNote =  midi;
    }
    
    void setMidiName(int m)
    {
        if(m ==  -1)
          midiInNote.setButtonText("");
        else
        {
            int i = m / 12;
            m %= 12;
            midiInNote.setButtonText(midiNotes[m] + std::to_string(i));
        }
    }
    
    bool stepArray [32];
   juce::OwnedArray    <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachmentArray;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxSpeedAtaachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxDurationAtaachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridNumberSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grids)
    
};
