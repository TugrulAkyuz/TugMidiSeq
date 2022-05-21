/*
  ==============================================================================

    stellite.cpp
    Created: 21 May 2022 2:45:47pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#include "Satellite.h"

juce::Colour colourarray [5] = {
    juce::Colours::red,
    juce::Colours::lightgreen,
    juce::Colours::lightblue,
    juce::Colours::yellow,
    juce::Colours::magenta
};

Satellite::Satellite(TugMidiSeqAudioProcessor& p): audioProcessor (p)
{
    startTimer(20);
}
void Satellite::paint (juce::Graphics& g)
{
   
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);
    g.setColour(juce::Colours::darkgrey);
    auto area = getLocalBounds();
 
    float center_x =  area.getHeight()/2;
    float center_y =  center_x;
    float r[5];
    for(int i = 0 ; i < 5 ; i++)
    {
        float x = i*(area.getHeight()/2)/7 +10;
        float y = x;
        float width = area.getHeight() - 2*x;
        float height = width;
        g.drawEllipse(x, y, width, height, 1);
        r[i] =  center_x - x; ;
    }
    for(int i = 0 ; i < 5 ; i++)
    {
    //  float  r = (5-i)*(area.getHeight()/2)/7 ;
        g.setColour(colourarray[i].withAlpha(0.8f));
     for(int j = 0 ; j < *audioProcessor.numOfGrid[i] ; j++)
      {
          if(*audioProcessor.gridsArr[i][j] == 0) continue;
          float angle = 2.0*j*juce::double_Pi/(*audioProcessor.numOfGrid[i]);
          
          float x =  center_x+r[i]*sin(angle);
          float y =  center_y-r[i]*cos(angle);

          g.fillEllipse(x-4, y-4, 8, 8);
     
         // g.drawEllipse(x-4, y-4, 8, 8);
      }
    }
    g.setColour(juce::Colours::grey);
    g.drawLine(0, center_y,getWidth(), center_y);
    g.drawLine(center_x, 0,center_x, getHeight());
    
    for(int i = 0 ; i < 5 ; i++)
    {
 
        if(audioProcessor.stepLoopResetInterval[i] == 0) return;
        float angle = audioProcessor.sampleNumber[i]*2.0*juce::double_Pi/(audioProcessor.stepLoopResetInterval[i]);
        float x =  center_x+r[i]*sin(angle);
        float y =  center_y-r[i]*cos(angle);
        if(audioProcessor.midiState[i] == false)
        {
             g.setColour(juce::Colours::grey);
            g.fillEllipse(x-4, y-4, 8, 8);
            
        }
        else {
            g.setColour(colourarray[i]);
            juce::ColourGradient cg{colourarray[i], x, y, colourarray[i].withAlpha(0.0f), x+14, y + 14, true};
            g.setGradientFill(cg);
            g.fillEllipse(x-10, y-10, 20, 20);
        }
            
    }

    
//    g.setColour(juce::Colours::white.withAlpha(0.6f));
//    auto xx = 4*audioProcessor.mySampleRate/(audioProcessor.myBpm/60);
//    auto angle = audioProcessor.measureSample*2.0*juce::double_Pi/xx;
//    g.drawLine(center_x, center_y, center_x+r[0]*sin(angle), center_y-r[0]*cos(angle));
//
//    float x = *audioProcessor.gridsArr[1][0];
//    x = 0;
    
   // *audioProcessor.steps[0]

}
void Satellite::resized()
{
    
}
void Satellite::timerCallback()
{
    repaint();

}
