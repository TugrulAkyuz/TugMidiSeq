# TugMidiSeq 2.7.1

- **Clean on Linux virus scanners.** The Linux build carried a small helper program that JUCE embeds for its web browser component — something TugMidiSeq never uses. Some scanners read an executable tucked inside a plugin as a trojan, and a few flagged 2.7.0 because of it. The component is switched off now: the helper is gone, the plugin is 1.5 MB smaller, and scanners come back clean. Nothing about how the plugin works has changed.

# TugMidiSeq 2.7 — Tunings and MTS-ESP

- **Scala tunings.** Load a .scl scale, and a .kbm keyboard mapping if you like, from the new **Tuning** button under the lanes. The built-in synth plays it, the Octave knob moves by the tuning's period (19 keys in 19-EDO), and a mapping that leaves keys out becomes the scale lock, so steps only land on mapped keys. The tuning is saved with the project.
- **MTS-ESP master.** With a tuning loaded, TugMidiSeq becomes the MTS-ESP master: every MTS-ESP synth you send its MIDI to plays in the same tuning. MTS-ESP is free from oddsound.com; if another master is already running, TugMidiSeq waits and takes over when it closes.
- **Follows MTS-ESP too.** With no tuning of its own, TugMidiSeq follows any MTS-ESP master (MTS-ESP Mini or Suite, Surge XT as a source, another TugMidiSeq): the built-in synth, the Octave knob and the scale lock all play its tuning.

# TugMidiSeq 2.6.2

- **Duplicate input notes are back.** The same note sent several times (for example a monophonic wind controller copied to every lane) again fills that many lanes, as it did before 2.6, and releasing one copy frees one lane. 2.6 had started merging repeated notes into one. Latch still holds a latched note only once when you press it again.

# TugMidiSeq 2.6.1

- **Steady playback in Logic.** Right after a project was opened, Logic's slightly rounded song position made the lanes restart every few milliseconds: the playhead shook at the start and the lanes kept replaying their first step until the tempo was changed. Fixed in the instrument and in the MIDI FX version.

# What's New in TugMidiSeq 2.6 — October 2026 Update

TugMidiSeq grows from a step sequencer into a performance instrument: four switchable patterns, chords and strums, conditional steps, ratchets, scale lock and a lot more — while everything you made in 2.5 opens exactly as before.

## Pattern slots A–D
Keep four complete patterns in one instance and switch between them while you play. Each lane finishes its own loop before moving to the new pattern, so lanes of different lengths change over musically and nothing gets cut in half. The slot buttons blink until every lane has switched, and each waiting lane shows the rest of its way to the switch. Right-click a slot to copy or clear it, and automate the new **Pattern Slot** parameter to change patterns from your DAW.

## Chords and strums
Any lane can now play the whole held chord instead of a single note. The new **STRM** knob next to the note box sets the strum: 0 plays a block chord, turning it right strums up, turning it left strums down, and **Strum Up/Down** alternates like a guitarist's hand. Right-click the knob for **Strum Shape**: a curve that speeds up or slows down across the strings, a velocity tilt, and humanize — with a live preview.

## Steps that think
- **Trig conditions** — play a step only on certain loops (1:2 … 4:4), only the first time (1ST), depending on the step before (PRE) or the lane below (NEI), or only while **Fill** is held. Every condition has a "not" version.
- **Ratchets** — repeat a step up to 4 times within its length, for rolls and stutters.
- **Step pitch** — shift any step up to two octaves up or down (Alt+drag on a pad).
- **Scale lock** — pick a key and one of 12 scales and every lane stays in it; step pitches then move in scale degrees.
- **Step velocity** — Shift+drag on a pad, now shown in a proper rotary popup.

## Lanes with a mind of their own
- **Play direction** — Forward, Reverse, Ping-Pong or Random, per lane.
- **Mutate** — let a lane flip some of its steps every loop for patterns that keep evolving, without touching what you wrote.
- **Mute** — per lane, automatable (Shift+click the note box; a plain click still solos).
- **Euclidean fill, copy / paste, shift, clear** — all in the new lane menu, opened by clicking the lane number. Copy and paste work between plugin instances too.

## Play and see your notes
- **Latch** holds the chord after you let go of the keys.
- **Note map** — a keyboard strip under the lanes shows which held note feeds which lane, which notes are sounding, and what's latched. Click its keys to build a chord with the mouse.
- **Note box styles** tell you at a glance whether a lane's note came from MIDI, the latch or a mouse click.
- **Drag MIDI to your DAW** — drag the MIDI button onto a track to drop the pattern in as a MIDI clip (1 to 16 bars), or save it as a file.

## Work faster
- **Undo / Redo** for every edit (Cmd/Ctrl+Z, Shift+Cmd/Ctrl+Z).
- **Keyboard shortcuts** with the mouse over a lane: ← / → shift its steps, ↑ / ↓ cycle its direction, **F / R / P / X** pick Forward, Reverse, Ping-Pong or Random. Hold Shift to apply to every lane. The shortcuts keep working after you've used a menu or a combo box.
- **Note values as music notation** — speed and duration menus show note symbols, with dotted and triplet values laid out in columns.

## A clearer screen
- Each lane's playhead is now a rail and a glowing head with a short tail in the lane's colour, so you can see where every lane is, how far through its loop, and which way it's going.
- Pad markings (pitch, condition, Event) each have their own place on the pad and no longer overlap; ratchets show as a split note strip above the pad.
- The **Event** knob only lights up on lanes that actually have Event steps.
- Column captions line up with their controls, and a menu icon marks the lane numbers.

## Fixes and stability
- Strums no longer drop or cut short their last notes when they're longer than the step, and lanes on different MIDI channels no longer cut each other's notes.
- Fixed crashes in the note visualiser and on Linux, and several thread-safety issues found by stricter validation.
- Projects from earlier versions open with their pattern in slot A, and older presets load as before.
