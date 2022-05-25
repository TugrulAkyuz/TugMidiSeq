/*
  ==============================================================================

    GlobalPanel.cpp
    Created: 23 May 2022 9:19:45pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#include "GlobalPanel.h"
#include "Satellite.h"

GlobalPanel::GlobalPanel(TugMidiSeqAudioProcessor& p): audioProcessor (p)
{
    startTimer(100);
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.add(new TextButton());
        addAndMakeVisible(randomButton.getLast()); 
        randomButton.getLast()->setLookAndFeel(&myLookAndFeel);
        randomButton.getLast()->setButtonText("Rnd "+std::to_string(5-i));
        randomButton.getLast()->setColour(TextButton::ColourIds::textColourOffId, colourarray[4-i]);
        
 
    }
   
    addAndMakeVisible(loopBarlenghtSlider);
    addAndMakeVisible( loopBarCounterLabel);
    addAndMakeVisible( resetButton);
    addAndMakeVisible( velUsageButton);
    velUsageButton.setClickingTogglesState (true);
    velUsageButton.setButtonText("Fixed Vel");
     
    addAndMakeVisible(gridAllNumberSlider);
    addAndMakeVisible(gridAllVelSlider);
    addAndMakeVisible(gridAllSpeedCombo);
    addAndMakeVisible(gridAllDurationCombo);
    gridAllNumberSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllVelSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllNumberSlider.setRange(1, numOfStep,1);
    gridAllVelSlider.setRange(1, 127,1);
    
    loopBarlenghtSlider.setSliderStyle (Slider::SliderStyle::LinearBarVertical);
    loopBarlenghtSlider.setColour (Slider::ColourIds::trackColourId, Colours::transparentWhite);
    loopBarlenghtSlider.setVelocityBasedMode (true);
    loopBarlenghtSlider.setVelocityModeParameters (0.4, 1, 0.09, false);
    loopBarlenghtSlider.setRange(1, 32,1);
    velUsageButton.onClick = [this](){
        if(velUsageButton.getToggleStateValue() == true)
            velUsageButton.setButtonText("Fixed Vel");
        else velUsageButton.setButtonText("In Vel");
    };
    resetButton.setLookAndFeel(&myLookAndFeel);
    velUsageButton.setLookAndFeel(&myLookAndFeel);
    resetButton.setButtonText("RESET");
    int i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllSpeedCombo.addItem(s,i);
        i++;
    }
 
     i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllDurationCombo.addItem(s,i);
        i++;
    }
 
    
    gridAllSpeedCombo.setLookAndFeel(&myLookAndFeel);
    gridAllSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
    gridAllDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridAllDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
    
    juce::String tmp_s;
    tmp_s << "GlobalRestncBar";
    loopBarlenghtSliderAttachment = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, loopBarlenghtSlider);
    
    gridAllSpeedCombo.onChange = [this]
    {
        audioProcessor.setAllValue("Speed",gridAllSpeedCombo.getSelectedItemIndex());
    };
    gridAllNumberSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue("GridNum",gridAllNumberSlider.getValue());
    };
    gridAllDurationCombo.onChange = [this]
    {
        audioProcessor.setAllValue("Dur",gridAllDurationCombo.getSelectedItemIndex());
    };
    gridAllVelSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue("Vel",gridAllVelSlider.getValue());
    };
 
    for(auto r : randomButton)
    {
      
        r->onClick = [this,r]
        {
            randomButton.indexOf(r);
            audioProcessor.randomizeGrids(4 -randomButton.indexOf(r));
        };
    };
    
   
}
void GlobalPanel::paint (juce::Graphics& g)
{
    g.fillAll (  Colour(0xff303030));
 
}
void GlobalPanel::resized()
{
//    auto area = getLocalBounds().reduced(5, 5);
//    auto w = getLocalBounds().getHeight();
//    //area.removeFromLeft(w);
//    gridAllNumberSlider.setBounds(area.removeFromLeft(w));
//    gridAllVelSlider.setBounds(area.removeFromLeft(w));
//    gridAllSpeedCombo.setBounds(area.removeFromLeft(2*w).reduced(5, 5));
//    gridAllDurationCombo.setBounds(area.removeFromLeft(2*w).reduced(5, 5));
    auto area = getLocalBounds();
    auto rightarea = area.removeFromRight(200);
    gridAllVelSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    gridAllDurationCombo.setBounds(area.removeFromRight(70).reduced(2,10)/*.withHeight(area.getHeight()-10)*/);
    gridAllSpeedCombo.setBounds(area.removeFromRight(70).reduced(2,10)/*.withHeight(area.getHeight()-)*/);
    gridAllNumberSlider.setBounds( area.removeFromRight(50).reduced(3, 5)/*.withHeight(area.getHeight()+5)*/);
    
    loopBarCounterLabel.setBounds( area.removeFromRight(30).reduced(3, 10)/*.withHeight(area.getHeight()+5)*/);
    
    loopBarlenghtSlider.setBounds( area.removeFromRight(30).reduced(3, 10)/*.withHeight(area.getHeight()+5)*/);
    

    resetButton.setBounds( area.removeFromRight(70).reduced(3, 10)/*.withHeight(area.getHeight()+5)*/);
    
    velUsageButton.setBounds( area.removeFromRight(70).reduced(3, 10)/*.withHeight(area.getHeight()+5)*/);
 
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.getUnchecked(i)->setBounds( area.removeFromRight(70).reduced(3, 14));
    }
    
}
