/*
  ==============================================================================

    NoteMap.h
    Keyboard strip under the lanes: shows which held note feeds which lane.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Theme.h"
#include "MyLookanAndFeels.h"

// Drag source for the pattern as a MIDI file: dragging it out renders the
// current pattern and hands the file to the host; a click opens the length
// menu and a "Save as" fallback for hosts that don't take file drops.
class MidiExportButton : public juce::Component
{
public:
    explicit MidiExportButton (TugMidiSeqAudioProcessor& p) : proc (p)
    {
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    }
    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { repaint(); }
    void mouseDown (const juce::MouseEvent&) override  { dragStarted = false; }
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void showMenu();
    TugMidiSeqAudioProcessor& proc;
    bool dragStarted = false;
    std::shared_ptr<juce::FileChooser> chooser;
};

// Each held input note is lit in the colour of the lane it feeds, with the lane
// number on the key; notes held only by Latch are drawn hollow, and notes no
// lane picked up (more keys than lanes) stay grey. A dot on top of a key marks
// the note a lane is sounding right now, i.e. after its octave shift.
// Also hosts the Latch switch.
//
// When `playable` is on, the keys can be clicked: a click toggles a note that
// stays down until clicked again (so a chord can be built with the mouse), and
// a right-click releases every clicked note.
class NoteMap : public juce::Component, private juce::Timer
{
public:
    // Set to false to make the keyboard display-only again.
    static constexpr bool playable = true;

    explicit NoteMap (TugMidiSeqAudioProcessor&);
    ~NoteMap() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    static constexpr int lowNote  = 24;    // C2 (in this plugin's naming: note / 12)
    static constexpr int highNote = 107;   // B8 -> 7 octaves

    struct Snapshot
    {
        uint64_t held[2] = {}, phys[2] = {}, screen[2] = {};
        int in[numOfLine] = {}, out[numOfLine] = {};
        int scaleKey = 0, scaleType = 0;
        bool operator!= (const Snapshot& o) const
        {
            return std::memcmp (this, &o, sizeof (Snapshot)) != 0;
        }
    };

    void timerCallback() override;
    Snapshot takeSnapshot() const;

    static bool isBlack (int note);
    juce::Rectangle<float> keyBounds (int note) const;
    int noteAt (juce::Point<float> pos) const;   // -1 if not on a key
    void drawKey (juce::Graphics&, int note, bool black) const;
    void drawRangeArrow (juce::Graphics&, bool left, juce::Colour) const;

    TugMidiSeqAudioProcessor& audioProcessor;
    Snapshot shown;

    juce::TextButton latchButton;
    juce::TextButton fillButton;   // momentary: Fill is on while held
    bool fillHeld = false;
    void setFill (bool on);
    juce::TextButton undoButton, redoButton;   // right end of the strip
    MidiExportButton midiExport { audioProcessor };

    // scale lock
    MyLookAndFeel comboLookAndFeel;
    WheelComboBox keyBox, scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment, scaleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> latchAttachment;

    juce::Rectangle<float> keyboard;
    float whiteW = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteMap)
};
