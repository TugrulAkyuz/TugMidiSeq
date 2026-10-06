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


class ComboBoxDialog :  public Component
{
public:
    ComboBoxDialog(TugMidiSeqAudioProcessor &p) : audioProcessor(p)
    {
        //combo = new juce::ComboBox();
        auto devices = juce::MidiInput::getAvailableDevices();
        int i = 1;
#ifdef JUCE_MAC
        //combo.addItem(myVirtualMidiName, i++);
#endif
        for (auto d : devices)
        {
            combo.addItem(d.name, i);
                i++;
        }
        combo.setSelectedId(1);
       

        addAndMakeVisible(combo);
        addAndMakeVisible(oKButton);
       // setContentComponentSize(100,100);
        setSize(300, 100);
        oKButton.setColour(TextButton::ColourIds::textColourOffId, Colours::pink);
        oKButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
        oKButton.setColour(ComboBox::outlineColourId, Colours::darkgrey);
        oKButton.setButtonText("OK");
        myLookAndFeel.setColour(ComboBox::textColourId, Colours::lightgrey);
        myLookAndFeel.setColour(PopupMenu::backgroundColourId, Colours::black);
        myLookAndFeel.setColour(ComboBox::backgroundColourId, Colours::black);
        combo.setLookAndFeel(&myLookAndFeel);
        oKButton.onClick = [this]()
            {
                String s = combo.getText();
                audioProcessor.setMidiPortName(s);
                if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                            {
                                dw->exitModalState(0);
                            }
               
            };

    }
    ~ComboBoxDialog()
    {
        setLookAndFeel(nullptr);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colours::black);
    }
        
    void resized() override
    {
        auto area = getBounds();
        area.reduce(5, 20);
        auto okba = area.removeFromBottom(area.getHeight()/2);
        combo.setBounds(area.reduced(0.15*area.getWidth(), 0.2*area.getHeight()));
        oKButton.setBounds(okba.reduced(0.3*okba.getWidth(), 0.2*okba.getHeight()));
    }

private:
    MyLookAndFeel myLookAndFeel;
    WheelComboBox combo;
    TextButton oKButton;
    TugMidiSeqAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBoxDialog)
};

class InformationComponent : public Component
{
public:
    InformationComponent()
    {
        a = new ArrowButton("aaaa",0.0f,juce::Colours::orange);
        addAndMakeVisible(a);
        setSize(300, 300);
    }
    void  paint (juce::Graphics& g) override
    {
        
    }
    void resized() override
    {
        a->setBounds(getLocalBounds());
    }
    juce::ArrowButton * a;
    
};

class BasicWindow : public DocumentWindow
{
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicWindow)

public:
    BasicWindow (const String& name, Colour backgroundColour, int buttonsNeeded)
    : DocumentWindow (name, backgroundColour, buttonsNeeded)
    {
        setUsingNativeTitleBar (true);
        setCentrePosition(400, 400);
        setVisible (true);
        centreWithSize (300, 200);
        setResizable(false, false);
        
    }

    void closeButtonPressed() override
    {
        delete this;
    }
};
class NewWindow    : public juce::ResizableWindow
{
public:
    NewWindow (const String& name,
               bool addToDesktop) : juce::ResizableWindow(name,addToDesktop)
    {
        
        setUsingNativeTitleBar (true);
        setCentrePosition(400, 400);
        setVisible (true);
        centreWithSize (300, 200);
        setResizable(false, false);
        addAndMakeVisible(&a);
    }
    void resized() override
     {
         a.setBounds(getLocalBounds());
     }
    void closeButtonPressed()
    {
    delete this;
    };
    juce::Slider a;
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
        addAndMakeVisible(&a);
    }
   void resized() override
    {
        a.setBounds(getLocalBounds());
    }
    
    void closeButtonPressed() override
    {
        delete this;
    }

    juce::Slider a;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};


