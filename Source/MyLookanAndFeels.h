/*
  ==============================================================================

    MyLookanAndFeels.h
    Created: 17 May 2022 11:18:30am
    Author:  Tuğrul Akyüz

    Reworked to the "Eurorack / hardware panel" language — all values sourced
    from Theme.h.

  ==============================================================================
*/


#include <JuceHeader.h>
#include "Theme.h"
#pragma once
using namespace juce;

const Colour myTextLabelColour = Theme::textSecondary;

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void setdrawRotaryCenterd(bool centered)
    {
        myCenterdSlider = centered;
    }

private:

    Label* createSliderTextBox (Slider& slider) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (slider);
        l->setColour (Label::textColourId, Theme::textValue);
        l->setColour (Label::outlineColourId, Colours::transparentBlack);
        l->setColour (Label::outlineWhenEditingColourId, Theme::accent.withAlpha (0.6f));
        l->setFont (Theme::valueFont (12.0f));
        return l;
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds  = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
        auto radius  = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre  = bounds.getCentre();
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto valStart = myCenterdSlider ? (rotaryStartAngle + rotaryEndAngle) * 0.5f : rotaryStartAngle;

        auto lineW = juce::jmax (2.5f, radius * 0.16f);
        auto arcR  = radius - lineW * 0.5f;

        // recessed track
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Theme::hairline);
        g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // value arc
        if (slider.isEnabled())
        {
            juce::Path val;
            val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, valStart, toAngle, true);
            g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
            g.strokePath (val, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // physical knob cap + pointer
        auto capR = radius - lineW - 2.5f;
        if (capR > 2.5f)
        {
            juce::Rectangle<float> cap (capR * 2.0f, capR * 2.0f);
            cap.setCentre (centre);
            g.setColour (Theme::surfaceAlt);
            g.fillEllipse (cap);
            g.setColour (Theme::highlight);
            g.drawEllipse (cap.reduced (0.5f), 1.0f);
            g.setColour (Theme::shadow);
            g.drawEllipse (cap.reduced (0.75f), 0.75f);

            auto ca = std::cos (toAngle - juce::MathConstants<float>::halfPi);
            auto sa = std::sin (toAngle - juce::MathConstants<float>::halfPi);
            g.setColour (slider.isEnabled() ? Theme::textPrimary : Theme::textDim);
            g.drawLine (centre.x + capR * 0.4f * ca, centre.y + capR * 0.4f * sa,
                        centre.x + (capR - 1.5f) * ca, centre.y + (capR - 1.5f) * sa,
                        juce::jmax (1.6f, lineW * 0.7f));
        }
    }

    Font getComboBoxFont (ComboBox& box) override
    {
        return Theme::valueFont (jmin (12.0f, (float) box.getHeight() * 0.8f));
    }

    Font getPopupMenuFont() override
    {
        return Theme::labelFont (12.0f);
    }

    Font getTextButtonFont (TextButton&, int buttonHeight) override
    {
        return Theme::labelFont (jmin (13.0f, buttonHeight * 0.52f));
    }

    void drawComboBox (Graphics& g, int width, int height, bool,
                       int, int, int, int, ComboBox& box) override
    {
        auto b = Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
        Theme::drawRecessedWell (g, b, Theme::radMd);

        Rectangle<int> arrowZone (width - 15, 0, 13, height);
        auto cx = (float) arrowZone.getCentreX();
        auto cy = (float) arrowZone.getCentreY();
        Path path;
        path.startNewSubPath (cx - 3.0f, cy - 1.5f);
        path.lineTo          (cx,        cy + 2.5f);
        path.lineTo          (cx + 3.0f, cy - 1.5f);
        g.setColour (box.isEnabled() ? Theme::textSecondary : Theme::textDim);
        g.strokePath (path, PathStrokeType (1.4f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    void positionComboBoxText (ComboBox& box, Label& label) override
    {
        label.setBounds (6, 1, box.getWidth() - 22, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
    }

private:
    bool myCenterdSlider = false;

    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override
    {
        auto b = button.getLocalBounds().toFloat().reduced (0.5f);
        auto r = Theme::radMd;
        auto on = button.getToggleState();

        auto base = backgroundColour;
        if (shouldDrawButtonAsHighlighted) base = base.brighter (0.10f);
        if (shouldDrawButtonAsDown)        base = base.darker   (0.14f);

        g.setColour (base);
        g.fillRoundedRectangle (b, r);

        if (! shouldDrawButtonAsDown)
        {
            g.setColour (Theme::highlight);
            g.drawLine (b.getX() + r, b.getY() + 0.75f, b.getRight() - r, b.getY() + 0.75f, 1.0f);
            g.setColour (Theme::shadow);
            g.drawLine (b.getX() + r, b.getBottom() - 0.75f, b.getRight() - r, b.getBottom() - 0.75f, 1.0f);
        }

        g.setColour (on ? Theme::accentBright.withAlpha (0.9f) : Theme::hairline);
        g.drawRoundedRectangle (b, r, on ? 1.2f : 1.0f);
    }
};

// ComboBox that steps its selection one item per mouse-wheel notch.
//
// Per-notch delta magnitude is wildly platform/device dependent, so we don't
// gate on a delta threshold (that made slow / single notches do nothing).
// Instead: one step per wheel event by sign, with a short time window that
// merges a trackpad's momentum burst so a single notch = a single step.
class WheelComboBox : public juce::ComboBox
{
public:
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (! isEnabled() || getNumItems() == 0)
        {
            juce::ComboBox::mouseWheelMove (e, wheel);
            return;
        }

        float d = wheel.deltaY * (wheel.isReversed ? -1.0f : 1.0f);
        if (std::abs (d) < 1.0e-4f) return;               // ignore null events

        auto now = juce::Time::getMillisecondCounter();
        if (now - lastStepMs < stepIntervalMs) return;    // throttle momentum bursts
        lastStepMs = now;

        int dir  = d > 0.0f ? -1 : +1;                    // wheel up -> previous item
        int cur  = getSelectedItemIndex();
        int next = juce::jlimit (0, getNumItems() - 1, cur + dir);
        if (next != cur)
            setSelectedItemIndex (next, juce::sendNotificationSync);
    }
private:
    static constexpr juce::uint32 stepIntervalMs = 50;    // tunable feel
    juce::uint32 lastStepMs = 0;
};

class CustomRoratySlider : public juce::Slider
{
public:
    CustomRoratySlider()
    {
        setLookAndFeel (&myLookAndFeel);
        setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        setRange (0.0f, 100.0f, 1);
        setTextBoxStyle (juce::Slider::TextBoxAbove, true, 120, 20);
        setColour (juce::Slider::ColourIds::rotarySliderFillColourId, Theme::accent);
        setColour (juce::Slider::ColourIds::thumbColourId, Theme::textPrimary);
        setColour (Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }
    ~CustomRoratySlider()
    {
        setLookAndFeel (nullptr);
    }
private:
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        getLookAndFeel().drawRotarySlider (g,
                                           b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                                           juce::jmap (getValue(), getRange().getStart(), getRange().getEnd(), 0.0, 1.0),
                                           juce::MathConstants<float>::pi * 1.5f,
                                           juce::MathConstants<float>::pi * 3.0f,
                                           *this);

        // value read-out inside the knob cap
        g.setFont (Theme::valueFont (11.0f));
        g.setColour (isEnabled() ? Theme::textValue : Theme::textDim);
        g.drawText (juce::String ((int) getValue()), b, juce::Justification::centred, false);

        // min / max ticks, silkscreen-faint
        g.setFont (Theme::labelFont (8.0f));
        g.setColour (Theme::textDim);
        g.drawText (juce::String ((int) getRange().getStart()), b.reduced (3), juce::Justification::bottomLeft,  false);
        g.drawText (juce::String ((int) getRange().getEnd()),   b.reduced (3), juce::Justification::bottomRight, false);
    }

    MyLookAndFeel myLookAndFeel;
};
