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

    comboLookAndFeel.setColour (ComboBox::textColourId, Theme::textValue);
    comboLookAndFeel.setColour (PopupMenu::backgroundColourId, Theme::well);
    comboLookAndFeel.setColour (PopupMenu::textColourId, Theme::textPrimary);
    comboLookAndFeel.setColour (PopupMenu::highlightedBackgroundColourId, Theme::accent);
    comboLookAndFeel.setColour (PopupMenu::highlightedTextColourId, Theme::screen);
    comboLookAndFeel.setColour (PopupMenu::headerTextColourId, Theme::accentBright);
    keyBox.addItemList (scaleKeyNames, 1);
    // scales in groups, a few groups per column (item ids stay index + 1)
    scaleBox.addItem (scaleTypeNames[0], 1);
    const int groups = (int) std::size (scaleGroups);
    for (int g = 0; g < groups; g++)
    {
        if (scaleGroups[g].newColumn)
            scaleBox.getRootMenu()->addColumnBreak();
        scaleBox.addSectionHeading (scaleGroups[g].name);
        const int end = g + 1 < groups ? scaleGroups[g + 1].first : scaleTypeNames.size();
        for (int i = scaleGroups[g].first; i < end; i++)
            scaleBox.addItem (scaleTypeNames[i], i + 1);
    }
    for (auto* box : { &keyBox, &scaleBox })
    {
        box->setLookAndFeel (&comboLookAndFeel);
        box->setMouseCursor (juce::MouseCursor::NormalCursor);
        addAndMakeVisible (*box);
    }
    keyAttachment   = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.valueTreeState, valueTreeNames[SCALEKEY], keyBox);
    scaleAttachment = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.valueTreeState, valueTreeNames[SCALETYPE], scaleBox);

    midiExport.menuLookAndFeel = &comboLookAndFeel;
    addAndMakeVisible (midiExport);
    slotSelector.menuLookAndFeel = &comboLookAndFeel;
    addAndMakeVisible (slotSelector);

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
    keyAttachment.reset();
    scaleAttachment.reset();
    for (auto* box : { &keyBox, &scaleBox })
        box->setLookAndFeel (nullptr);
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
    s.scaleKey  = audioProcessor.getScaleKey();
    s.scaleType = audioProcessor.getScaleType();
    return s;
}

// only repaint when what's held or sounding actually changed
void NoteMap::timerCallback()
{
    undoButton.setEnabled (audioProcessor.canUndo());
    redoButton.setEnabled (audioProcessor.canRedo());
    // lit while held here or while the host automates the parameter
    fillButton.setToggleState (audioProcessor.isFillOn(), juce::dontSendNotification);
    slotSelector.repaint();

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
    auto legendRow = area.removeFromBottom (10);
    area.removeFromLeft (26);                                    // line up with the lane-number column
    latchButton.setBounds (area.removeFromLeft (52).reduced (2, 4));
    fillButton.setBounds (area.removeFromLeft (44).reduced (2, 4));
    area.removeFromLeft (4);
    slotSelector.setBounds (area.removeFromLeft (104).reduced (2, 4));
    area.removeFromLeft (4);
    keyBox.setBounds (area.removeFromLeft (46).reduced (2, 4));
    scaleBox.setBounds (area.removeFromLeft (122).reduced (2, 4));   // room for "Hungarian Min."
    area.removeFromLeft (8);

    area.removeFromRight (6);
    redoButton.setBounds (area.removeFromRight (48).reduced (2, 4));
    undoButton.setBounds (area.removeFromRight (48).reduced (2, 4));
    area.removeFromRight (4);
    midiExport.setBounds (area.removeFromRight (94).reduced (2, 4));
    area.removeFromRight (8);

    keyboard = area.reduced (2, 4).toFloat();
    legend = legendRow.toFloat().withX (keyboard.getX()).withWidth (keyboard.getWidth()).withTrimmedBottom (1.0f);
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

//==============================================================================
void MidiExportButton::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);
    Theme::drawRaisedPanel (g, b, Theme::radMd, isMouseOver() ? Theme::surface.brighter (0.08f) : Theme::surfaceAlt);

    // "drag out" icon: an arrow dropping into a tray
    auto icon = b.removeFromLeft (20.0f).withSizeKeepingCentre (10.0f, 12.0f);
    g.setColour (Theme::accentBright);
    const float cx = icon.getCentreX();
    g.drawLine (cx, icon.getY(), cx, icon.getBottom() - 4.0f, 1.4f);
    juce::Path head;
    head.addTriangle (cx - 3.5f, icon.getBottom() - 6.0f, cx + 3.5f, icon.getBottom() - 6.0f, cx, icon.getBottom() - 2.5f);
    g.fillPath (head);
    g.drawLine (icon.getX(), icon.getBottom() - 0.5f, icon.getRight(), icon.getBottom() - 0.5f, 1.4f);

    const int bars = proc.getExportBars();
    Theme::drawCaption (g, "MIDI", b.removeFromLeft (32.0f).toNearestInt(), juce::Justification::centredLeft,
                        Theme::textPrimary, 10.0f);
    Theme::drawCaption (g, juce::String (bars) + (bars == 1 ? " bar" : " bars"), b.reduced (2.0f, 0.0f).toNearestInt(),
                        juce::Justification::centredRight, Theme::textDim, 8.5f);
}

