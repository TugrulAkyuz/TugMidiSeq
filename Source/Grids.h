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
        stopTimer();
    }
    void resized() override;
    void  paint (juce::Graphics& g) override;
    void rP()
    {
        repaint();
    }
    void timerCallback()  override
    {
        // remember the last couple of played steps: Random draws them as a trail
        const int st = audioProcessor.getPlayheadStep (myLine);
        if (st != lastPlayStep)
        {
            if (st < 0)                 { trail[0] = trail[1] = -1; }
            else if (lastPlayStep >= 0) { trail[0] = trail[1]; trail[1] = lastPlayStep; }
            lastPlayStep = st;
        }
        repaint();
    }
private:
    // Playhead under the pads, at the step actually being played and moving the
    // way the lane travels: rightward, leftward (Reverse / Ping-Pong's way back),
    // or, for Random, just under the current pad with a fading trail.
    void drawPlayhead (juce::Graphics& g, int step, int numSteps);
    int lastPlayStep = -1;
    int trail[2] = { -1, -1 };   // older, newer
    int subGridType;
    TugMidiSeqAudioProcessor& audioProcessor;
    Grids& ownerGrid;
    int myLine;

};




// Floating read-out for drag-editing a step value (shift+drag velocity,
// alt+drag pitch). It lives in the editor content (not inside the tiny pad),
// sits just above the pad being edited and points at it; created on
// mouse-down, removed on mouse-up.
class StepValuePopup : public juce::Component
{
public:
    StepValuePopup (juce::Colour laneColour) : lane (laneColour)
    {
        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (true);
    }

    // caption + value text, and a meter filled from `from` to `to` (0..1);
    // dim: the value currently has no effect (e.g. step velocity in "In Vel" mode)
    void setContent (const juce::String& newCaption, const juce::String& newValue,
                     float from, float to, bool dim)
    {
        if (newCaption == caption && newValue == valueText && from == meterFrom && to == meterTo && dim == isDim)
            return;
        caption = newCaption; valueText = newValue;
        meterFrom = from; meterTo = to; isDim = dim;
        repaint();
    }

    // place above `pad` (inside `parent`), or below it when there's no room
    void placeFor (juce::Component& pad, juce::Component& parent)
    {
        auto padArea = parent.getLocalArea (&pad, pad.getLocalBounds());
        auto b = juce::Rectangle<int> (width, height).withCentre ({ padArea.getCentreX(), 0 });
        pointsDown = padArea.getY() - height - gap >= 0;
        b.setY (pointsDown ? padArea.getY() - height - gap : padArea.getBottom() + gap);
        setBounds (b.constrainedWithin (parent.getLocalBounds()));
        arrowX = (float) (padArea.getCentreX() - getX());
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        auto body = pointsDown ? b.withTrimmedBottom (arrow) : b.withTrimmedTop (arrow);

        // pointer toward the pad
        juce::Path tip;
        const float ax = jlimit (body.getX() + 8.0f, body.getRight() - 8.0f, arrowX);
        if (pointsDown) tip.addTriangle (ax - arrow, body.getBottom() - 1.0f, ax + arrow, body.getBottom() - 1.0f, ax, b.getBottom());
        else            tip.addTriangle (ax - arrow, body.getY() + 1.0f,      ax + arrow, body.getY() + 1.0f,      ax, b.getY());
        g.setColour (Theme::surface);
        g.fillPath (tip);
        g.setColour (Theme::hairline);
        g.strokePath (tip, juce::PathStrokeType (1.0f));

        Theme::drawRaisedPanel (g, body, Theme::radMd);
        auto inner = body.reduced (6.0f, 4.0f);

        // caption + number on one row
        auto top = inner.removeFromTop (12.0f);
        Theme::drawCaption (g, caption, top.toNearestInt(),
                            juce::Justification::centredLeft, isDim ? Theme::textDim : Theme::textSecondary, 9.0f);
        g.setColour (isDim ? Theme::textDim : Theme::textValue);
        g.setFont (Theme::valueFont (12.0f));
        g.drawText (valueText, top, juce::Justification::centredRight, false);

        // meter in the lane colour
        inner.removeFromTop (3.0f);
        auto meter = inner.removeFromTop (5.0f);
        Theme::drawRecessedWell (g, meter, 2.0f);
        g.setColour (isDim ? Theme::textDim : lane);
        const float x0 = meter.getX() + meter.getWidth() * jmin (meterFrom, meterTo);
        const float x1 = meter.getX() + meter.getWidth() * jmax (meterFrom, meterTo);
        g.fillRoundedRectangle (meter.withX (x0).withWidth (jmax (2.0f, x1 - x0)), 2.0f);
        if (meterFrom == 0.5f)   // centre-zero meter (pitch): mark the zero
        {
            g.setColour (Theme::textSecondary);
            g.fillRect (meter.getCentreX() - 0.5f, meter.getY() - 1.5f, 1.0f, meter.getHeight() + 3.0f);
        }
    }

private:
    static constexpr int width = 88, height = 36, gap = 1;
    static constexpr float arrow = 5.0f;
    juce::Colour lane;
    juce::String caption, valueText;
    float meterFrom = 0.0f, meterTo = 0.0f;
    bool isDim = false;
    bool pointsDown = true;
    float arrowX = 0.0f;
};

