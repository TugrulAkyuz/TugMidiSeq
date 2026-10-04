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

// Note-length values ("1nd" / "16n" / "8nt": dotted, straight, triplet) shown as
// a note glyph plus a fraction ("1/1." / "1/16" / "1/8t"), the way TugPhonon
// draws them. Only the drawing changes: the item strings — and so the Speed /
// Dur parameters and the presets that store their index — stay as they are.
namespace NoteValue
{
    struct Parsed
    {
        int  div = 0;   // 1, 2, 4 ... 128
        bool dotted = false, triplet = false;
        bool valid() const { return div > 0; }
    };

    inline Parsed parse (const String& text)
    {
        Parsed p;
        auto t = text.trim();
        if      (t.endsWith ("nd")) { p.dotted  = true; t = t.dropLastCharacters (2); }
        else if (t.endsWith ("nt")) { p.triplet = true; t = t.dropLastCharacters (2); }
        else if (t.endsWith ("n"))  {                   t = t.dropLastCharacters (1); }
        else return {};
        if (t.isEmpty() || ! t.containsOnly ("0123456789")) return {};
        const int d = t.getIntValue();
        if (d < 1 || d > 128 || ! isPowerOfTwo (d)) return {};
        p.div = d;
        return p;
    }

    inline String fraction (Parsed p)
    {
        return "1/" + String (p.div) + (p.dotted ? "." : p.triplet ? "t" : "");
    }

    // a combo whose every item is a note value (Speed / Dur)
    inline bool isNoteValueBox (const ComboBox& box)
    {
        if (box.getNumItems() == 0) return false;
        for (int i = 0; i < box.getNumItems(); i++)
            if (! parse (box.getItemText (i)).valid()) return false;
        return true;
    }

    inline Font font (float height) { return Font (FontOptions (height)); }

    // width drawGlyph() uses for a glyph of the given height
    inline float glyphWidth (float noteH) { return noteH * 0.70f; }

    // Note head at the bottom-left, stem rising from its right edge, one flag per
    // halving below a quarter (8th = 1 ... 128th = 5), an augmentation dot for
    // dotted values and a small "3" over the head for triplets.
    inline void drawGlyph (Graphics& g, Rectangle<float> area, Parsed p, Colour c)
    {
        const float noteH = area.getHeight();
        const float headW = noteH * 0.40f, headH = noteH * 0.29f;
        const float headCX = area.getX() + headW * 0.55f;
        const float headCY = area.getBottom() - headH * 0.5f - 0.5f;
        const float stemX  = headCX + headW * 0.42f;
        const float stemTop = area.getY() + 0.5f;
        const float stemBot = headCY - headH * 0.15f;

        g.setColour (c);
        Path head;
        head.addEllipse (headCX - headW * 0.5f, headCY - headH * 0.5f, headW, headH);
        head.applyTransform (AffineTransform::rotation (-0.35f, headCX, headCY));

        if (p.div <= 2)
            g.strokePath (head, PathStrokeType (1.2f));     // whole / half: open head
        else
            g.fillPath (head);

        if (p.div >= 2)
            g.drawLine (stemX, stemBot, stemX, stemTop, 1.1f);

        int flags = 0;
        for (int d = p.div; d >= 8; d /= 2) flags++;
        const float spacing = noteH * 0.15f, reach = headW * 0.75f;
        for (int f = 0; f < flags; f++)
        {
            const float fy = stemTop + (float) f * spacing;
            Path flag;
            flag.startNewSubPath (stemX, fy);
            flag.quadraticTo (stemX + reach * 0.9f, fy + spacing * 0.4f, stemX + reach, fy + spacing * 1.3f);
            g.strokePath (flag, PathStrokeType (1.1f));
        }

        if (p.dotted)
        {
            const float r = jmax (1.1f, noteH * 0.075f);
            g.fillEllipse (headCX + headW * 0.78f - r, headCY - headH * 0.55f - r, r * 2.0f, r * 2.0f);
        }

        if (p.triplet)
        {
            g.setFont (font (noteH * 0.46f).boldened());
            g.drawText ("3", Rectangle<float> (area.getX() - 1.0f, area.getY() - 1.0f,
                                               stemX - area.getX(), noteH * 0.5f),
                        Justification::centred, false);
        }
    }

