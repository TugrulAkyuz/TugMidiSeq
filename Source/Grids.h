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
#include "Satellite.h"
#pragma once

enum  {
    SELECTEDGRID,
    FOLLOWGRID
};

enum  {
    GETDURATION,
    GETSPEED,
    GETNUMOF,
    GETCOORDOFBUTTON,
    GETCURRSETEP,
    GETBUTTONLEN
};
const std::string midiNotes[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
class Grids;

class SubGrids: public juce::Component,   public juce::Timer
{
public:
    SubGrids(Grids& g, TugMidiSeqAudioProcessor& p ,int line, int type) : ownerGrid(g), audioProcessor(p),myLine(line),subGridType(type)
    {
         startTimer(20);
    }
    ~SubGrids()
    {
        
    }
    void  paint (juce::Graphics& g) override;
    void rP()
    {
        repaint();
    }
    void timerCallback()  override
    {
       
        
        //if(ratio == -1 ) return;
        repaint();
        
    }
private:
    int subGridType;
    TugMidiSeqAudioProcessor& audioProcessor;
    Grids& ownerGrid;
    int myLine;

};



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

    void setEnable(bool r)
    {
        gridVelSlider.setEnabled(r);
    }
    int getParam(int type, int index = -1)
    {
        int x = -1;
        if(type ==  GETCOORDOFBUTTON)
        {
            if(buttons[index]->getToggleStateValue() == 1)
               x = buttons[index]->getX() - subGrids->getX();
        }
        if(type ==  GETNUMOF)
            x = gridNumberSlider.getValue();
        return x;
    }
private:
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<juce::TextButton> buttons;
    CustomRoratySlider gridNumberSlider;
    CustomRoratySlider gridVelSlider;
    juce::ComboBox gridSpeedCombo;
    juce::ComboBox gridDurationCombo;
    juce::ArrowButton stepArrow;
    
    juce::TextButton midiInNote;
    juce::Slider octaveSlider;
    juce::Label myLineLabel;
    std::unique_ptr  <SubGrids> subGrids;
    std::unique_ptr  <SubGrids> subGrids2;
    
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
        {
          midiInNote.setButtonText("");
            
           // midiInNote.setColour(juce::Text);
            auto x = getLookAndFeel().findColour(juce::TextButton::ColourIds::buttonColourId);
            midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId, x);
        }
        else
        {
            int i = m / 12;
            m %= 12;
            
            midiInNote.setButtonText(midiNotes[m] + std::to_string(i));
           // midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId, colourarray[myLine]);
            midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId,Colours::lightgreen);
        }
    }
    
    bool stepArray [32];
   juce::OwnedArray    <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachmentArray;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxSpeedAtaachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxDurationAtaachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridNumberSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridVelSliderAttachment;

    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> octaveAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grids)
    
};
