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

    // version stamp, silkscreen-faint
    String ver;
    ver << "v" << ProjectInfo::versionString;
    g.setColour (Theme::textDim);
    g.setFont (Theme::valueFont (9.0f));
    g.drawFittedText (ver, getWidth() - 62, topInLabel[11]->getBounds().getY(),
                      54, topInLabel[11]->getBounds().getHeight(), Justification::centredRight, 1);
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
    label_area.removeFromLeft (LaneLayout::number);
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
