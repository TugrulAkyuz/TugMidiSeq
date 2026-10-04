/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// EditorContent — the fixed-size UI. Identical layout to before; it just no
// longer lives directly in the editor so it can be scaled as one unit.
//==============================================================================
EditorContent::EditorContent (TugMidiSeqAudioProcessor& p)
    : audioProcessor (p), satellite (p), noteMap (p), globalPanel (p)
{
    addAndMakeVisible (satellite);
    addAndMakeVisible (noteMap);
    for (auto i = 4; i >= 0; i--)
    {
        auto g = new Grids (audioProcessor, i);
        grids.add (g);
        addAndMakeVisible (g);
    }
    addAndMakeVisible (globalPanel);

    for (auto i = 0; i < topLabel.size(); i++)
    {
        auto* lbl = new juce::Label (topLabel.at (i), juce::String (topLabel.at (i)).toUpperCase());
        lbl->setColour (Label::textColourId, myTextLabelColour);
        lbl->setJustificationType (juce::Justification::centred);
        lbl->setFont (Theme::labelFont (10.5f));
        topInLabel.add (lbl);
        addAndMakeVisible (lbl);
    }
    for (auto i = 0; i < 5; i++)
        globalPanel.setGridComp (grids[i], i);

    // takes the keyboard focus when anything inside is clicked, so keyPressed
    // sees the shortcuts; unhandled keys go on to the host. Combo boxes want the
    // focus by default and would eat the arrow keys (to step their selection),
    // so they're told not to: they still work with the mouse and wheel, and an
    // open popup menu still takes the arrows.
    setWantsKeyboardFocus (true);
    std::function<void (juce::Component&)> keepArrowsForShortcuts = [&] (juce::Component& c)
    {
        if (auto* box = dynamic_cast<juce::ComboBox*> (&c))
            box->setWantsKeyboardFocus (false);
        for (auto* child : c.getChildren())
            keepArrowsForShortcuts (*child);
    };
    keepArrowsForShortcuts (*this);

    setSize (kEditorDesignW, kEditorDesignH);
}

void EditorContent::paint (juce::Graphics& g)
{
    // hardware module plate
    g.fillAll (Theme::panel);
    auto allarea = getLocalBounds();

    // outer frame + corner screws
    g.setColour (Theme::hairline);
    g.drawRect (allarea, 1);
    const float sc = 8.0f;
    Theme::drawScrew (g, sc,               sc,                sc * 0.55f);
    Theme::drawScrew (g, getWidth() - sc,  sc,                sc * 0.55f);
    Theme::drawScrew (g, sc,               getHeight() - sc,  sc * 0.55f);
    Theme::drawScrew (g, getWidth() - sc,  getHeight() - sc,  sc * 0.55f);

    // menu glyph over the lane numbers, which open each lane's menu
    {
        auto r = laneMenuHint.toFloat().withSizeKeepingCentre (11.0f, 8.0f).translated (2.0f, 1.0f);
        g.setColour (Theme::textSecondary);
        for (int k = 0; k < 3; k++)
            g.fillRoundedRectangle (r.getX(), r.getY() + (float) k * 3.5f, r.getWidth(), 1.4f, 0.7f);
    }

    // version stamp, silkscreen-faint
    String ver;
    ver << "v" << ProjectInfo::versionString;
    g.setColour (Theme::textDim);
    g.setFont (Theme::valueFont (9.0f));
    g.drawFittedText (ver, getWidth() - 62, topInLabel[11]->getBounds().getY(),
                      54, topInLabel[11]->getBounds().getHeight(), Justification::centredRight, 1);
}

Grids* EditorContent::laneUnderMouse() const
{
    const auto mouse = juce::Desktop::getMousePosition();
    for (auto* g : grids)
        if (g->getScreenBounds().contains (mouse))
            return g;
    return nullptr;
}

// Cmd+Z / Cmd+Shift+Z (Ctrl on Windows): undo / redo.
// With the mouse over a lane: Left / Right shift that lane a step,
// Shift+Left / Shift+Right shift every lane. Away from the lanes the arrows
// are left to the host, as is every other key.
bool EditorContent::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();

    if (mods.isCommandDown() && juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) code) == 'z')
    {
        if (mods.isShiftDown()) audioProcessor.redo();
        else                    audioProcessor.undo();
        return true;
    }

    if ((code == juce::KeyPress::leftKey || code == juce::KeyPress::rightKey)
        && ! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        auto* lane = laneUnderMouse();
        if (lane == nullptr) return false;
        const int delta = code == juce::KeyPress::leftKey ? -1 : 1;
        if (mods.isShiftDown()) audioProcessor.shiftAllLanes (delta);
        else                    audioProcessor.shiftLane (lane->getLine(), delta);
        return true;
    }

    // up / down: the lane's play direction (shift: every lane)
    if ((code == juce::KeyPress::upKey || code == juce::KeyPress::downKey)
        && ! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        auto* lane = laneUnderMouse();
        if (lane == nullptr) return false;
        audioProcessor.cycleLaneDirection (lane->getLine(), code == juce::KeyPress::downKey ? 1 : -1, mods.isShiftDown());
        return true;
    }

    // F / R / P / X: Forward, Reverse, Ping-Pong, Random (shift: every lane)
    if (! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        const auto letter = juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) code);
        int dir = -1;
        for (int d = 0; d < directionNames.size(); d++)
            if (letter == juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) directionKeys[d]))
                dir = d;
        if (dir >= 0)
        {
            auto* lane = laneUnderMouse();
            if (lane == nullptr) return false;
            audioProcessor.setDirectionOfLanes (lane->getLine(), dir, mods.isShiftDown());
            return true;
        }
    }
    return false;
}

