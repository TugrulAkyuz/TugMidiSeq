/*
  ==============================================================================

    Theme.h
    Central visual language for TugMidiSeq.

    Design language: "Eurorack / hardware panel" — a warm graphite module with
    recessed wells, raised surfaces, silkscreen labels and a curated (non-rainbow)
    5-colour lane palette. Every colour / font / spacing decision lives here so
    the LookAndFeel and all components stay in one voice.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using namespace juce;

namespace Theme
{
    //== Backgrounds (warm graphite, layered by elevation) =====================
    const Colour panel       (0xff1a1b1d); // main editor plate
    const Colour surface     (0xff242528); // raised lane surface (A)
    const Colour surfaceAlt  (0xff1e1f22); // raised lane surface (B, alternating)
    const Colour section     (0xff202123); // global panel
    const Colour well         (0xff101113); // recessed area: grid bed, combo, value wells
    const Colour screen      (0xff0a0c0e); // satellite scope screen

    const Colour hairline    (0xff36383c); // subtle borders (replaces harsh grey lines)
    const Colour highlight   = Colours::white.withAlpha (0.05f);  // top bevel edge
    const Colour shadow      = Colours::black.withAlpha (0.38f);  // bottom bevel edge

    //== Text ==================================================================
    const Colour textPrimary  (0xffe9e4da); // warm off-white
    // Contrast on the lane surfaces: secondary ~7:1, dim ~4.6:1 (both readable
    // at the small sizes used), disabled ~2.8:1 (meant to read as "off").
    const Colour textSecondary(0xffb3aea4); // silkscreen labels
    const Colour textDim      (0xff908c84); // min/max, hints, captions
    const Colour textDisabled (0xff6a6760); // disabled controls, empty / unused things
    const Colour textValue    (0xffcfcabf); // numeric read-outs

    //== Accent (deep brass — the single interactive highlight) ================
    const Colour accent       (0xffca8f42);
    const Colour accentBright (0xffe6ad57);
    const Colour accentDeep   (0xff7d5a2a);

    //== Lane palette (curated, muted, harmonious — NOT primary rainbow) =======
    const Colour lane[5] =
    {
        Colour (0xffcf7b6d), // 1  coral / terracotta
        Colour (0xffa2b072), // 2  sage
        Colour (0xffe3c06b), // 3  honey
        Colour (0xffb08bc4), // 4  dusty mauve
        Colour (0xff6fa6c0)  // 5  steel blue
    };

    //== Geometry ==============================================================
    const float radSm  = 3.0f;  // grid pads
    const float radMd  = 5.0f;  // buttons, combos
    const float radLg  = 8.0f;  // panels / cards
    const float padXS  = 2.0f;
    const float padS   = 4.0f;
    const float padM   = 8.0f;
    const float padL   = 12.0f;

    //== Typography ============================================================
    // Silkscreen labels are uppercase + kerned; callers uppercase the string.
    inline Font labelFont (float h) { return Font (Font::getDefaultSansSerifFontName(), h, Font::plain).withExtraKerningFactor (0.14f); }
    inline Font titleFont (float h) { return Font (Font::getDefaultSansSerifFontName(), h, Font::bold ).withExtraKerningFactor (0.22f); }
    inline Font valueFont (float h) { return Font (Font::getDefaultMonospacedFontName(), h, Font::plain); }

    //== Draw helpers ==========================================================

    // Raised, physically-lit surface (lane rows, hardware buttons).
    inline void drawRaisedPanel (Graphics& g, Rectangle<float> b, float radius,
                                 Colour fill = surface)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (b, radius);

        g.setColour (highlight);                 // top bevel
        g.drawLine (b.getX() + radius, b.getY() + 0.75f,
                    b.getRight() - radius, b.getY() + 0.75f, 1.0f);

        g.setColour (shadow);                    // bottom bevel
        g.drawLine (b.getX() + radius, b.getBottom() - 0.75f,
                    b.getRight() - radius, b.getBottom() - 0.75f, 1.0f);

        g.setColour (hairline);
        g.drawRoundedRectangle (b.reduced (0.5f), radius, 1.0f);
    }

    // Recessed well (grid bed, combo interior, value read-outs).
    inline void drawRecessedWell (Graphics& g, Rectangle<float> b, float radius,
                                  Colour fill = well)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (b, radius);

        g.setColour (Colours::black.withAlpha (0.4f));   // inner top shadow
        g.drawLine (b.getX() + radius, b.getY() + 0.9f,
                    b.getRight() - radius, b.getY() + 0.9f, 1.2f);

        g.setColour (hairline.withAlpha (0.7f));
        g.drawRoundedRectangle (b.reduced (0.5f), radius, 1.0f);
    }

    // Panel-mount screw — sells the "hardware module" read at the corners.
    inline void drawScrew (Graphics& g, float cx, float cy, float r)
    {
        g.setColour (Colour (0xff2c2d30));
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
        g.setColour (Colours::black.withAlpha (0.55f));
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);
        g.setColour (highlight);
        g.drawEllipse (cx - r + 0.5f, cy - r + 0.5f, r * 2.0f - 1.0f, r * 2.0f - 1.0f, 0.75f);

        const float a = 0.7f, s = r * 0.62f;       // slot
        g.setColour (Colour (0xff0e0f11));
        g.drawLine (cx - s * std::cos (a), cy - s * std::sin (a),
                    cx + s * std::cos (a), cy + s * std::sin (a), 1.3f);
    }

    // Silkscreen section caption.
    inline void drawCaption (Graphics& g, const String& text, Rectangle<int> area,
                             Justification just = Justification::centredLeft,
                             Colour c = textSecondary, float h = 11.0f)
    {
        g.setColour (c);
        g.setFont (labelFont (h));
        g.drawText (text.toUpperCase(), area, just, false);
    }
}
