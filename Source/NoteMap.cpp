/*
  ==============================================================================

    NoteMap.cpp

  ==============================================================================
*/

#include "NoteMap.h"

NoteMap::NoteMap (TugMidiSeqAudioProcessor& p) : audioProcessor (p)
{
    latchButton.setButtonText ("Latch");
    latchButton.setClickingTogglesState (true);
    latchButton.setColour (TextButton::textColourOffId, Theme::textSecondary);
    latchButton.setColour (TextButton::textColourOnId, Theme::screen);
    latchButton.setColour (TextButton::buttonColourId, Theme::surfaceAlt);
    latchButton.setColour (TextButton::buttonOnColourId, Theme::accent);
    latchButton.setColour (ComboBox::outlineColourId, Theme::hairline);
    addAndMakeVisible (latchButton);
    latchAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.valueTreeState, valueTreeNames[LATCH], latchButton);

    fillButton.setButtonText ("Fill");
    fillButton.setColour (TextButton::textColourOffId, Theme::textSecondary);
    fillButton.setColour (TextButton::textColourOnId, Theme::screen);
    fillButton.setColour (TextButton::buttonColourId, Theme::surfaceAlt);
    fillButton.setColour (TextButton::buttonOnColourId, Theme::accentBright);
    fillButton.setColour (ComboBox::outlineColourId, Theme::hairline);
    fillButton.setMouseCursor (juce::MouseCursor::NormalCursor);
    fillButton.onStateChange = [this] { setFill (fillButton.isDown()); };
    addAndMakeVisible (fillButton);

    for (auto* b : { &undoButton, &redoButton })
    {
        b->setColour (TextButton::textColourOffId, Theme::textSecondary);
        b->setColour (TextButton::buttonColourId, Theme::surfaceAlt);
        b->setColour (ComboBox::outlineColourId, Theme::hairline);
        b->setMouseCursor (juce::MouseCursor::NormalCursor);
        addAndMakeVisible (*b);
    }
    undoButton.setButtonText ("Undo");
    redoButton.setButtonText ("Redo");
    undoButton.onClick = [this] { audioProcessor.undo(); };
    redoButton.onClick = [this] { audioProcessor.redo(); };

    if (playable)
        setMouseCursor (juce::MouseCursor::PointingHandCursor);

    shown = takeSnapshot();
    startTimerHz (30);
}

NoteMap::~NoteMap()
{
    stopTimer();
    setFill (false);   // don't leave Fill stuck on if the editor closes mid-press
}

void NoteMap::setFill (bool on)
{
    if (on == fillHeld) return;
    fillHeld = on;
    if (auto* p = audioProcessor.valueTreeState.getParameter (valueTreeNames[FILL]))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (on ? 1.0f : 0.0f);
        p->endChangeGesture();
    }
}

NoteMap::Snapshot NoteMap::takeSnapshot() const
{
    Snapshot s;
    for (int n = 0; n < 128; n++)
    {
        if (audioProcessor.isNoteHeld (n))           s.held[n >> 6] |= (uint64_t) 1 << (n & 63);
        if (audioProcessor.isNotePhysicallyHeld (n)) s.phys[n >> 6] |= (uint64_t) 1 << (n & 63);
        if (audioProcessor.isScreenNote (n))         s.screen[n >> 6] |= (uint64_t) 1 << (n & 63);
    }
    for (int i = 0; i < numOfLine; i++)
    {
        s.in[i]  = audioProcessor.getMidi (i);
        s.out[i] = audioProcessor.getLaneOutNote (i);
    }
    return s;
}

// only repaint when what's held or sounding actually changed
void NoteMap::timerCallback()
{
    undoButton.setEnabled (audioProcessor.canUndo());
    redoButton.setEnabled (audioProcessor.canRedo());
    // lit while held here or while the host automates the parameter
    fillButton.setToggleState (audioProcessor.isFillOn(), juce::dontSendNotification);

    auto now = takeSnapshot();
    if (now != shown)
    {
        shown = now;
        repaint();
    }
}

void NoteMap::resized()
{
    auto area = getLocalBounds();
    area.removeFromLeft (26);                                    // line up with the lane-number column
    latchButton.setBounds (area.removeFromLeft (52).reduced (2, 4));
    fillButton.setBounds (area.removeFromLeft (44).reduced (2, 4));
    area.removeFromLeft (8);

    area.removeFromRight (6);
    redoButton.setBounds (area.removeFromRight (48).reduced (2, 4));
    undoButton.setBounds (area.removeFromRight (48).reduced (2, 4));
    area.removeFromRight (8);

    keyboard = area.reduced (2, 4).toFloat();
    int whites = 0;
    for (int n = lowNote; n <= highNote; n++)
        if (! isBlack (n)) whites++;
    whiteW = keyboard.getWidth() / (float) whites;
}

void NoteMap::mouseDown (const juce::MouseEvent& e)
{
    if (! playable) return;

    if (e.mods.isPopupMenu())
        audioProcessor.releaseScreenNotes();
    else if (auto note = noteAt (e.position); note >= 0)
        audioProcessor.toggleScreenNote (note);
    else
        return;

    shown = takeSnapshot();   // show the marker now; the lane colours follow next block
    repaint();
}

int NoteMap::noteAt (juce::Point<float> pos) const
{
    // black keys sit on top of the white ones, so test them first
    for (int n = lowNote; n <= highNote; n++)
        if (isBlack (n) && keyBounds (n).contains (pos)) return n;
    for (int n = lowNote; n <= highNote; n++)
        if (! isBlack (n) && keyBounds (n).contains (pos)) return n;
    return -1;
}

