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

extern ChangeBroadcaster myGridChangeListener;

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
    void resized() override;
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

    MultiStateButton(TugMidiSeqAudioProcessor& p, int line , int step) : juce::Button("MultiStateButton"),
    audioProcesor(p),myLine(line), myStep(step)
    {
        setClickingTogglesState(true);
        //setInterceptsMouseClicks (false, true);
      
    }
    ~MultiStateButton()
    {
       
        myState->removeParameterListener(myParameterID, this);

    }
  
    // Left-drag "paint" and shift-drag velocity. Bodies live in Grids.cpp
    // because the paint path calls back into the (here still incomplete) Grids.
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp   (const MouseEvent& e) override;

    void setOwnerGrid (Grids* g) { ownerGrid = g; }

    // Next state in the click cycle (mirrors MultiStateButtonAttachment).
    State cycledState (bool ctrl) const
    {
        if (currentState == State::ButtonOffState && ! ctrl) return State::ButtonOnState;
        if (currentState == State::ButtonOffState &&   ctrl) return State::ButtonEventState;
        return State::ButtonOffState;
    }

    // Apply a state directly (drag-painting) and persist it to the parameter.
    void paintTo (State s)
    {
        setCurrentState (s);
        juce::String pid;
        pid << "block" << myLine << myStep;
        audioProcesor.valueTreeState.getParameterAsValue (pid).setValue ((float) s);
    }


    void mouseMove(const MouseEvent& e) override
    {
        if(shiftPressed == true)
        {
            
        }
       
    }

    void mouseExit(const MouseEvent& e) override
    {
        shiftPressed = false;
        //setInterceptsMouseClicks(true, false);
    }

   
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto b    = getLocalBounds().toFloat().reduced (0.5f);
        auto lane = colourarray[myLine];

        if (currentState == State::ButtonOffState)
        {
            // recessed, unlit pad
            g.setColour (Theme::well);
            g.fillRoundedRectangle (b, Theme::radSm);
            g.setColour (Theme::hairline.withAlpha (isMouseOverButton ? 1.0f : 0.6f));
            g.drawRoundedRectangle (b, Theme::radSm, 1.0f);
        }
        else
        {
            // lit pad, brightness tracks velocity, tinted to the lane colour
            float v   = jmap (audioProcesor.getVelButton (myLine, myStep, true), 0.0f, 1.0f, 0.35f, 1.0f);
            auto  lit = lane.withMultipliedBrightness (0.62f + 0.5f * v);
            g.setColour (lit);
            g.fillRoundedRectangle (b, Theme::radSm);
            g.setColour (Colours::white.withAlpha (0.14f * v));      // top glow
            g.drawLine (b.getX() + Theme::radSm, b.getY() + 1.0f,
                        b.getRight() - Theme::radSm, b.getY() + 1.0f, 1.0f);
            g.setColour (lane.brighter (0.35f));
            g.drawRoundedRectangle (b, Theme::radSm, 1.0f);
        }

        if (currentState == State::ButtonEventState)
        {
            // probability "dice" mark: event bow-tie + accent corner dot
            juce::Path p;
            int   y     = getHeight() / 2;
            float effet = cos (evenAlpha * 3.14 / 2) * y / 2;
            g.setColour (Theme::screen.withAlpha (0.85f));
            p.startNewSubPath (getWidth() / 5.0f, y + effet);
            p.lineTo (1.5 * getWidth() / 5, y + effet);
            p.lineTo (3.5 * getWidth() / 5, y - effet);
            p.lineTo (4.0 * getWidth() / 5, y - effet);
            g.strokePath (p, PathStrokeType (2.0f));
            p.clear();
            p.startNewSubPath (getWidth() / 5.0f, y - effet);
            p.lineTo (1.5 * getWidth() / 5, y - effet);
            p.lineTo (3.5 * getWidth() / 5, y + effet);
            p.lineTo (4.0 * getWidth() / 5, y + effet);
            g.strokePath (p, PathStrokeType (2.0f));

            g.setColour (Theme::accentBright);
            g.fillEllipse ((float) getWidth() - 5.0f, 2.0f, 3.0f, 3.0f);
        }

        if (shiftPressed == true)
            showVelocity (g, audioProcesor.getVelButton (myLine, myStep));
    }

    void showVelocity(juce::Graphics& g,float value)
    {
        juce::Path backgroundArc;
        
         //g.setColour(Colours::darkgrey);
        //g.fillAll();
        Colour tmpC = Theme::screen.withAlpha (0.9f);
        g.setColour(tmpC);
        int centerX = g.getClipBounds().getCentreX();
        int centerY = g.getClipBounds().getCentreY();
        g.drawLine(centerX, centerY, (centerX) - cos(value * 6 * 3.14 / 4 - 1 * 3.14 / 4  )*(centerX-2), (centerY) -  sin(value * 6 * 3.14 / 4  - 1 * 3.14 / 4)*(centerY-2), 3);
        
        backgroundArc.addCentredArc(centerX,
                                    centerY,
                                    centerX-2,
                                    centerY-2,
            0.0f,
            -3 * 3.14 / 4,
            value * 6 * 3.14 / 4 - 3 * 3.14 / 4,
            true);
        PathStrokeType stroke(3.0f, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded);
        g.strokePath(backgroundArc, stroke);

        backgroundArc.addCentredArc(centerX,
                                    centerY,
                                    centerX-2,
                                    centerY-2,
            0.0f,
            -3 * 3.14 / 4,
            1.0 * 6 * 3.14 / 4 - 3 * 3.14 / 4,
            true);
        stroke.setStrokeThickness(1.5f);
        g.setColour(tmpC.withAlpha(0.5f));
        g.strokePath(backgroundArc, stroke);

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
    
    bool shiftPressed = false;
    juce::String myParameterID;
    TugMidiSeqAudioProcessor& audioProcesor;
    int myLine, myStep;
    juce::AudioProcessorValueTreeState *myState;
    State currentState = State::ButtonOffState;
    float y;
    float evenAlpha =1.0;
    Grids* ownerGrid = nullptr;
    
    
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

class Grids : public juce::Component,  public juce::Timer , private AudioProcessorValueTreeState::Listener,ChangeListener
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
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDSHUFFLE] << myLine;
        audioProcessor.valueTreeState.removeParameterListener(tmp_s, this);
        myGridChangeListener.removeChangeListener(this);
        setLookAndFeel (nullptr);
    }
    void  paint (juce::Graphics& g) override;
    void  paintOverChildren (juce::Graphics& g) override;
    void resized() override;
    void parameterChanged (const String& parameterID, float newValue) override
    {
      
        myShuffleChabged =  true;
      
       
    }

    void changeListenerCallback(ChangeBroadcaster *source) override
    {
        resized();

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

    // Drag-to-paint across the step pads — and across lanes. The origin pad
    // opens the gesture with a brush state; every pad the cursor then crosses,
    // in this lane or any sibling lane, is painted to it once.
    void beginPaint (MultiStateButton::State s, int originStep);
    void paintDrag  (juce::Point<int> screenPos);
    void endPaint();
private:
    // Stamp any pad under screenPos in THIS lane with brush state s.
    void paintLocal (juce::Point<int> screenPos, MultiStateButton::State s);
    void resetPainted() { for (auto& p : paintedStep) p = false; }

    bool myShuffleChabged = false;
    bool painting = false;
    MultiStateButton::State brushState = MultiStateButton::State::ButtonOffState;
    bool paintedStep[numOfStep] = {};
    MyLookAndFeel myLookAndFeel;
    MyLookAndFeel myLookAndFeel2;
    TugMidiSeqAudioProcessor& audioProcessor;
    juce::OwnedArray<MultiStateButton> buttons;
    juce::OwnedArray<CustomRoratySlider> velButtons;
    CustomRoratySlider gridNumberSlider;
    CustomRoratySlider gridVelSlider;
    CustomRoratySlider gridEventSlider;
    CustomRoratySlider gridShuffleSlider;
    CustomRoratySlider gridDelaySlider;
    WheelComboBox gridSpeedCombo;
    WheelComboBox gridDurationCombo;
    juce::ArrowButton stepArrow;

    WheelComboBox gridMidiRouteCombo;
    
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
        repaint();
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
            midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
            midiInNote.setColour(juce::TextButton::textColourOffId, Theme::textSecondary);
        }
        else
        {
            int i = m / 12;
            m %= 12;

            midiInNote.setButtonText(midiNotes[m] + std::to_string(i));
            midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId, colourarray[myLine]);
            midiInNote.setColour(juce::TextButton::textColourOffId, Theme::screen);
        }
    }
    
    juce::OwnedArray    <MultiStateButtonAttachment> buttonAttachmentArray;
    juce::OwnedArray  <AudioProcessorValueTreeState::SliderAttachment> velButtonAttachmentArray;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxSpeedAtaachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  comBoxDurationAtaachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridNumberSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridVelSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridEventSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridShuffleSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridDelaySliderAttachment;
    std::unique_ptr < AudioProcessorValueTreeState::ComboBoxAttachment>  gridMidiRouteAttachment;

    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> octaveAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Grids)
    
};