    // glyph + fraction in `area` (centred, or left-aligned so a column of them
    // lines up); the font shrinks if the pair doesn't fit
    inline void drawValue (Graphics& g, Rectangle<float> area, Parsed p, Colour c,
                           float noteH, float fontH, bool centred = true)
    {
        const auto text = fraction (p);
        const float gap = 3.0f, gw = glyphWidth (noteH);
        float textW = GlyphArrangement::getStringWidth (font (fontH), text);
        while (gw + gap + textW > area.getWidth() && fontH > 8.0f)
        {
            fontH -= 0.5f;
            textW = GlyphArrangement::getStringWidth (font (fontH), text);
        }
        const float x = centred ? jmax (area.getX(), area.getCentreX() - (gw + gap + textW) * 0.5f)
                                : area.getX();

        drawGlyph (g, { x, area.getCentreY() - noteH * 0.5f, gw, noteH }, p, c);
        g.setColour (c);
        g.setFont (font (fontH));
        g.drawText (text, Rectangle<float> (x + gw + gap, area.getY(), area.getRight() - (x + gw + gap), area.getHeight()),
                    Justification::centredLeft, false);
    }
}

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

        // note-value boxes: glyph + fraction (their label is hidden, see below)
        if (auto p = NoteValue::parse (box.getText()); p.valid() && NoteValue::isNoteValueBox (box))
        {
            auto c = box.findColour (ComboBox::textColourId);
            NoteValue::drawValue (g, Rectangle<float> (4.0f, 0.0f, (float) width - 20.0f, (float) height),
                                  p, box.isEnabled() ? c : c.withAlpha (0.4f),
                                  jmin (14.0f, height * 0.6f), jmin (11.5f, height * 0.5f));
        }
    }

    void positionComboBoxText (ComboBox& box, Label& label) override
    {
        label.setBounds (6, 1, box.getWidth() - 22, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
        // drawComboBox draws note values itself; ComboBox re-applies the label
        // colour on every look-and-feel / colour change and then calls this
        if (NoteValue::isNoteValueBox (box))
            label.setColour (Label::textColourId, Colours::transparentBlack);
    }

    // Popup rows for note values: glyph + fraction, the current one framed.
    void drawPopupMenuItem (Graphics& g, const Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                            bool hasSubMenu, const String& text, const String& shortcutKeyText,
                            const Drawable* icon, const Colour* textColour) override
    {
        auto p = NoteValue::parse (text);
        if (isSeparator || ! p.valid())
        {
            LookAndFeel_V4::drawPopupMenuItem (g, area, isSeparator, isActive, isHighlighted, isTicked,
                                               hasSubMenu, text, shortcutKeyText, icon, textColour);
            return;
        }

        auto r = area.toFloat().reduced (2.0f, 1.0f);
        auto c = textColour != nullptr ? *textColour : findColour (PopupMenu::textColourId);
        if (isHighlighted && isActive)
        {
            g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
            g.fillRoundedRectangle (r, Theme::radSm);
            c = findColour (PopupMenu::highlightedTextColourId);
        }
        else if (isTicked)
        {
            c = Theme::accentBright;
        }
        if (isTicked)
        {
            g.setColour (Theme::accentBright);
            g.drawRoundedRectangle (r.reduced (0.5f), Theme::radSm, 1.2f);
        }

        NoteValue::drawValue (g, r.reduced (10.0f, 0.0f), p,
                              isActive ? c : c.withAlpha (0.4f),
                              jmin (15.0f, r.getHeight() * 0.72f), 12.0f, false);
    }

    void getIdealPopupMenuItemSize (const String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override
    {
        if (! isSeparator && NoteValue::parse (text).valid())
        {
            idealWidth  = 78;
            idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : 22;
            return;
        }
        LookAndFeel_V4::getIdealPopupMenuItemSize (text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);
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

    // Note-value boxes open as a grid instead of a 24-row list: one column per
    // kind (dotted / straight / triplet), one row per length. Items keep their
    // ids, so the selection (and any parameter attachment) is unaffected.
    void showPopup() override
    {
        if (! NoteValue::isNoteValueBox (*this))
        {
            juce::ComboBox::showPopup();
            return;
        }

        static const char* headers[] = { "Dotted", "Straight", "Triplet" };
        const int selected = getSelectedId();
        juce::PopupMenu m;
        for (int col = 0; col < 3; col++)
        {
            if (col > 0) m.addColumnBreak();
            m.addSectionHeader (headers[col]);
            for (int i = 0; i < getNumItems(); i++)
            {
                auto p = NoteValue::parse (getItemText (i));
                const int kind = p.dotted ? 0 : p.triplet ? 2 : 1;
                if (kind == col)
                    m.addItem (getItemId (i), getItemText (i), true, getItemId (i) == selected);
            }
        }

        m.setLookAndFeel (&getLookAndFeel());
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withStandardItemHeight (22),
                         [safe = juce::Component::SafePointer<WheelComboBox> (this)] (int result)
                         {
                             if (safe == nullptr) return;
                             safe->hidePopup();   // clears ComboBox's "menu open" flag
                             if (result != 0)
                                 safe->setSelectedId (result);
                         });
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
    // the min / max captions under the knob; off for knobs too narrow for them
    void setShowRangeLabels (bool show) { showRangeLabels = show; repaint(); }

    // When set, a right-click calls this instead of starting a drag.
    std::function<void()> onRightClick;
    // When set, the read-out in the cap (otherwise the value as an integer).
    std::function<juce::String (double)> valueText;
    void mouseDown (const juce::MouseEvent& e) override
    {
        rightClicked = onRightClick != nullptr && e.mods.isRightButtonDown();
        if (rightClicked) { onRightClick(); return; }
        juce::Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override { if (! rightClicked) juce::Slider::mouseDrag (e); }
    void mouseUp (const juce::MouseEvent& e) override   { if (! rightClicked) juce::Slider::mouseUp (e); }
private:
    bool showRangeLabels = true;
    bool rightClicked = false;
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds();
        getLookAndFeel().drawRotarySlider (g,
                                           b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                                           valueToProportionOfLength (getValue()),   // follows a skewed range
                                           juce::MathConstants<float>::pi * 1.5f,
                                           juce::MathConstants<float>::pi * 3.0f,
                                           *this);

        // value read-out inside the knob cap
        g.setFont (Theme::valueFont (11.0f));
        g.setColour (isEnabled() ? Theme::textValue : Theme::textDim);
        g.drawText (valueText != nullptr ? valueText (getValue()) : juce::String (juce::roundToInt (getValue())),
                    b, juce::Justification::centred, false);

        if (! showRangeLabels)
            return;

        // min / max ticks, silkscreen-faint
        g.setFont (Theme::labelFont (8.0f));
        g.setColour (Theme::textDim);
        g.drawText (juce::String ((int) getRange().getStart()), b.reduced (3), juce::Justification::bottomLeft,  false);
        g.drawText (juce::String ((int) getRange().getEnd()),   b.reduced (3), juce::Justification::bottomRight, false);
    }

    MyLookAndFeel myLookAndFeel;
};
