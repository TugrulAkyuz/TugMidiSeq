/*
  ==============================================================================

    GlobalPanel.cpp
    Created: 23 May 2022 9:19:45pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#include "GlobalPanel.h"

GlobalPanel::GlobalPanel(TugMidiSeqAudioProcessor& p): audioProcessor (p)
{
    addAndMakeVisible(gridAllNumberSlider);
    addAndMakeVisible(gridAllVelSlider);
    addAndMakeVisible(gridAllSpeedCombo);
    addAndMakeVisible(gridAllDurationCombo);
    gridAllNumberSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllVelSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    
    int i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllSpeedCombo.addItem(s,i);
        i++;
    }
    gridAllSpeedCombo.setSelectedId(14);
     i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllDurationCombo.addItem(s,i);
        i++;
    }
    gridAllDurationCombo.setSelectedId(14);
    
    gridAllSpeedCombo.setLookAndFeel(&myLookAndFeel);
    gridAllSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
    gridAllDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridAllDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
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
    
}
