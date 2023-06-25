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



class MultiStateButton : public juce::Button ,  private AudioProcessorValueTreeState::Listener
{
public:
    enum class State
    {
        ButtonOffState,
        ButtonOnState,
        ButtonEventState
    };

    MultiStateButton() : juce::Button("MultiStateButton")
    {
        setClickingTogglesState(true);
       
    }
    ~MultiStateButton()
    {
        setClickingTogglesState(true);
       
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::Colour buttonColor;
        String t = "";;
        // Determine the color based on the current state
        switch (currentState)
        {
            case State::ButtonOffState:
                buttonColor = juce::Colours::darkgrey;
                break;
            case State::ButtonOnState:
                buttonColor = juce::Colours::orange;
                t = "F";
                break;
            case State::ButtonEventState:
                buttonColor = juce::Colours::orange.withAlpha(juce::jlimit(0.1f, 1.0f, evenAlpha));
                t = "R";
                break;
        }

      
        g.fillAll(buttonColor);
        g.drawText(t, 0, 0, getWidth(), getHeight(), Justification::centred);
        //juce::Button::paintButton(g, isMouseOverButton, isButtonDown);
    }

    State getCurrentState() const
    {
        return currentState;
    }

    void setCurrentState(State newState) 
    {
        currentState = newState;
        repaint();
    }
    void setparameterListener(juce::AudioProcessorValueTreeState& state,juce::String& parameterID)
    {
         
        state.addParameterListener ( parameterID, this);
    }
    void parameterChanged (const String& parameterID, float newValue) override
    {
        
        
        // Called when parameter "yourParamId" is changed.
        if(parameterID.contains(valueTreeNames[EVENT]) == true)
        {
            evenAlpha = newValue /100;
            return;
        }
        setCurrentState((State)newValue);
    }
    void setCurrentAlpha( float a)
    {
        evenAlpha = a;
        repaint();
    }
private:
    State currentState = State::ButtonOffState;
    float evenAlpha =1.0;;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiStateButton)
    
};

class MultiStateButtonAttachment : public juce::Button::Listener
{
public:
    MultiStateButtonAttachment(juce::AudioProcessorValueTreeState& state,
                               const juce::String& parameterID,
                               MultiStateButton& button)
        : state(state),
          parameterID(parameterID),
          button(button)
    {
        button.addListener(this);
        updateButtonStateFromParameter();
    }

    ~MultiStateButtonAttachment()
    {
        button.removeListener(this);
    }

    void buttonClicked(juce::Button* clickedButton) override
    {
      
        if (clickedButton == &button)
        {
            auto ctrl_down = juce::ModifierKeys::getCurrentModifiers().isCtrlDown();
            auto currentState = button.getCurrentState();
            MultiStateButton::State newValue;
            if(currentState == MultiStateButton::State::ButtonOffState && ctrl_down == false)
                newValue =  MultiStateButton::State::ButtonOnState;
            else if(currentState == MultiStateButton::State::ButtonOffState && ctrl_down == true)
                newValue =  MultiStateButton::State::ButtonEventState;
            else  newValue =  MultiStateButton::State::ButtonOffState;
            float x = (float)newValue;
            state.getParameter(parameterID)->setValueNotifyingHost(x);
            button.setCurrentState(newValue);
        }
    }



private:
    juce::AudioProcessorValueTreeState& state;
    juce::String parameterID;
    MultiStateButton& button;

    void updateButtonStateFromParameter()
    {
        auto* parameter = state.getParameter(parameterID);
        if (parameter != nullptr)
        {
          
            auto currentValue = parameter->convertFrom0to1(parameter->getValue());
            auto currentState = static_cast<MultiStateButton::State>(currentValue);
            button.setCurrentState(currentState);
            button.setparameterListener(state,parameterID);
        }
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiStateButtonAttachment)
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
            if(buttons[index]->getCurrentState() != MultiStateButton::State::ButtonOffState)
               x = buttons[index]->getX() - subGrids->getX();
        }
        if(type ==  GETNUMOF)
            x = gridNumberSlider.getValue();
        return x;
    }
private:
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<MultiStateButton> buttons;
    CustomRoratySlider gridNumberSlider;
    CustomRoratySlider gridVelSlider;
    CustomRoratySlider gridEventSlider;
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
   juce::OwnedArray    <MultiStateButtonAttachment> buttonAttachmentArray;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxSpeedAtaachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxDurationAtaachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridNumberSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridVelSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridEventSliderAttachment;

    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> octaveAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grids)
    
};
