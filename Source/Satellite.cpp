/*
  ==============================================================================

    stellite.cpp
    Created: 21 May 2022 2:45:47pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/

#include "Satellite.h"



Satellite::Satellite(TugMidiSeqAudioProcessor& p): audioProcessor (p)
{
    startTimer(33);
}
void Satellite::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // recessed scope screen with a hardware bezel
    g.fillAll (Theme::panel);
    auto screen = area.toFloat().reduced (3.0f);
    g.setColour (Theme::screen);
    g.fillRoundedRectangle (screen, Theme::radMd);
    g.setColour (Theme::shadow);                       // inner top shadow
    g.drawLine (screen.getX() + Theme::radMd, screen.getY() + 1.0f,
                screen.getRight() - Theme::radMd, screen.getY() + 1.0f, 1.4f);

    float center_x =  area.getHeight()/2;
    float center_y =  center_x;
    float r[5];

    // phosphor grid: crosshair + diagonals
    g.setColour (Theme::hairline.withAlpha (0.55f));
    g.drawLine (0, center_y, (float) getWidth(), center_y);
    g.drawLine (center_x, 0, center_x, (float) getHeight());
    g.setColour (Theme::hairline.withAlpha (0.28f));
    g.drawLine (0, 0, (float) getWidth(), (float) getWidth());
    g.drawLine (0, (float) getHeight(), (float) getWidth(), 0);
    for(int i = 0 ; i < 5 ; i++)
    {
        float x = i*(area.getHeight()/2)/7 +10;
        float y = x;
        float width = area.getHeight() - 2*x;
        float height = width;
        g.setColour (Theme::hairline.withAlpha (0.4f + i * 0.05f));
        g.drawEllipse(x, y, width, height, 1);
        r[i] =  center_x - x; ;
    }
    for(int i = 0 ; i < 5 ; i++)
    {
    //  float  r = (5-i)*(area.getHeight()/2)/7 ;
        g.setColour(colourarray[i].withAlpha(0.8f));
        float arcangle = audioProcessor.getDurAngle(i);
        float startplacediff = 0;
        int n = *audioProcessor.numOfGrid[i];
     for(int j = 0 ; j < n ; j++)
      {

          float alpha ;
          if(*audioProcessor.gridsArr[i][j] == 1) alpha = 0.99;
          else alpha = audioProcessor.getEventRandom(i);
          
          Path p;
          float delay = 0 ;//2.0*juce::double_Pi*audioProcessor.getDelayRatio(i);
          float angle = delay + 2.0*(j + startplacediff )*juce::double_Pi/(*audioProcessor.numOfGrid[i]);
          
          auto sr = audioProcessor.getSfuffleRatios(i,  j);
          startplacediff =  startplacediff + (sr -1);
          if(*audioProcessor.gridsArr[i][j] == 0) continue;

          p.addCentredArc (center_x, center_y, r[i], r[i], 0.f, angle, angle+sr*arcangle, true);
          g.setColour(colourarray[i].withAlpha(alpha));
          g.strokePath (p, juce::PathStrokeType (4.f));
          g.setColour(colourarray[i].withAlpha(0.8f));
          //g.fillEllipse(x-4, y-4, 8, 8);
          g.drawLine(center_x+(r[i]-5)*sin(angle), center_y-(r[i]-5)*cos(angle),center_x+(r[i]+5)*sin(angle), center_y-(r[i]+5)*cos(angle),4.0f);
         // g.fillEllipse(center_x + r[i]*cos(angle+arcangle -3.14/2)-4, center_y + r[i]*sin(angle+arcangle -3.14/2)-4, 8, 8);
     
         // g.drawEllipse(x-4, y-4, 8, 8);
      }
    }
 
    
    for(int i = 0 ; i < 5 ; i++)
    {
 
        if(audioProcessor.stepLoopResetInterval[i] == 0) return;
        float angle = audioProcessor.sampleNumber[i]*2.0*juce::double_Pi/(audioProcessor.stepLoopResetInterval[i]);
       // if (audioProcessor.myIsPlaying == false) angle = 0;
        float x =  center_x+r[i]*sin(angle);
        float y =  center_y-r[i]*cos(angle);
        juce::String tmp = std::to_string(i+1);
        if(audioProcessor.midiState[i] == false  || audioProcessor.myIsPlaying == false)
        {
            g.setColour (Theme::surfaceAlt);
            g.fillEllipse (x - 6, y - 6, 12, 12);
            g.setColour (colourarray[i].withAlpha (0.55f));
            g.drawEllipse (x - 6, y - 6, 12, 12, 1.0f);
            g.setColour (Theme::textSecondary);
            g.setFont (Theme::valueFont (9.0f));
            g.drawText (tmp, x - 6, y - 6, 12, 12, juce::Justification::centred);
        }
        else {
            juce::ColourGradient cg{colourarray[i], x, y, colourarray[i].withAlpha(0.0f), x+15, y + 15, true};
            g.setGradientFill(cg);
            g.fillEllipse(x-15, y-15, 30, 30);
            g.setColour (colourarray[i].brighter (0.4f));
            g.fillEllipse (x - 3, y - 3, 6, 6);
        }

    }
    // hardware bezel around the scope screen
    g.setColour (Theme::hairline);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (3.0f), Theme::radMd, 1.0f);


}
void Satellite::resized()
{
    
}
void Satellite::timerCallback()
{
    /*
    if (audioProcessor.myIsPlaying == false)
    {
        counter++;
        counter %= 20;
        if(counter != 0) return;
    }
     */
    repaint();

}
