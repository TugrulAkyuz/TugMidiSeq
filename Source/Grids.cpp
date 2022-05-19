/*
  ==============================================================================

    Grids.cpp
    Created: 13 May 2022 3:35:51pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/


#include "Grids.h"



Grids::Grids(TugMidiSeqAudioProcessor& p,int line)  : audioProcessor (p)
{
    myLine = line;
    step = 0;
    addAndMakeVisible(gridNumberSlider);
    addAndMakeVisible(gridSpeedCombo);
    addAndMakeVisible(gridDurationCombo);
    gridNumberSlider.setSliderStyle(juce::Slider::Rotary);
    gridNumberSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridNumberSlider.setValue(16);
    addAndMakeVisible(gridVelSlider);
    gridVelSlider.setSliderStyle(juce::Slider::Rotary);
    gridVelSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridVelSlider.setValue(90);
    juce::String  tmp_s;
    for (int i = 0; i < numOfStep; ++i)
    {
        addAndMakeVisible (buttons.add (new juce::TextButton ()));
        buttons.getLast()->setClickingTogglesState(true);
        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::orange);
        buttons.getLast()->setConnectedEdges(30);
        buttons.getLast()->onClick = [this]()
        {
            int i = 0;
            for(juce::TextButton * b : buttons)
            {
                stepArray[i] = (b->getToggleState());
                i++;
            }
        };
        tmp_s.clear();
        tmp_s << "block" << line << i;
        juce::TextButton *  st = buttons.getLast();
        buttonAttachmentArray.add (new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.valueTreeState, tmp_s,*st));

   
    }
    tmp_s.clear();
    tmp_s << "Speed"<<line;
    comBoxSpeedAtaachment = std::make_unique <AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.valueTreeState, tmp_s, gridSpeedCombo);
    tmp_s.clear();
    tmp_s << "Dur"<<line;
    comBoxDurationAtaachment =  std::make_unique <AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.valueTreeState, tmp_s, gridDurationCombo);
    gridNumberSlider.onValueChange = [this]()
    {
        resized();
    };
    gridVelSlider.onValueChange = [this]()
    {
//         step = gridVelSlider.getValue() -1;
//        resized();
    };
    startTimer(20);
    gridNumberSlider.setRange(1, numOfStep,1);
    gridVelSlider.setRange(1, 127,1);
    
    
    int i= 1;
    for(auto s: myNotetUnit)
    {
        gridSpeedCombo.addItem(s,i);
        i++;
    }
    gridSpeedCombo.setSelectedId(14);
     i= 1;
    for(auto s: myNotetUnit)
    {
        gridDurationCombo.addItem(s,i);
        i++;
    }
    gridDurationCombo.setSelectedId(14);
    gridSpeedCombo.onChange = [this]
    {
        auto x = gridSpeedCombo.getSelectedId();
 
        audioProcessor.setSpeedofLine(x, myLine);
        
    };
    gridDurationCombo.onChange = [this]
    {
        auto x = gridDurationCombo.getSelectedId();
 
        audioProcessor.setDurationofLine(x, myLine);
        
    };
}
void Grids::paint (juce::Graphics& g)
{
   
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}
void Grids::resized()
{
    int marjin =  1;
    auto x=90;
    auto area = getLocalBounds();
   
    gridVelSlider.setBounds( area.removeFromRight(50));
    gridSpeedCombo.setBounds(area.removeFromRight(90).reduced(5)/*.withHeight(area.getHeight()-10)*/);
    gridDurationCombo.setBounds(area.removeFromRight(90).reduced(5)/*.withHeight(area.getHeight()-)*/);
    gridNumberSlider.setBounds( area.removeFromRight(50)/*.withHeight(area.getHeight()+5)*/);
 

    auto griidbounds =  area;
    juce::FlexBox fb;
   fb.flexWrap = juce::FlexBox::Wrap::wrap;

    fb.alignContent = juce::FlexBox::AlignContent::center;

    fb.flexWrap= juce::FlexBox::Wrap::noWrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
 
    int n = gridNumberSlider.getValue() ;
    auto w = (griidbounds.toFloat().getWidth()   /(n)) ;
    for ( auto *b : buttons) b->setVisible(false);
    for ( int i = 0; i < n;i++)
    {// [5]
        fb.items.add (juce::FlexItem (*buttons[i]).withMinWidth (w-2*marjin).withMinHeight ((float) griidbounds.getHeight() -2 ).withMargin(marjin));
        buttons[i]->setVisible(true);
        
        buttons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
        buttons[i]->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::orange);
        buttons[i]->setButtonText("");
    }
     
    
    fb.performLayout (griidbounds.toFloat());

    buttons[step]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green.withAlpha(0.90f));
    buttons[step]->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::yellow.withAlpha(0.90f));
    buttons[step]->setButtonText("->");
    buttons[step]->setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    buttons[step]->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
}