void MidiExportButton::mouseDrag (const juce::MouseEvent& e)
{
    if (dragStarted || e.getDistanceFromDragStart() < 4) return;
    dragStarted = true;
    auto file = proc.renderPatternToMidiFile (proc.getExportBars());
    if (file.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}

void MidiExportButton::mouseUp (const juce::MouseEvent&)
{
    if (! dragStarted)
        showMenu();
}

void MidiExportButton::showMenu()
{
    enum { barsBase = 100, saveId = 1 };
    const int current = proc.getExportBars();

    juce::PopupMenu m;
    m.addSectionHeader ("Drag this button into your DAW");
    for (int bars : { 1, 2, 4, 8, 16 })
        m.addItem (barsBase + bars, juce::String (bars) + (bars == 1 ? " bar" : " bars"), true, bars == current);
    m.addSeparator();
    m.addItem (saveId, "Save as .mid file...");
    if (menuLookAndFeel != nullptr)
        m.setLookAndFeel (menuLookAndFeel);

    juce::Component::SafePointer<MidiExportButton> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this), [safe] (int r)
    {
        if (safe == nullptr || r == 0) return;
        if (r > barsBase)
        {
            safe->proc.setExportBars (r - barsBase);
            safe->repaint();
            return;
        }
        auto rendered = safe->proc.renderPatternToMidiFile (safe->proc.getExportBars());
        if (! rendered.existsAsFile()) return;
        safe->chooser = std::make_shared<juce::FileChooser> ("Save pattern as MIDI file",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("TugMidiSeq pattern.mid"), "*.mid");
        safe->chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
            [rendered] (const juce::FileChooser& fc)
            {
                auto target = fc.getResult();
                if (target != juce::File())
                    rendered.copyFileTo (target.withFileExtension ("mid"));
            });
    });
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

    // base key; with a scale set, keys outside it are darker
    const bool scaleOn = shown.scaleType > 0;
    const bool inScale = audioProcessor.isInScale (note);
    Colour base = black ? Theme::well : Colour (0xff2b2c30);
    if (scaleOn)
        base = inScale ? (black ? Colour (0xff24262a) : Colour (0xff3a3c41))
                       : (black ? Colour (0xff0b0c0d) : Colour (0xff1e1f22));
    g.setColour (base);
    g.fillRoundedRectangle (r, 1.5f);
    if (scaleOn && ((note - shown.scaleKey) % 12 + 12) % 12 == 0)
    {
        // tonic: a thin brass line along the bottom
        g.setColour (Theme::accent.withAlpha (0.7f));
        g.fillRect (r.getX() + 2.0f, r.getBottom() - 1.5f, r.getWidth() - 4.0f, 1.5f);
    }
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

        const bool clicked = ((shown.screen[note >> 6] >> (note & 63)) & 1) != 0;
        if (lane >= 0)
        {
            auto label = black ? r : r.withTrimmedTop (r.getHeight() * 0.45f);
            if (clicked)
            {
                // held with the mouse: the lane number in a brass ring, as on
                // the lane's own number
                const float d = jmin (label.getWidth() - 1.0f, label.getHeight() - 1.0f, 12.0f);
                auto ring = label.withSizeKeepingCentre (d, d);
                g.setColour (Theme::screen.withAlpha (0.85f));
                g.fillEllipse (ring);
                g.setColour (Theme::accentBright);
                g.drawEllipse (ring.reduced (0.6f), 1.4f);
                g.setColour (Theme::accentBright);
            }
            else
            {
                g.setColour (phys ? Theme::screen : c.brighter (0.4f));
            }
            g.setFont (Theme::valueFont (black ? 8.5f : 10.0f));
            g.drawText (juce::String (lane + 1), label, juce::Justification::centred, false);
        }
        else if (clicked)
        {
            // clicked but no lane takes it: a brass frame round the key
            g.setColour (Theme::accentBright);
            g.drawRoundedRectangle (r.reduced (0.75f), 1.5f, 1.6f);
        }
        if (clicked)
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

