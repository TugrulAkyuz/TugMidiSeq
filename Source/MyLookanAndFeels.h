/*
  ==============================================================================

    MyLookanAndFeels.h
    Created: 17 May 2022 11:18:30am
    Author:  Tuğrul Akyüz

  ==============================================================================
*/


#include <JuceHeader.h>

#pragma once
using namespace juce;

class MyLookAndFeel : public juce::LookAndFeel_V4
{
private:
    void drawRotarySlider(juce::Graphics & g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider & slider) override
    {
    
        auto outline = juce::Colours::grey;
        auto fill    = slider.findColour (juce::Slider::rotarySliderFillColourId);
        
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (5);
        
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin (2.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;
        
    
        
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(),
                                     bounds.getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);
        
        g.setColour (outline);
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    toAngle,
                                    true);
            
            g.setColour (fill);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        
        auto thumbWidth = lineW * 2.0f;
        juce::Point<float> thumbPoint (bounds.getCentreX() + (arcRadius - 1) * std::cos (toAngle - juce::MathConstants<float>::halfPi),
                                       bounds.getCentreY() + (arcRadius - 1) * std::sin (toAngle - juce::MathConstants<float>::halfPi));
        
        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        // g.fillEllipse (juce::Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
        g.drawLine(backgroundArc.getBounds().getCentreX(), backgroundArc.getBounds().getCentreY(), thumbPoint.getX(), thumbPoint.getY(),lineW);
        
        
    }
    Font getComboBoxFont (ComboBox& box) override
    {
        //return getCommonMenuFont();
        return { jmin (12.0f, (float) box.getHeight() * 0.85f) };
    }
    
    Font getPopupMenuFont() override
    {
        return Font (12.0f);
        //return getCommonMenuFont();
        //return { jmin (16.0f, (float) box.getHeight() * 0.85f) };
    }
    void drawComboBox (Graphics& g, int width, int height, bool,
                                       int, int, int, int, ComboBox& box) override
    {
        auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        Rectangle<int> boxBounds (0, 0, width, height);

        g.setColour (box.findColour (ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

        g.setColour (juce::Colours::orange);
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        Rectangle<int> arrowZone (width - 15, 0, 15, height);
        Path path;
        path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
        path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

        g.setColour (juce::Colours::orange);
        g.strokePath (path, PathStrokeType (2.0f));
    }
    void  positionComboBoxText (ComboBox& box, Label& label) override
    {
        label.setBounds (1, 1,
                         box.getWidth() - 20,
                         box.getHeight() - 2);

        label.setFont (getComboBoxFont (box));
    }

private:
    Font getCommonMenuFont()
    {
        return Font ("Times", "Regular", 10.f);
    }
    
};
class CustomRoratySlider :public juce::Slider{
public:
    CustomRoratySlider()
    {
        
        setLookAndFeel(&myLookAndFeel);
        //        auto rotaryparams = getRotaryParameters();
        //        rotaryparams.startAngleRadians = juce::MathConstants<float>::pi *1.5;
        //        rotaryparams.endAngleRadians = juce::MathConstants<float>::pi *3.0;
        //        setRotaryParameters(rotaryparams);
        setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        //
        setRange(0.0f, 100.0f,1);
        setTextBoxStyle(juce::Slider::TextBoxAbove, true, 120, 20);
        
        //
        setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::orange);
        setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::orange);
        setColour(Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        

        
    }
    ~CustomRoratySlider(){
        setLookAndFeel(nullptr);
        
    }
private:
    //juce:
    
    void paint(juce::Graphics & g) override
    {
        auto sBounds = getLocalBounds();
        getLookAndFeel().drawRotarySlider(g,
                                          sBounds.getX(),
                                          sBounds.getY(),
                                          sBounds.getWidth(),
                                          sBounds.getHeight(),
                                          juce::jmap(getValue(),getRange().getStart(),getRange().getEnd(),0.0,1.0),
                                          juce::MathConstants<float>::pi *1.5,
                                          juce::MathConstants<float>::pi *3.0,
                                          *this);
        
        g.setFont(11);
       g.drawFittedText(juce::String(getRange().getStart()), sBounds, juce::Justification::centredLeft, 0.1);
       g.drawFittedText(juce::String(getRange().getEnd()), sBounds.removeFromLeft(sBounds.getWidth()/2), juce::Justification::bottomRight, 0.1);
        g.drawFittedText(juce::String(getValue()), sBounds, juce::Justification::topRight, 0.1);
        
        
        
    }
    MyLookAndFeel myLookAndFeel;
    
    
};
