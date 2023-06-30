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
        int rread_m = audioProcessor.getLoopMeasure();
        loopBarCounterLabel.setText(std::to_string(rread_m),juce::NotificationType::dontSendNotification) ;
        if(myMeasure != 1 && rread_m == 1)
        {
            loopBarlenghtSliderLabel.setColour(Label::textColourId, Colours::red);
            //loopBarlenghtSliderLabel.setColour(Label::backgroundColourId,  Colours::black  );
        }
        else
        {
            loopBarlenghtSliderLabel.setColour(Label::textColourId,  Colours::lightgrey);
            //loopBarlenghtSliderLabel.setColour(Label::backgroundColourId,  Colours::transparentBlack  );
        }
        myMeasure = rread_m;
        getComponentID();
    }
    
    void setGridComp(Grids *g , int i)
    {
        otherg[i] = g;
    }
    void deleteConfirmed();
    void deletePresetMenu();
    void factoryConfirmed();
    void setPresetMenu(String preset_name);
private:
    int myMeasure;
    Grids *otherg[5] = {};
    MyLookAndFeel myLookAndFeel;
    TugMidiSeqAudioProcessor& audioProcessor;
    Component  associatedComponent;
    CustomRoratySlider gridAllNumberSlider;
    CustomRoratySlider gridAllVelSlider;
    CustomRoratySlider gridAllEventSlider;
    juce::ComboBox gridAllSpeedCombo;
    juce::ComboBox gridAllDurationCombo;
    juce::ComboBox presetCombo;
    
    CustomRoratySlider gridAllShuffleSlider;
    
    CustomRoratySlider loopBarlenghtSlider;
    Label loopBarCounterLabel;
    TextButton resetButton;
    TextButton velUsageButton;
    TextButton inBuiltSynthButton;
    TextButton sortedOrFirstEmptySelectButton;
    
    Label globalNameLabel;
    Label loopBarlenghtSliderLabel;
    Label ShuffleNameLabel;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> loopBarlenghtSliderAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> velUsageButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> inBultSynthButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::ButtonAttachment> sortedOrFixedButtonAttachment;
    std::unique_ptr  <AudioProcessorValueTreeState::SliderAttachment> gridAllShuffleSliderAttachment;
    
    
    TextButton writeButton;
    TextButton deleteButton;
    
    juce::OwnedArray<juce::TextButton> randomButton;
    int preset_index_sil = 0;
    AlertWindow *pwdDialog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};




 
  