// A key-style swatch, a caption and a count for each way a key can be lit:
// held on a MIDI keyboard, held by a click here, kept by Latch, and the dot of
// a note being played.
void NoteMap::drawLegend (juce::Graphics& g) const
{
    auto count = [] (const uint64_t* mask)
    {
        return (int) (std::bitset<64> (mask[0]).count() + std::bitset<64> (mask[1]).count());
    };
    const uint64_t midiOnly[2] = { shown.phys[0] & ~shown.screen[0], shown.phys[1] & ~shown.screen[1] };
    const uint64_t latched[2]  = { shown.held[0] & ~shown.phys[0],   shown.held[1] & ~shown.phys[1] };

    auto r = legend;
    const float h = r.getHeight();
    const auto swatchColour = Theme::textSecondary;
    auto item = [&] (const juce::String& caption, int n, auto&& drawSwatch)
    {
        auto sw = r.removeFromLeft (h + 1.0f).reduced (1.5f, 1.0f);
        drawSwatch (sw);
        r.removeFromLeft (3.0f);
        const auto text = n >= 0 ? caption + " " + juce::String (n) : caption;
        const float w = juce::GlyphArrangement::getStringWidth (Theme::labelFont (8.0f), text.toUpperCase()) + 4.0f;
        Theme::drawCaption (g, text, r.removeFromLeft (w).toNearestInt(), juce::Justification::centredLeft,
                            n > 0 || n < 0 ? Theme::textSecondary : Theme::textDim, 8.0f);
        r.removeFromLeft (12.0f);
    };

    item ("Midi", count (midiOnly), [&] (juce::Rectangle<float> s)
          { g.setColour (swatchColour); g.fillRoundedRectangle (s, 1.0f); });
    item ("Clicked", count (shown.screen), [&] (juce::Rectangle<float> s)
          { g.setColour (Theme::accentBright); g.drawEllipse (s.withSizeKeepingCentre (7.0f, 7.0f), 1.3f); });
    item ("Latched", count (latched), [&] (juce::Rectangle<float> s)
          { g.setColour (swatchColour); g.drawRoundedRectangle (s.reduced (0.5f), 1.0f, 1.0f); });
    item ("Sounding", -1, [&] (juce::Rectangle<float> s)
          { g.setColour (Theme::accentBright); g.fillEllipse (s.withSizeKeepingCentre (5.0f, 5.0f)); });
}

void NoteMap::paint (juce::Graphics& g)
{
    g.fillAll (Theme::panel);
    drawLegend (g);
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

//==============================================================================
int SlotSelector::slotAt (juce::Point<float> p) const
{
    return juce::jlimit (0, numSlots - 1, (int) (p.x / ((float) getWidth() / (float) numSlots)));
}

void SlotSelector::paint (juce::Graphics& g)
{
    const int requested = proc.getRequestedSlot();
    bool waiting = false, playing[numSlots] = {};
    for (int i = 0; i < numOfLine; i++)
    {
        waiting |= proc.isLaneWaitingForSlot (i);
        playing[proc.getActiveSlot (i)] = true;
    }
    const bool blinkOn = (juce::Time::getMillisecondCounter() / 250) % 2 == 0;

    const float w = (float) getWidth() / (float) numSlots;
    for (int s = 0; s < numSlots; s++)
    {
        auto b = juce::Rectangle<float> ((float) s * w, 0.0f, w, (float) getHeight()).reduced (1.5f, 0.5f);
        const bool lit = s == requested && (! waiting || blinkOn);
        Theme::drawRaisedPanel (g, b, Theme::radMd, lit ? Theme::accent : Theme::surfaceAlt);
        if (playing[s] && s != requested)   // lanes still playing it until their loop ends
        {
            g.setColour (Theme::accent);
            g.drawRoundedRectangle (b.reduced (0.5f), Theme::radMd, 1.4f);
        }
        const auto text = lit ? Theme::screen : proc.isSlotEmpty (s) ? Theme::textDim : Theme::textPrimary;
        Theme::drawCaption (g, slotNames[s], b.toNearestInt(), juce::Justification::centred, text, 11.0f);
    }
}

void SlotSelector::mouseDown (const juce::MouseEvent& e)
{
    const int slot = slotAt (e.position);
    if (e.mods.isPopupMenu())
        showMenu (slot);
    else
        proc.requestSlot (slot);
    repaint();
}

void SlotSelector::showMenu (int slot)
{
    enum { copyBase = 100, clearId = 1 };
    juce::PopupMenu copyTo;
    for (int s = 0; s < numSlots; s++)
        if (s != slot)
            copyTo.addItem (copyBase + s, slotNames[s] + (proc.isSlotEmpty (s) ? juce::String ("  (empty)") : juce::String ("  (replaces it)")));

    juce::PopupMenu m;
    m.setLookAndFeel (menuLookAndFeel);
    m.addSectionHeader ("Pattern slot " + slotNames[slot]);
    m.addSubMenu ("Copy " + slotNames[slot] + " to", copyTo, ! proc.isSlotEmpty (slot));
    m.addItem (clearId, "Clear " + slotNames[slot], ! proc.isSlotEmpty (slot));
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                     [this, slot, safe = juce::Component::SafePointer<SlotSelector> (this)] (int id)
                     {
                         if (safe == nullptr || id == 0) return;
                         if (id == clearId) proc.clearSlot (slot);
                         else               proc.copySlot (slot, id - copyBase);
                         repaint();
                     });
}
