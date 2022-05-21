/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TugMidiSeqAudioProcessorEditor::TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) ,  satellite(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    //setResizable (true, true);
    addAndMakeVisible(satellite);
    for(auto i =  0; i < 5 ; i++ )
    {
        auto g = new Grids(audioProcessor,i);
        grids.add(g);
        addAndMakeVisible(g);
    }

 
   // gridSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
//    for (int i = 0; i < 20; ++i)
//    {
//        addAndMakeVisible (buttons.add (new juce::TextButton (juce::String (i))));
//        buttons.getLast()->setClickingTogglesState(true);
//        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
//        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::orange);
//
//    }
//    gridSlider.onValueChange = [this]()
//    {
//        resized();
//    };
    
    setSize (600, 400);
}

TugMidiSeqAudioProcessorEditor::~TugMidiSeqAudioProcessorEditor()
{
}

//==============================================================================
void TugMidiSeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

 
}

void TugMidiSeqAudioProcessorEditor::resized()
{
////    auto bounds = getLocalBounds();
////    auto buttonSize = fmin(bounds.getWidth() / 3,bounds.getHeight() / 3);
////
////    for (int i = 0; i < buttons.size(); ++i)
////    {
////        buttons[i]->setBounds (buttonSize * (i % 3),
////                               buttonSize * (i / 3) + bounds.getHeight() / 3,
////                               buttonSize,
////                               buttonSize);
////    }
//    juce::FlexBox fb;                                               // [1]
//   fb.flexWrap = juce::FlexBox::Wrap::wrap;                        // [2]
//    fb.justifyContent = juce::FlexBox::JustifyContent::center;      // [3]
//    fb.alignContent = juce::FlexBox::AlignContent::center;          // [4]
//    //fb.flexDirection = juce::FlexBox::Direction::column;
//   //fb.alignItems = juce::FlexBox::AlignItems::stretch;
//    //fb.alignContent = juce::FlexBox::AlignContent::stretch;
//    fb.flexWrap= juce::FlexBox::Wrap::noWrap;
//    fb.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
//    fb.justifyContent = juce::FlexBox::JustifyContent::center ;
//    int n = gridSlider.getValue() + 1;
//    auto w = getLocalBounds().toFloat().getWidth()   /(n);
//    for ( auto *b : buttons) b->setVisible(false);
//    for ( int i = 0; i < n;i++)
//    {// [5]
//        fb.items.add (juce::FlexItem (*buttons[i]).withMinWidth (w).withMinHeight (50.0f));
//        buttons[i]->setVisible(true);
//        buttons[i]->addListener(this);
//    }
//    fb.performLayout (getLocalBounds().toFloat());
////    juce::Grid grid;
////
////            using Track = juce::Grid::TrackInfo;
////            using Fr = juce::Grid::Fr;
////
////            grid.templateRows    = { Track (Fr (1)) };
////            grid.templateColumns = { Track (Fr (1)), Track (Fr (2)), Track (Fr (1)) };
////
////            grid.items = {juce::GridItem()};
////            //grid.items = { juce::GridItem (buttons)};
////
////            grid.performLayout (getLocalBounds());
////    int n = gridSlider.getValue() + 1;
////    for ( int i = 0; i < 20;i++)
////    {
////        buttons[i]->setVisible(false);
////    }
////
////    for ( int i = 0; i < n;i++)
////    {
////
////        int w = (getLocalBounds().getWidth()/n);
////        int x = i*(getLocalBounds().getWidth()/n);
////        buttons[i]->setBounds(x, getLocalBounds().getHeight()/2, w, 50);
////        buttons[i]->setVisible(true);
////    }
    //gridSlider.setBounds(0,0,100,100);
    auto allarea = getLocalBounds();
    auto area = allarea.removeFromTop(200);
    auto h = area.getHeight()/5;
    for(auto g :grids )
    {
        g->setBounds(area.removeFromTop (h));
        
    }
    area = allarea.removeFromRight(allarea.getHeight());
    satellite.setBounds(area);
   
}

//void TugMidiSeqAudioProcessorEditor::buttonClicked(juce::Button *b)
//{
//   // DBG((b->getToggleStateValue());
////
////    if(b->getToggleStateValue() == 0)
////       b->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::aliceblue);
////    else  b->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::azure);
//}
