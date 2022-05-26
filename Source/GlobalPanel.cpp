/*
  ==============================================================================

    GlobalPanel.cpp
    Created: 23 May 2022 9:19:45pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#include "GlobalPanel.h"
#include "Satellite.h"


GlobalPanel::GlobalPanel(TugMidiSeqAudioProcessor& p ): audioProcessor (p)
{
    startTimer(100);
    
    addAndMakeVisible(writeButton);
    addAndMakeVisible(deleteButton);
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.add(new TextButton());
        addAndMakeVisible(randomButton.getLast()); 
        randomButton.getLast()->setLookAndFeel(&myLookAndFeel);
        randomButton.getLast()->setButtonText("Rnd "+std::to_string(5-i));
        randomButton.getLast()->setColour(TextButton::ColourIds::textColourOffId, colourarray[4-i]);
        
 
    }
    loopBarlenghtSliderLabel.setFont (juce::Font (12, juce::Font::italic));
    globalNameLabel.setText("GLOBAL CONTROLS", juce::dontSendNotification);
    globalNameLabel.setColour(juce::Label::ColourIds::textColourId, Colours::lightgrey);
    
    addAndMakeVisible(globalNameLabel);
    loopBarlenghtSliderLabel.setText("RESYNC BAR", juce::dontSendNotification);
    loopBarlenghtSliderLabel.setColour(juce::Label::ColourIds::textColourId, Colours::lightgrey);
    
  
    addAndMakeVisible(loopBarlenghtSliderLabel);
    
    presetCombo.setLookAndFeel(&myLookAndFeel);
    addAndMakeVisible(presetCombo);
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
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxOutlineColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::textBoxBackgroundColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Label::textWhenEditingColourId, Colours::orange);
 
    loopBarlenghtSlider.setVelocityBasedMode (true);
    loopBarlenghtSlider.setVelocityModeParameters (0.4, 1, 0.09, false);
    loopBarlenghtSlider.setRange(1, 32,1);
    velUsageButton.onClick = [this](){
       
        for(auto i = 0; i < 5 ;i++)
        if(otherg[i] == 0) return;
        if(velUsageButton.getToggleStateValue() == false)
        {
            for(auto i = 0; i < 5 ;i++)
               otherg[i]->setEnable(true);
            velUsageButton.setButtonText("Fixed Vel");
        }
        else
        {
            for(auto i = 0; i < 5 ;i++)
                otherg[i]->setEnable(false);
            velUsageButton.setButtonText("In Vel");
        }
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
    
    tmp_s.clear();
    tmp_s << "GlobalInOrFixedVel";
    velUsageButtonAttachment = std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.valueTreeState, tmp_s, velUsageButton);
    
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
    resetButton.onClick = [this]
    {
        audioProcessor.resetAllParam();
    };

    writeButton.onClick = [this]
    {
        String s = "Preset";
        s << std::to_string(preset_index_sil);
        preset_index_sil++;
        audioProcessor.createPrograms(s);
    };
    int k =  audioProcessor.getNumPrograms();
    if(k == 1 ) return;
    for(auto i = 0  ; i < k ; i++ )
    {
       String s  = audioProcessor.getProgramName(i + 1);
        
        presetCombo.addItem(s, i + 1);
    }
    presetCombo.onChange = [this]
    {
        auto x = presetCombo.getNumItems();
        if(presetCombo.getNumItems() == 0) return;
        x = presetCombo.getSelectedId();
        if(x == 0) return;
        audioProcessor.setCurrentProgram(x);
        
    };
    deleteButton.onClick = [&]
    {
        new MainWindow("");
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
    gridAllDurationCombo.setBounds(area.removeFromRight(70).reduced(2,10));
    gridAllSpeedCombo.setBounds(area.removeFromRight(70).reduced(2,10));
    gridAllNumberSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    
    auto ra = area.removeFromRight(80);
    auto ar = ra.removeFromTop(12);
    ra.removeFromBottom(10);
    loopBarCounterLabel.setBounds( ra.removeFromRight(40).reduced(3, 5));
    loopBarlenghtSlider.setBounds( ra.removeFromRight(40).reduced(3, 5));
    
    //rightarea.reduce(5, 0);
    loopBarlenghtSliderLabel.setBounds(ar );

    velUsageButton.setBounds( area.removeFromRight(70).reduced(3, 10));

    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.getUnchecked(i)->setBounds( area.removeFromRight(60).reduced(3, 14));
    }
    globalNameLabel.setBounds(area.removeFromLeft(100).reduced(3, 14));
    
    resetButton.setBounds( rightarea.removeFromRight(60).reduced(3, 10));
    presetCombo.setBounds( rightarea.removeFromRight(70).reduced(3, 10));
 
    writeButton.setBounds( rightarea.removeFromTop(25).reduced(3, 5));
    deleteButton.setBounds( rightarea.reduced(3, 5));

    
}
