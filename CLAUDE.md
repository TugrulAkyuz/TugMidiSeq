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
- **Trig conditions** (`TrigCond`: `1:2`…`4:4`, `1ST`, `PRE`, `NEI` and negations) are per-step but deliberately **not** APVTS parameters — they live in `std::atomic<int> stepCond[lane][step]`, are saved into a `stepConds` child of the state tree in `getStateInformation`, and go into preset JSON as sparse `cond<lane><step>` keys. `evaluateTrigCond` counts loops per lane from the step index wrapping; `NEI` reads the lane below (`lane - 1`, wrapping).
- **Ratchet** (`stepRatchet[lane][step]`, 1..`maxRatchet`) is stored exactly like the conditions (`stepRatchets` state child, sparse `ratchet<lane><step>` preset keys). `subComputrFunc` plays the first hit and arms a per-lane countdown (`ratchetLeft` / `ratchetCountdown`) that `processBlock` ticks every sample; every hit goes through `emitLaneNote`. New `TrigCond` values must be **appended** (FILL/!FILL were): their numbers are stored in projects and presets.
- **Step pitch + scale lock**: `stepPitch[lane][step]` (±`maxStepPitch`, stored like ratchets, preset key `pitch<lane><step>`) and the global `scaleKey` / `scaleType` params. Everything goes through `pitchedNote()`: scale Off → offset in semitones; a scale set → the note snaps to the nearest scale tone (ties down) and the offset counts scale *degrees*. Edited with alt+drag on a pad (`StepValuePopup`, shared with shift+drag velocity).
- **Mutate** (`Mutate<lane>`, 0–100 %): non-destructive. `mutateMask[lane]` (audio thread) XORs the written on/off state at fire time; `mutateLane` flips each step with that chance once per completed loop. Cleared on every play and by `requestMutationReset`; the pads read the published `pubMutateMask`.
- **Chord / strum lanes** (`PlayMode<lane>`: Voice / Chord / Strum Up / Down / Up-Down, `Spread<lane>` ms): a non-Voice lane plays every held note through `playChord()`; strummed notes wait in a fixed per-lane `strumQueue` ticked each sample by `tickStrum`, and are shortened so all end together. Ratchet repeats replay the whole chord.
- **Fill** (`fill` param) is held by the momentary Fill button in `NoteMap`; it only feeds the FILL / !FILL conditions.
- **Latch** (`latch` param): note-offs are ignored while on; the first key after all keys are released replaces the held chord. `physHeld[]` tracks real key state, since the held-note lists keep latched notes.
- **Play direction** (`Direction<lane>`: Forward / Reverse / Ping-Pong / Random): `steps[]` is the *time slot* (shuffle, delay and note-length maths stay on slots) and `playStep[]` is the grid step that slot plays (`directedStep`). Read cells/velocity/conditions through `playStep`, timing through `steps`; `getSteps()` returns `playStep` for the GUI highlight. Drawing uses `getStepDisplayRatio()` (the shuffle ratio of the slot a step is *played* in), so pad widths mirror while a lane travels backward and the playhead keeps an even speed.
- **Undo**: the APVTS was always created with `undoManager`, so parameter changes are recorded when it flushes them to the tree (on a timer). `undoableEdit()` / `beginUndoStep()` + `endUndoStep()` group one user action and flush first; step conditions go through `setStepCondUndoable` (an `UndoableAction`). The undo history is message-thread only. `beginUndoStep()` flushes *before* opening the step too, or a change still waiting for the APVTS timer lands in the new step with the wrong before-value.
- **Lane edits** (`copyLane` / `pasteLane` / `shiftLane` / `euclidLane` / `clearLane`) live in `PluginProcessor.cpp`; the clipboard is file-static, so it works across plugin instances. The lane menu opens from the lane number (`Grids::showLaneMenu`).
- **MIDI export** (`renderPattern` / `renderPatternToMidiFile`): constructs a private `TugMidiSeqAudioProcessor`, copies this one's state into it, sets `offlineRender` (no external MIDI port, no synth) and runs `processBlock` against an offline 120 BPM playhead. Keep it at 48 kHz: step lengths round up to whole samples, which only stays inside a 960-PPQ tick at that rate. Dragged out from `MidiExportButton` on the note-map strip.
- The GUI never reads the held-note vectors directly: `publishNoteMap()` writes an atomic snapshot (`laneInNote`, `laneOutNote`, held/physical bitmasks) once per block.
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
- `NoteMap` (`NoteMap.{h,cpp}`) is the keyboard strip under the lanes (held note → lane colour, latched notes hollow, sounding notes as dots) and hosts the Latch button. Its height is `kNoteMapH`, included in `kEditorDesignH`. With `NoteMap::playable` on, clicking a key toggles a held note (right-click releases all); the processor injects these at the top of `processBlock` from a lock-free `AbstractFifo` (`toggleScreenNote` / `drainScreenNotes`), so they go through the normal MIDI-input path, Latch included.
- **Keyboard** (`EditorContent::keyPressed`, which takes focus when anything inside is clicked): Cmd/Ctrl+Z and +Shift for undo / redo; with the mouse over a lane, Left / Right shift it and Shift+Left / Right shift all lanes. Every other key returns false so it reaches the host. Menu hints come from `ShortcutText`.
- **Pad markings** live in compartments (`MultiStateButton::paintButton`): step pitch in the top row, trig condition in the bottom row, the Event bow-tie in what's left (its dot in the top-right corner always). Ratchet is drawn by `SubGrids` as the note-length strip above the pad split into its hits. The MIDI-in box shows ALL (+ strum arrow) for chord / strum lanes; the note map's legend row counts MIDI / clicked / latched notes.
- Right-clicking a step pad opens its trig-condition menu (`MultiStateButton::showStepMenu`); ctrl+click stays "Event cell", so check `isRightButtonDown()`, not `isPopupMenu()`.
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
