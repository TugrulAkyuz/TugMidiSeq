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
        // lane-tinted orbit ring — clearer, and each ring reads as its lane
        g.setColour (colourarray[i].withAlpha (0.32f));
        g.drawEllipse(x, y, width, height, 1.4f);
        r[i] =  center_x - x; ;
    }
    for(int i = 0 ; i < 5 ; i++)
    {
    //  float  r = (5-i)*(area.getHeight()/2)/7 ;
        g.setColour(colourarray[i].withAlpha(0.8f));
        float arcangle = audioProcessor.getDurAngle(i);
        if (! std::isfinite (arcangle)) arcangle = 0.0f;
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

          // A step's real ratio is 1 +/- shuffle, so anything outside [0, 2] is a
          // value the audio thread hasn't recomputed yet (e.g. just after #Grid
          // was automated up).
          auto sr = audioProcessor.getSfuffleRatios(i,  j);
          if (! std::isfinite (sr)) sr = 0.0f;
          sr = jlimit (0.0f, 2.0f, sr);
          startplacediff =  startplacediff + (sr -1);
          if(*audioProcessor.gridsArr[i][j] == 0) continue;

          // addCentredArc steps from the start angle in pi/100 increments; at a
          // large float magnitude the increment is lost to rounding and the loop
          // never ends, growing the path until memory runs out. Keep it on [0, 2pi).
          if (! std::isfinite (angle)) continue;
          angle = std::fmod (angle, MathConstants<float>::twoPi);

          // Path::addCentredArc emits a line segment every pi/100 radians, so the
          // sweep must stay bounded. A long note duration over a very short step
          // (fast speed + long duration) makes getDurAngle() enormous, which would
          // add millions of points and exhaust memory. A ring cannot show more than
          // one full turn anyway.
          float sweep = sr * arcangle;
          if (! std::isfinite (sweep)) sweep = 0.0f;
          sweep = jlimit (-MathConstants<float>::twoPi, MathConstants<float>::twoPi, sweep);

          p.addCentredArc (center_x, center_y, r[i], r[i], 0.f, angle, angle + sweep, true);
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

        // On the arc of the step being played (arcs above are laid out by the
        // same shuffle ratios), moving the way the lane travels: clockwise, or
        // anticlockwise for Reverse / Ping-Pong's way back; Random jumps.
        float angle = 0.0f;
        const int n    = (int) *audioProcessor.numOfGrid[i];
        const int step = audioProcessor.getPlayheadStep (i);
        if (step >= 0 && step < n)
        {
            auto ratio = [&] (int k)
            {
                auto sr = audioProcessor.getSfuffleRatios (i, k);
                return std::isfinite (sr) ? jlimit (0.0f, 2.0f, sr) : 0.0f;
            };
            float start = 0.0f;
            for (int k = 0; k < step; k++) start += ratio (k);
            const float frac = audioProcessor.getPlayheadFraction (i);
            const float pos  = start + (audioProcessor.isPlayheadBackward (i) ? 1.0f - frac : frac) * ratio (step);
            angle = MathConstants<float>::twoPi * pos / (float) n;
        }
        float x =  center_x+r[i]*sin(angle);
        float y =  center_y-r[i]*cos(angle);
        juce::String tmp = std::to_string(i+1);
        if(audioProcessor.midiState[i] == false  || audioProcessor.myIsPlaying == false)
        {
            // idle orbiting marker — larger, crisper, full lane-colour ring
            g.setColour (Theme::surfaceAlt);
            g.fillEllipse (x - 7, y - 7, 14, 14);
            g.setColour (colourarray[i].withAlpha (0.9f));
            g.drawEllipse (x - 7, y - 7, 14, 14, 1.6f);
            g.setColour (Theme::textSecondary);
            g.setFont (Theme::valueFont (9.0f));
            g.drawText (tmp, x - 7, y - 7, 14, 14, juce::Justification::centred);
        }
        else {
            // active marker — strong bloom + hot white centre
            float R = 26.0f;
            juce::ColourGradient cg { colourarray[i].brighter (0.4f), x, y,
                                      colourarray[i].withAlpha (0.0f), x + R, y + R, true };
            cg.addColour (0.35, colourarray[i].withAlpha (0.85f));
            g.setGradientFill (cg);
            g.fillEllipse (x - R, y - R, 2 * R, 2 * R);

            g.setColour (colourarray[i].brighter (0.6f));
            g.fillEllipse (x - 6, y - 6, 12, 12);

            g.setColour (juce::Colours::white.withAlpha (0.95f));
            g.fillEllipse (x - 2.5f, y - 2.5f, 5, 5);
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