void EditorContent::resized()
{
    auto allarea = getLocalBounds();
    allarea.reduce (3, 3);
    // Captions sit over the lane columns, taken from the same LaneLayout
    // widths Grids::resized uses (the lanes span the width left of the
    // satellite). The narrow left columns get no label insets, or their
    // captions are cut short.
    auto label_area = allarea.removeFromTop (20);
    topInLabel[11]->setBounds (label_area.removeFromRight (200));   // satellite
    for (int i : { 0, 1, 12 })
        topInLabel[i]->setBorderSize ({});
    laneMenuHint = label_area.removeFromLeft (LaneLayout::number).withTrimmedLeft (4);
    topInLabel[0]->setBounds (label_area.removeFromLeft (LaneLayout::midiIn));
    topInLabel[12]->setBounds (label_area.removeFromLeft (LaneLayout::spread));
    // "OCT" is wider than its 16 px slider: let it run into the grid's caption area
    auto octave = label_area.removeFromLeft (LaneLayout::octave);
    topInLabel[1]->setJustificationType (juce::Justification::centredLeft);
    topInLabel[1]->setBounds (octave.withWidth (30));
    label_area.removeFromLeft (14);
    topInLabel[10]->setBounds (label_area.removeFromRight (LaneLayout::chan));
    topInLabel[9]->setBounds (label_area.removeFromRight (LaneLayout::delay));
    topInLabel[8]->setBounds (label_area.removeFromRight (LaneLayout::shuffle));
    topInLabel[7]->setBounds (label_area.removeFromRight (LaneLayout::event));
    topInLabel[6]->setBounds (label_area.removeFromRight (LaneLayout::vel));
    topInLabel[5]->setBounds (label_area.removeFromRight (LaneLayout::duration));
    topInLabel[4]->setBounds (label_area.removeFromRight (LaneLayout::speed));
    topInLabel[3]->setBounds (label_area.removeFromRight (LaneLayout::steps));
    topInLabel[2]->setBounds (label_area);

    auto area = allarea.removeFromTop (200);
    auto satelite_area = area.removeFromRight (200);
    auto h = area.getHeight() / 5;
    for (auto g : grids)
        g->setBounds (area.removeFromTop (h));

    satellite.setBounds (satelite_area);
    noteMap.setBounds (allarea.removeFromTop (kNoteMapH));
    globalPanel.setBounds (allarea);
}

//==============================================================================
// The editor is a thin uniform scaler around EditorContent.
//==============================================================================
TugMidiSeqAudioProcessorEditor::TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), content (p)
{
    addAndMakeVisible (content);

    // minimum = the authored size; larger allowed, aspect ratio locked so the
    // whole panel scales together (works the same in AU and VST3).
    constrainer.setFixedAspectRatio ((double) kEditorDesignW / (double) kEditorDesignH);
    constrainer.setSizeLimits (kEditorDesignW, kEditorDesignH,
                               kEditorDesignW * 3, kEditorDesignH * 3);
    setConstrainer (&constrainer);
    setResizable (true, true);

    // restore the last-used width (project- and session-persistent); height
    // follows from the locked aspect ratio.
    int w = juce::jlimit (kEditorDesignW, kEditorDesignW * 3,
                          audioProcessor.getEditorWidth (kEditorDesignW));
    int h = juce::roundToInt (w * (double) kEditorDesignH / (double) kEditorDesignW);
    setSize (w, h);
}

TugMidiSeqAudioProcessorEditor::~TugMidiSeqAudioProcessorEditor()
{
}

void TugMidiSeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::panel);
}

void TugMidiSeqAudioProcessorEditor::resized()
{
    auto scale = (float) getWidth() / (float) kEditorDesignW;
    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, kEditorDesignW, kEditorDesignH);

    // remember the new width so it survives GUI reopen and project reload
    audioProcessor.setEditorWidth (getWidth());
}
