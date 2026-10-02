/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Grids.h"
#include "Satellite.h"
#include "GlobalPanel.h"
#include "NoteMap.h"


// index 12 ("Strum") was added later, so the existing indices stay put
const std::vector <std::string> topLabel={"midi in","Oct","G R I D S","#Grid","Speed","Duration","Vel","Event","Shuffle","Delay","Chan","Note Satellite","Strum"};

// The UI is authored at this fixed size; the editor scales it uniformly.
static constexpr int kNoteMapH      = 30;
static constexpr int kEditorDesignW = 1212;
static constexpr int kEditorDesignH = 276 + kNoteMapH;

//==============================================================================
// All widgets live here at the fixed design size. The editor owns one of these
// and applies a uniform scale transform when the window is resized, so the
// magic-number layout never has to change.
class EditorContent  : public juce::Component
{
public:
    EditorContent (TugMidiSeqAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TugMidiSeqAudioProcessor& audioProcessor;

    juce::OwnedArray< Grids> grids;
    Satellite satellite;
    NoteMap noteMap;
    GlobalPanel globalPanel;
    juce::OwnedArray< juce::Label > topInLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorContent)
};

//==============================================================================
/**
*/
class TugMidiSeqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor&);
    ~TugMidiSeqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TugMidiSeqAudioProcessor& audioProcessor;

    EditorContent content;
    juce::ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessorEditor)
};