// Call-out from the lane menu: Euclidean fill with live Hits / Rotate knobs.
// The whole session (open -> close) is one undo step.
class EuclidPanel : public juce::Component
{
public:
    EuclidPanel (TugMidiSeqAudioProcessor& p, int line);
    ~EuclidPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TugMidiSeqAudioProcessor& proc;
    int line, length;
    CustomRoratySlider hits, rotate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EuclidPanel)
};

class MultiStateButton : public juce::Button ,  private AudioProcessorValueTreeState::Listener,
                         private juce::AsyncUpdater
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
        cancelPendingUpdate();

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

        // Mutate shows what will actually play: a written step that rests is
        // dimmed, an empty one that plays gets a faint lane fill (plus the dashed
        // frame drawn further down)
        const bool mutated = audioProcesor.isStepMutated (myLine, myStep);
        if (mutated)
        {
            g.setColour (currentState == State::ButtonOffState ? lane.withAlpha (0.62f)
                                                                : Theme::well.withAlpha (0.86f));
            g.fillRoundedRectangle (b, Theme::radSm);
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

        const int cond = audioProcesor.getStepCond (myLine, myStep);
        if (cond != CondNone)
            drawCondTag (g, b, cond, currentState != State::ButtonOffState);

        const int ratchet = audioProcesor.getStepRatchet (myLine, myStep);
        if (ratchet > 1)
            drawRatchetMarks (g, b, ratchet, currentState != State::ButtonOffState);

        const int pitch = audioProcesor.getStepPitch (myLine, myStep);
        if (pitch != 0)
            drawPitchTag (g, b, pitch, currentState != State::ButtonOffState);

        // flipped by Mutate for now: a dashed frame (the written state is unchanged)
        if (mutated)
        {
            juce::Path frame, dashed;
            frame.addRoundedRectangle (b.reduced (1.0f), Theme::radSm);
            const float dashes[] = { 2.5f, 2.0f };
            juce::PathStrokeType (1.3f).createDashedStroke (dashed, frame, dashes, 2);
            g.setColour (Theme::accentBright);
            g.fillPath (dashed);
        }

        // being shift-dragged: outline the pad the velocity popup points at
        if (valuePopup != nullptr)
        {
            g.setColour (Theme::accentBright);
            g.drawRoundedRectangle (b, Theme::radSm, 1.5f);
        }
    }

    // Trig-condition tag: a dark chip along the pad's bottom edge with the
    // condition's short name, or just a corner flag when the pad is too narrow
    // for text (long lanes). Dimmed on an off pad, where it has no effect.
    void drawCondTag (juce::Graphics& g, juce::Rectangle<float> b, int cond, bool active)
    {
        auto lane = colourarray[myLine];
        const float alpha = active ? 1.0f : 0.45f;

        if (b.getWidth() < 18.0f)
        {
            // dark on a lit pad, lane-coloured on an unlit one: readable in every lane
            juce::Path flag;
            flag.addTriangle (b.getX(), b.getBottom(), b.getX() + 7.0f, b.getBottom(), b.getX(), b.getBottom() - 7.0f);
            g.setColour (active ? Theme::screen.withAlpha (0.85f) : lane.withAlpha (alpha));
            g.fillPath (flag);
            return;
        }

        auto chip = b.removeFromBottom (9.0f).reduced (1.5f, 0.5f);
        g.setColour (Theme::screen.withAlpha (0.78f * alpha));
        g.fillRoundedRectangle (chip, 2.0f);
        g.setColour (lane.brighter (0.5f).withAlpha (alpha));
        g.setFont (Theme::valueFont (8.5f));
        g.drawText (trigCondNames[cond], chip, juce::Justification::centred, false);
    }

    // Ratchet: the pad's upper half is split into as many parts as the step has
    // hits, like a subdivided note.
    void drawRatchetMarks (juce::Graphics& g, juce::Rectangle<float> b, int hits, bool active)
    {
        g.setColour (active ? Theme::screen.withAlpha (0.75f) : colourarray[myLine].withAlpha (0.6f));
        for (int k = 1; k < hits; k++)
        {
            const float x = b.getX() + b.getWidth() * (float) k / (float) hits;
            g.drawLine (x, b.getY() + 2.0f, x, b.getY() + b.getHeight() * 0.45f, 1.3f);
        }
    }

    // Step pitch: "+3" / "-5" in the top-left corner, or a small up / down
    // triangle when the pad is too narrow for text.
    void drawPitchTag (juce::Graphics& g, juce::Rectangle<float> b, int pitch, bool active)
    {
        g.setColour (active ? Theme::screen.withAlpha (0.85f) : colourarray[myLine].withAlpha (0.75f));
        if (b.getWidth() < 18.0f)
        {
            juce::Path t;
            const float x = b.getX() + 2.0f, y = b.getY() + 2.0f;
            if (pitch > 0) t.addTriangle (x, y + 5.0f, x + 6.0f, y + 5.0f, x + 3.0f, y);
            else           t.addTriangle (x, y, x + 6.0f, y, x + 3.0f, y + 5.0f);
            g.fillPath (t);
            return;
        }
        g.setFont (Theme::valueFont (8.5f));
        g.drawText ((pitch > 0 ? "+" : "") + juce::String (pitch),
                    b.reduced (2.0f, 1.0f).removeFromTop (9.0f), juce::Justification::topLeft, false);
    }

    // Right-click: this step's ratchet, pitch reset and trig condition. Defined in Grids.cpp.
    void showStepMenu();

    // Shift+drag velocity editing: the popup is parented to the editor content
    // (the lane's parent) so it can sit outside this pad and the lane row.
    // Bodies in Grids.cpp.
    void showValuePopup();
    void updateValuePopup();
    void hideValuePopup();
    juce::RangedAudioParameter* stepVelParam() const;

    State getCurrentState() const
    {
        return currentState;
    }

    // Message-thread only (mouse handling / attachment).
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
 
    // Called by the host on the automation/audio thread, so this may only touch
    // atomics — repainting a Component off the message thread corrupts its
    // pending-repaint RectangleList. The redraw is deferred to handleAsyncUpdate().
    void parameterChanged (const String& parameterID, float newValue) override
    {
        if(parameterID.contains(valueTreeNames[EVENT]) == true)
        {
            evenAlpha = newValue /100;
            triggerAsyncUpdate();
            return;
        }
        if(parameterID.contains(valueTreeNames[BLOCK]) == true)
        {
            currentState = (State)newValue;
            triggerAsyncUpdate();
            return;
        }

    }

    // Message thread: pick up whatever parameterChanged() stored.
    void handleAsyncUpdate() override
    {
        repaint();
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
    // Written by parameterChanged() on the host's automation thread, read by
    // paint() on the message thread.
    std::atomic<State> currentState { State::ButtonOffState };
    float y;
    std::atomic<float> evenAlpha { 1.0f };
    Grids* ownerGrid = nullptr;
    std::unique_ptr<StepValuePopup> valuePopup;
    bool altPressed = false;   // alt+drag edits the step's pitch
    float pitchDrag = 0.0f;    // drag distance not yet turned into a semitone / degree
    
    
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

    // the pads' popup menus share the lane's look and feel
    juce::LookAndFeel& getMenuLookAndFeel() { return myLookAndFeel; }

    // a pad's bounds in the SubGrids overlay's coordinates
    juce::Rectangle<float> padBounds (int step) const
    {
        if (step < 0 || step >= buttons.size()) return {};
        return buttons[step]->getBounds().toFloat()
                   .translated (-(float) subGrids->getX(), -(float) subGrids->getY());
    }

    // clicks on the lane number open the lane menu
    void mouseDown (const juce::MouseEvent& e) override;
private:
    void showLaneMenu();
    void drawDirectionGlyph (juce::Graphics& g, juce::Rectangle<float> area) const;
    juce::Rectangle<int> directionArea;   // under the lane number
    // Stamp any pad under screenPos in THIS lane with brush state s.
    void paintLocal (juce::Point<int> screenPos, MultiStateButton::State s);
    void resetPainted() { for (auto& p : paintedStep) p = false; }

    // Set from parameterChanged() on the automation thread, consumed by the timer.
    std::atomic<bool> myShuffleChabged { false };
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
