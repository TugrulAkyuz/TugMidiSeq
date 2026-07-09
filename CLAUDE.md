# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

TugMidiSeq is a JUCE audio plugin (by "2Rule") — a 5-lane MIDI step sequencer / arpeggiator. It takes incoming MIDI notes and re-triggers them across up to 5 independent lanes, each a 32-step grid with its own speed, duration, octave, velocity, event-probability, shuffle and delay. It builds as AU, VST3, and Standalone. Despite being a MIDI effect, it declares itself a synth (`pluginIsSynth`) and also ships an optional in-built synth voice, so hosts route it as an instrument.

## Build

The project is generated from `TugMidiSeq.jucer` by Projucer (JUCE's project tool). `Builds/` and `JuceLibraryCode/` are **generated and git-ignored** — never hand-edit them; edit the `.jucer` file or `Source/` instead, then re-save in Projucer to regenerate.

- **Regenerate projects:** open `TugMidiSeq.jucer` in Projucer and Save, or `Projucer --resave TugMidiSeq.jucer`.
- **macOS build:** `xcodebuild -project Builds/MacOSX/TugMidiSeq.xcodeproj -configuration Debug` (or `Release`). Targets: AU, VST3, Standalone.
- **Windows:** open `Builds/VisualStudio2022/TugMidiSeq.sln`.
- **JUCE modules** are expected at `../../JUCE/modules` on macOS and `C:\JUCE\modules` on Windows (`useGlobalPath`). Adding/removing a source file means adding it to `<GROUP name="Source">` in the `.jucer`, not just to `Source/`.

There are no automated tests. The `.jucer` history references pluginVal — validate the built plugin with `pluginval` if checking host-compatibility.

## Architecture

Audio-thread engine and GUI are cleanly split; they communicate almost exclusively through the `AudioProcessorValueTreeState` (APVTS) and a couple of global `ChangeBroadcaster`s.

### Processor / engine (`PluginProcessor.{h,cpp}`)
- `TugMidiSeqAudioProcessor` owns `valueTreeState` (APVTS). All per-lane controls are parameters named by concatenating a base name + lane index (e.g. `Speed0`..`Speed4`), and each grid cell is `block<lane><step>` / `velGridButton<lane><step>`. The base names and their enum live in `valueTreeNames` / `valueTreeNamesEnum` in `PluginProcessor.h`. Parameters are created in the constructor (`createAndAddParameter` loops).
- The engine caches raw `std::atomic<float>*` pointers to every parameter (`gridsArr`, `gridsSpeedAtomic`, etc.) so `processBlock` reads parameter values lock-free.
- `processBlock` drives timing from the host playhead (`getPlayHead()` → `CurrentPositionInfo`, `ppqPosition`). It advances a per-lane sample counter, and per step calls `subComputrFunc(lane, ...)` to decide/emit notes. Incoming MIDI is captured into `inMidiNoteList` / `inMidiNoteListVector` (chord memory) and re-emitted according to each lane's grid; `sortedOrFirstEmptySelect` chooses note-selection behaviour.
- **MIDI output** goes two ways: into the host's `MidiBuffer`, and out an external MIDI port via the nested `MidiProcessor` helper (opens a `juce::MidiOutput`, guarded by the global `midiOutputMutex`). The chosen port name is persisted inside the APVTS state tree under a `midiPort` child (see `get/setMidiPortNameToXml`).
- Constants: `numOfLine` = 5 lanes, `numOfStep` = 32 steps. Note-duration units are the `myNotetUnit` / `myNotetUnitSA` arrays (`1nd`..`128nt`, d=dotted, t=triplet).

### Presets (`PresetMenu.cpp`)
Presets are **not** stored in the APVTS/host state — they live in a single external JSON file, `TugMidiSeqPresets.json`, and are loaded into `std::vector<TugMidiSeqProgram> myProgram`. `TugMidiSeqProgram` (in `PluginProcessor.h`) is the plain-struct snapshot of all lane arrays. Preset file location (see top of `PluginProcessor.cpp`):
- macOS: `~/Library/Audio/Presets/2Rule/…`
- Windows: `<userAppData>/2Rule/…`

`writePresetToFileJSON` / `readPresetToFileJSON` / `createPrograms` / `deletePreset` manage this file.

### Editor / GUI (`PluginEditor.{h,cpp}` + panels)
- `TugMidiSeqAudioProcessorEditor` lays out 5 `Grids` (an `OwnedArray`), one `GlobalPanel`, one `Satellite`, and a row of top labels (`topLabel`).
- `Grids` / `SubGrids` / `MultiStateButton` (`Grids.{h,cpp}`) render one lane: the step grid plus per-lane knobs. `MultiStateButton` is a 3-state grid cell (Off / On / Event) that listens to the APVTS directly. `SubGrids` are `Timer`-driven (~20 ms) and repaint to animate playhead position — they read live engine state through getter methods on the processor (`getGridContinousRatio`, `getSteps`, `getSfuffleRatios`, `getDurAngle`, etc.).
- `GlobalPanel` (`GlobalPanel.{h,cpp}`) holds global controls including the MIDI-out port picker (`ComboBoxDialog`), preset menu, solo, and global velocity/resync-bar settings.
- `Satellite` (`Satellite.{h,cpp}`) is a visualiser of the currently-held incoming notes.
- `MyLookanAndFeels.h` — shared custom `LookAndFeel`.

### Cross-thread signalling
Three file-scope globals (declared `extern` in headers, defined in `PluginProcessor.cpp`) push updates from engine/processor to GUI:
- `myGridChangeListener` — broadcast when grid parameters change in bulk (e.g. `setAllValue`, `randomizeGrids`, preset load) so cells refresh.
- `updateMidiPort` — MIDI-port selection changed.
- `midiOutputMutex` (`CriticalSection`) — guards the external MIDI output device.

### In-built synth (`SynthVoice.{h,cpp}`)
Optional wavetable `Synthesiser` (`SynthVoice` / `SynthSound` / `OscWaves` / `WaveTableList`) so the plugin can make sound directly. Gated by the `inBuiltSynth` parameter.

## Conventions / gotchas
- `using namespace juce;` is applied globally (`PluginProcessor.h`), so JUCE types appear unqualified throughout.
- Parameter identity is string-based and index-suffixed — when touching per-lane logic, the pattern is almost always a loop `for (line 0..4)` building `baseName + std::to_string(line)`, and for cells `+ std::to_string(step)`.
- Editing a lane count or step count means changing `numOfLine` / `numOfStep` **and** the fixed-size `[5]` / `[numOfStep]` arrays scattered through the processor.
- Presets are versioned informally: newer fields are read with `preset.hasProperty(...)` guards for backward compatibility — follow that pattern when adding a preset field.
