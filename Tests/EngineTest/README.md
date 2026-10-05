# EngineTest

Regression tests for the TugMidiSeq engine, and a renderer for screenshots.

It builds the plugin's own `Source/` files into a console app, drives
`processBlock` with a fake host (120 BPM, 48 kHz, 4/4: a bar is 96000 samples,
a 1/16 step about 6000) and checks the MIDI that comes out. Separate from the
Projucer build; nothing here ships.

## Run

```
cmake -S Tests/EngineTest -B Tests/EngineTest/build -DCMAKE_BUILD_TYPE=Debug
cmake --build Tests/EngineTest/build -j 10
Tests/EngineTest/build/EngineTest_artefacts/Debug/EngineTest
```

Prints one line per check and ends with `ALL PASSED` (exit code 0) or the
number of failures. `EngineTestMidiFX` runs the same checks built as the MIDI
FX AU (`MidiFX/`): no audio buses, no synth. JUCE is expected next to `2RuleProgramming`, as in the
`.jucer`; pass `-DJUCE_DIR=...` otherwise.

## Screenshots

```
EngineTest --snapshot editor.png   # the editor while a busy pattern plays
EngineTest --media ReleaseMedia/x  # the product-page images (cover, slots, steps, strum shape)
```

The version stamp in the images is read from `TugMidiSeq.jucer` when CMake
configures, so re-run the first command after a version bump.

## Writing a test

`Rig` is one plugin instance and its host: set parameters with `param`, cells
with `step`, press keys with `hold`, then `stop()` and `play (samples)`.
Playback starts on a bar line. Lane `i` sends on channel `10 + i`, which is how
`onsets` / `notes` / `perBar` tell the lanes apart. Steps are 6001 samples (the
engine rounds step lengths up) and every bar resyncs the lanes, so compare
times with `near()` and keep windows inside a bar where it matters.