bool NoteMap::isBlack (int note)
{
    switch (note % 12) { case 1: case 3: case 6: case 8: case 10: return true; default: return false; }
}

juce::Rectangle<float> NoteMap::keyBounds (int note) const
{
    int whitesBelow = 0;
    for (int n = lowNote; n < note; n++)
        if (! isBlack (n)) whitesBelow++;

    const float x = keyboard.getX() + whitesBelow * whiteW;
    if (! isBlack (note))
        return { x, keyboard.getY(), whiteW, keyboard.getHeight() };

    // a black key straddles the boundary before the next white key
    const float w = whiteW * 0.62f;
    return { x - w * 0.5f, keyboard.getY(), w, keyboard.getHeight() * 0.6f };
}

void NoteMap::drawKey (juce::Graphics& g, int note, bool black) const
{
    auto r = keyBounds (note).reduced (black ? 0.0f : 0.5f, 0.0f);

    int lane = -1;
    for (int i = 0; i < numOfLine; i++)
        if (shown.in[i] == note) { lane = i; break; }

    const bool held = ((shown.held[note >> 6] >> (note & 63)) & 1) != 0;
    const bool phys = ((shown.phys[note >> 6] >> (note & 63)) & 1) != 0;

    // base key
    g.setColour (black ? Theme::well : Colour (0xff2b2c30));
    g.fillRoundedRectangle (r, 1.5f);
    if (black)
    {
        g.setColour (Theme::hairline.withAlpha (0.8f));
        g.drawRoundedRectangle (r, 1.5f, 0.8f);
    }

    if (held)
    {
        const auto c = lane >= 0 ? Theme::lane[lane] : Theme::textDim;
        if (phys || lane < 0)
        {
            g.setColour (c.withAlpha (0.95f));
            g.fillRoundedRectangle (r, 1.5f);
        }
        else
        {
            // held only by Latch: hollow key
            g.setColour (c.withAlpha (0.28f));
            g.fillRoundedRectangle (r, 1.5f);
            g.setColour (c);
            g.drawRoundedRectangle (r.reduced (0.8f), 1.5f, 1.4f);
        }

        if (lane >= 0)
        {
            g.setColour (phys ? Theme::screen : c.brighter (0.4f));
            g.setFont (Theme::valueFont (black ? 8.5f : 10.0f));
            auto label = black ? r : r.withTrimmedTop (r.getHeight() * 0.45f);
            g.drawText (juce::String (lane + 1), label, juce::Justification::centred, false);
        }
    }
    // held with the mouse: a brass bar along the bottom of the key
    if (((shown.screen[note >> 6] >> (note & 63)) & 1) != 0)
    {
        g.setColour (Theme::accentBright);
        g.fillRoundedRectangle (r.removeFromBottom (3.0f).reduced (black ? 1.5f : 3.0f, 0.0f), 1.0f);
        return;
    }

    if (! held && ! black && note % 12 == 0)
    {
        // octave landmark on every C
        g.setColour (Theme::textDim);
        g.setFont (Theme::valueFont (8.0f));
        g.drawText ("C" + juce::String (note / 12), r.withTrimmedTop (r.getHeight() * 0.55f),
                    juce::Justification::centred, false);
    }
}

void NoteMap::drawRangeArrow (juce::Graphics& g, bool left, juce::Colour c) const
{
    const float y = keyboard.getCentreY();
    const float x = left ? keyboard.getX() - 1.0f : keyboard.getRight() + 1.0f;
    const float d = left ? -5.0f : 5.0f;
    juce::Path p;
    p.addTriangle (x, y - 4.0f, x, y + 4.0f, x + d, y);
    g.setColour (c);
    g.fillPath (p);
}

void NoteMap::paint (juce::Graphics& g)
{
    g.fillAll (Theme::panel);
    Theme::drawRecessedWell (g, keyboard.expanded (2.0f, 2.0f), Theme::radSm);

    for (int n = lowNote; n <= highNote; n++)
        if (! isBlack (n)) drawKey (g, n, false);
    for (int n = lowNote; n <= highNote; n++)
        if (isBlack (n)) drawKey (g, n, true);

    // sounding notes: a dot per lane on top of its output key; lanes landing on
    // the same key sit side by side
    for (int i = 0; i < numOfLine; i++)
    {
        const int note = shown.out[i];
        if (note < 0) continue;

        if (note < lowNote || note > highNote)
        {
            drawRangeArrow (g, note < lowNote, Theme::lane[i]);
            continue;
        }

        int sharing = 0, slot = 0;
        for (int j = 0; j < numOfLine; j++)
            if (shown.out[j] == note) { if (j < i) slot++; sharing++; }

        auto key = keyBounds (note);
        const float rad = 3.0f;
        const float cx  = key.getCentreX() + (slot - (sharing - 1) * 0.5f) * (rad * 2.2f);
        const float cy  = key.getY() + rad + 2.0f;

        g.setColour (Theme::lane[i].withAlpha (0.35f));
        g.fillEllipse (cx - rad * 2.0f, cy - rad * 2.0f, rad * 4.0f, rad * 4.0f);   // glow
        g.setColour (Theme::lane[i].brighter (0.5f));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.0f, rad * 2.0f);
        g.setColour (Theme::screen);
        g.drawEllipse (cx - rad, cy - rad, rad * 2.0f, rad * 2.0f, 0.8f);
    }

    // held notes outside the drawn range
    for (int i = 0; i < numOfLine; i++)
    {
        const int note = shown.in[i];
        if (note >= 0 && (note < lowNote || note > highNote))
            drawRangeArrow (g, note < lowNote, Theme::lane[i].withAlpha (0.6f));
    }
}
