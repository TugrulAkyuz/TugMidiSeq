/*
  ==============================================================================

    Tuning.h

    A Scala tuning: an .scl scale and an optional .kbm keyboard mapping, turned
    into the frequency of every MIDI note plus what the sequencer needs to move
    around in it (the period in keys and which keys are mapped).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

struct Tuning
{
    juce::String name;               // the .scl description, or its file name
    juce::String scl, kbm;           // file contents: a project carries its tuning, not a path
    juce::String sclFile, kbmFile;   // file names, for the panel

    int    notes = 0;                // scale degrees per period
    std::vector<double> cents;       // degree 1 .. notes; the last one is the period

    // keyboard mapping (no .kbm: linear, middle C on 261.63 Hz)
    int    mapSize = 0, firstKey = 0, lastKey = 127, middleKey = 60, refKey = 60, octaveDegree = 0;
    double refFreq = 261.6255653005986;
    std::vector<int> mapping;        // degree per map entry, -1 = unmapped ("x")

    double freq[128] {};
    bool   mapped[128] {};

    // Parses the files' text (kbm empty = linear). Returns an error, empty when it worked.
    juce::String load (const juce::String& sclText, const juce::String& kbmText);

    bool   isLoaded() const        { return notes > 0; }
    int    periodKeys() const      { return mapSize > 0 ? mapSize : notes; }   // keys up to the same note a period higher
    double periodRatio() const;
    bool   hasUnmappedKeys() const;
    int    mapStartKey() const;   // where the pattern repeats nearest below the reference key
    double degreeCents (int degree) const;
};
