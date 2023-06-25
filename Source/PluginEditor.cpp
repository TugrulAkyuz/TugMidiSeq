/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TugMidiSeqAudioProcessorEditor::TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) ,  satellite(p) , globalPanel(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //setResizable (true, true);
    addAndMakeVisible(satellite);
    for(auto i =  4; i >= 0 ; i-- )
    {
        auto g = new Grids(audioProcessor,i);
        grids.add(g);
        addAndMakeVisible(g);
    }
    addAndMakeVisible(globalPanel);
 
    
    for(auto i =  0; i < topLabel.size() ; i++ )
    {
        
        topInLabel.add(new juce::Label(topLabel.at(i),topLabel.at(i)));
        topInLabel.getLast()->setColour(Label::textColourId, myTextLabelColour);
        topInLabel.getLast()->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(topInLabel.getLast());
    }
    for(auto i =  0; i < 5; i++ )
        globalPanel.setGridComp(grids[i],i);
    setSize (1050, 276);
}

TugMidiSeqAudioProcessorEditor::~TugMidiSeqAudioProcessorEditor()
{
}

//==============================================================================
void TugMidiSeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (  Colour(0xff303030));
    auto allarea = getLocalBounds();
    g.setColour(Colours::yellow.withAlpha(0.2f));
    g.drawRect(allarea,3.0f);
 
}

void TugMidiSeqAudioProcessorEditor::resized()
{

    auto allarea = getLocalBounds();
    allarea.reduce(3, 3);
    auto label_area = allarea.removeFromTop(20);
    label_area.removeFromLeft(26);
    //topInLabel[1]->setBounds(label_area.removeFromRight(400));
    topInLabel[0]->setBounds(label_area.removeFromLeft(50));
    topInLabel[1]->setBounds(label_area.removeFromLeft(60));
    topInLabel[2]->setBounds(label_area.removeFromLeft(415));
    topInLabel[3]->setBounds(label_area.removeFromLeft(50));
    topInLabel[4]->setBounds(label_area.removeFromLeft(70));
    topInLabel[5]->setBounds(label_area.removeFromLeft(70));
    topInLabel[6]->setBounds(label_area.removeFromLeft(60));
    topInLabel[7]->setBounds(label_area);
 

    auto area = allarea.removeFromTop(200);
    auto satelite_area = area.removeFromRight(200);
    auto h = area.getHeight()/5;
    for(auto g :grids )
    {
        g->setBounds(area.removeFromTop (h));
        
    }
    satellite.setBounds(satelite_area);
    //area = allarea.removeFromRight(allarea.getHeight());
    globalPanel.setBounds(allarea);
}

