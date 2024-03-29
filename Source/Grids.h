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
       
        myState->removeParameterListener(myParameterID, this);

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

        g.fillAll(juce::Colours::darkgrey);
        g.fillAll(buttonColor);
        if (currentState == State::ButtonEventState)
        {
            g.setColour(Colours::red);
            Rectangle<float> area(getWidth() - 4, 1, 3, 3);
            g.fillEllipse(area);
        }
           
       // g.drawText(t, 0, 0, getWidth(), getHeight(), Justification::centred);
        g.setColour(Colours::darkgrey);
        if(currentState == State::ButtonOnState)
        {
            Line <float> l  (getWidth()/3,getHeight()/2,3*getWidth()/4,getHeight()/2);
            g.drawArrow(l, 2, 3, 4);
        }
        if(currentState == State::ButtonEventState)
        {
            juce::Path p;
            int  y = getHeight()/2;
            float effet = sin(evenAlpha*3.14)*y/2;
            p.startNewSubPath(getWidth()/5, y + effet);
            p.lineTo(2*getWidth()/5, y + effet);
            p.lineTo(3*getWidth()/5, y - effet);
            p.lineTo(4*getWidth()/5, y - effet);
            g.strokePath(p, PathStrokeType(2.0f));
            p.clear();
            p.startNewSubPath(getWidth()/5, y - effet);
            p.lineTo(2*getWidth()/5, y - effet);
            p.lineTo(3*getWidth()/5, y + effet);
            p.lineTo(4*getWidth()/5, y + effet);
            g.strokePath(p, PathStrokeType(2.0f));
            //p.cubicTo(getWidth()/3, 0, <#float controlPoint2X#>, <#float controlPoint2Y#>, <#float endPointX#>, <#float endPointY#>)
        }
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
        myState = &state;
        myParameterID = parameterID;
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
        if(parameterID.contains(valueTreeNames[BLOCK]) == true)
        {
            setCurrentState((State)newValue);
            return;
        }
       
    }
    void setCurrentAlpha( float a)
    {
        evenAlpha = a;
        repaint();
    }
private:
    juce::String myParameterID;
    juce::AudioProcessorValueTreeState *myState;
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
           
            button.setCurrentState(newValue);
            state.getParameterAsValue(parameterID).setValue(x);
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
            button.setparameterListener(state,parameterID);
            auto currentValue = parameter->convertFrom0to1(parameter->getValue());
            auto currentState = static_cast<MultiStateButton::State>(currentValue);
            button.setCurrentState(currentState);
            
        }
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiStateButtonAttachment)
};

class Grids : public juce::Component,  public juce::Timer , private AudioProcessorValueTreeState::Listener
{
public:
    Grids(TugMidiSeqAudioProcessor&,int line);
    ~Grids()
    {
        juce::String  tmp_s;
        for (int i = 0; i < numOfStep; ++i)
        {
            
            tmp_s.clear();
            tmp_s << "block" << myLine << i;
            MultiStateButton *  st = buttons.getUnchecked(i);
            tmp_s.clear();
            tmp_s << valueTreeNames[EVENT] << myLine;
            audioProcessor.valueTreeState.removeParameterListener(tmp_s,(juce::AudioProcessorValueTreeState::Listener *)(st));
        };;
        
       audioProcessor.valueTreeState.removeParameterListener(valueTreeNames[SHUFFLE], this);
    
        setLookAndFeel (nullptr);
    }
    void  paint (juce::Graphics& g) override;
    void resized() override;
    void parameterChanged (const String& parameterID, float newValue) override
    {
      
        myShuffleChabged =  true;
      
       
    }
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
    bool myShuffleChabged = false;
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
     
        if(myShuffleChabged == true)
        {
            resized();
            myShuffleChabged = false;
        }
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