// A yes / no question in the plugin's own look, shown as a call-out next to
// the button that asks it. Return confirms; Escape or a click outside cancels.
// With no confirm text it is just a note with an OK button.
class ConfirmPanel : public juce::Component
{
public:
    ConfirmPanel (const juce::String& titleText, const juce::String& messageText,
                  const juce::String& confirmText, std::function<void()> onConfirm)
        : title (titleText), message (messageText), confirm (std::move (onConfirm))
    {
        for (auto* b : { &okButton, &cancelButton })
        {
            b->setColour (juce::TextButton::buttonColourId, Theme::surfaceAlt);
            b->setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
            b->setColour (juce::ComboBox::outlineColourId, Theme::hairline);
            addAndMakeVisible (*b);
        }
        okButton.setButtonText (confirmText);
        okButton.setColour (juce::TextButton::buttonColourId, Theme::accent);
        okButton.setColour (juce::TextButton::textColourOffId, Theme::screen);
        okButton.setVisible (confirmText.isNotEmpty());
        cancelButton.setButtonText (confirmText.isNotEmpty() ? "Cancel" : "OK");
        okButton.onClick     = [this] { auto action = confirm; dismiss(); if (action) action(); };
        cancelButton.onClick = [this] { dismiss(); };
        setWantsKeyboardFocus (true);
        setSize (280, 122);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Theme::section);
        auto b = getLocalBounds().reduced (12, 10);
        Theme::drawCaption (g, title, b.removeFromTop (14), juce::Justification::centredLeft, Theme::accentBright, 10.5f);
        b.removeFromTop (6);
        b.removeFromBottom (32);
        g.setColour (Theme::textPrimary);
        g.setFont (Theme::labelFont (12.0f));
        g.drawFittedText (message, b, juce::Justification::topLeft, 5, 1.0f);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (12, 10).removeFromBottom (24);
        cancelButton.setBounds (b.removeFromRight (84));
        b.removeFromRight (8);
        okButton.setBounds (b.removeFromRight (84));
    }

    void visibilityChanged() override { if (isShowing()) grabKeyboardFocus(); }

    bool keyPressed (const juce::KeyPress& k) override
    {
        if (k == juce::KeyPress::returnKey) { (okButton.isVisible() ? okButton : cancelButton).triggerClick(); return true; }
        if (k == juce::KeyPress::escapeKey) { dismiss(); return true; }
        return false;
    }

    static void show (juce::Component& anchor, const juce::String& titleText, const juce::String& messageText,
                      const juce::String& confirmText, std::function<void()> onConfirm)
    {
        launchCallOut (std::make_unique<ConfirmPanel> (titleText, messageText, confirmText, std::move (onConfirm)), anchor);
    }

private:
    void dismiss()
    {
        if (auto* box = findParentComponentOfClass<juce::CallOutBox>())
            box->dismiss();
    }

    juce::String title, message;
    std::function<void()> confirm;
    juce::TextButton okButton, cancelButton;
};

class GlobalPanel   : public juce::Component , juce::Timer  , ChangeListener
{
    public:
    GlobalPanel(TugMidiSeqAudioProcessor&  );
    void  paint (juce::Graphics& g) override;
    void resized() override;
    void ButtonClick(TextButton *b) {};
   ~GlobalPanel()
    {
       updateMidiPort.removeChangeListener(this);
       setLookAndFeel (nullptr);
    }
    
    void timerCallback() override
    {
        int rread_m = audioProcessor.getLoopMeasure();
        loopBarCounterLabel.setText(std::to_string(rread_m),juce::NotificationType::dontSendNotification) ;
        if(myMeasure != 1 && rread_m == 1)
        {
            loopBarlenghtSliderLabel.setColour(Label::textColourId, Colours::red);
            //loopBarlenghtSliderLabel.setColour(Label::backgroundColourId,  Colours::black  );
        }
        else
        {
            loopBarlenghtSliderLabel.setColour(Label::textColourId,  Theme::textSecondary);
            //loopBarlenghtSliderLabel.setColour(Label::backgroundColourId,  Colours::transparentBlack  );
        }
        myMeasure = rread_m;
        getComponentID();
    }

    void changeListenerCallback(ChangeBroadcaster* source) override
    {
        resized();

    }

    void setGridComp(Grids *g , int i)
    {
        otherg[i] = g;
    }
    void deleteConfirmed();
    void deletePresetMenu();
    void factoryConfirmed();
    void stepPreset(int delta);
    void refreshPresetList();
    void showSaveDialog();
    void showFolderMenu();
private:
    int myMeasure;
    Grids *otherg[5] = {};
    MyLookAndFeel myLookAndFeel;
    MyLookAndFeel myLookAndFeel2;
    TugMidiSeqAudioProcessor& audioProcessor;
    Component  associatedComponent;
    CustomRoratySlider gridAllNumberSlider;
    CustomRoratySlider gridAllVelSlider;
    CustomRoratySlider gridAllEventSlider;
    CustomRoratySlider gridGridAllShuffleSlider;
    CustomRoratySlider gridAllDelaySlider;
    WheelComboBox gridAllSpeedCombo;
    WheelComboBox gridAllDurationCombo;
    WheelComboBox presetCombo;
    
    TextButton midiPort;

    CustomRoratySlider gridAllShuffleSlider;
    
    CustomRoratySlider loopBarlenghtSlider;
    Label loopBarCounterLabel;
    TextButton resetButton;
    TextButton velUsageButton;
    TextButton inBuiltSynthButton;
    TextButton sortedOrFirstEmptySelectButton;
    TextButton channelOnButton;
    
    
    Label globalNameLabel;
    Label loopBarlenghtSliderLabel;
    Label ShuffleNameLabel;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> loopBarlenghtSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> velUsageButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> inBultSynthButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> sortedOrFixedButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridAllShuffleSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> channelOnButtonAttachment;
    
    TextButton writeButton;
    TextButton deleteButton;
    TextButton openFolderButton;
    juce::ArrowButton presetPrevButton;
    juce::ArrowButton presetNextButton;
    std::shared_ptr<juce::FileChooser> folderChooser;
    
    juce::OwnedArray<juce::TextButton> randomButton;
    int preset_index_sil = 0;
    AlertWindow *pwdDialog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};




 
  
