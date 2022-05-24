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
    startTimer(33);
}
void Satellite::paint (juce::Graphics& g)
{
   
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);

    auto area = getLocalBounds();
 
    g.setColour(juce::Colours::darkgrey);
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
        juce::String tmp = std::to_string(i+1);
        if(audioProcessor.midiState[i] == false)
        {

            g.setColour(juce::Colours::lightgrey);
            g.fillEllipse(x-6, y-6, 12, 12);
            g.setColour(juce::Colours::black);
            g.drawText(tmp, x-5, y-5, 11, 11, juce::Justification::centred);
            
        }
        else {
            g.setColour(colourarray[i]);
            juce::ColourGradient cg{colourarray[i], x, y, colourarray[i].withAlpha(0.0f), x+15, y + 15, true};
            g.setGradientFill(cg);
            g.fillEllipse(x-15, y-15, 30, 30);
        }
            
    }
    g.setColour(juce::Colours::grey);
    //getLocalBounds().toFloat().reduce(1, 1);
    auto x = getLocalBounds();
    x.reduce(2, 2);
    
     g.drawRoundedRectangle(x.toFloat(), 2.0f, 2.0f);
    
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
