// TugMidiSeq engine regression tests.
//
// Drives the real TugMidiSeqAudioProcessor with a fake host playhead (120 BPM,
// 48 kHz, 4/4, so a bar is 96000 samples and a 1/16 step about 6000) and
// checks the MIDI that comes out, sample by sample. Also renders the editor
// to PNG for visual checks and the product page. See README.md.
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Grids.h"

static int failures = 0;
#define CHECK(cond, msg) do { if (cond) std::cout << "  ok   " << msg << "\n"; \
                              else { std::cout << "  FAIL " << msg << "\n"; ++failures; } } while (0)

//==============================================================================
struct FakePlayHead : AudioPlayHead
{
    double sr = 48000, bpm = 120;
    int64 pos = 0;
    bool playing = false;
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo i;
        i.setBpm (bpm);
        i.setIsPlaying (playing);
        i.setTimeInSamples (pos);
        i.setPpqPosition ((double) pos / (sr * 60.0 / bpm));
        i.setTimeSignature (TimeSignature { 4, 4 });
        return i;
    }
};

struct OutNote { int64 t; int lane; int note; int vel; };

// One plugin instance and its host. Lane i sends on channel 10 + i (the
// default routing), which is how notes are told apart.
struct Rig
{
    static constexpr int block = 512;
    static constexpr int64 bar = 96000;   // 120 bpm, 48 kHz, 4/4

    std::unique_ptr<TugMidiSeqAudioProcessor> p = std::make_unique<TugMidiSeqAudioProcessor>();
    FakePlayHead ph;
    AudioBuffer<float> buf { 2, block };
    std::vector<std::pair<int64, MidiMessage>> pending;
    std::vector<OutNote> out, offs;
    int64 playStart = -1;

    Rig()
    {
        p->setPlayHead (&ph);
        p->prepareToPlay (48000, block);
    }
    void param (const String& id, float v) { p->setParamValue (id, v); }
    void step (int lane, int s, int state = 1) { param ("block" + String (lane) + String (s), (float) state); }
    void in (int64 t, MidiMessage m) { pending.push_back ({ t, m }); }
    // keys pressed now (before the next block)
    void hold (std::initializer_list<int> notes, uint8 vel = 100)
    {
        int64 t = ph.pos;
        for (int n : notes) in (t++, MidiMessage::noteOn (1, n, vel));
    }

    void run (int64 samples)
    {
        for (int64 done = 0; done < samples; done += block)
        {
            MidiBuffer mb;
            for (auto& [t, m] : pending)
                if (t >= ph.pos && t < ph.pos + block)
                    mb.addEvent (m, (int) (t - ph.pos));
            buf.clear();
            p->processBlock (buf, mb);
            if (ph.playing)
                for (const auto meta : mb)
                {
                    auto m = meta.getMessage();
                    if (m.isNoteOn())
                        out.push_back ({ ph.pos + meta.samplePosition, m.getChannel() - 10, m.getNoteNumber(), m.getVelocity() });
                    else if (m.isNoteOff())
                        offs.push_back ({ ph.pos + meta.samplePosition, m.getChannel() - 10, m.getNoteNumber(), 0 });
                }
            ph.pos += block;
        }
    }
    void stop (int64 samples = 4 * block) { ph.playing = false; run (samples); }
    int64 startAt() const { return ((ph.pos + bar - 1) / bar) * bar; }   // playback starts on a bar line
    void play (int64 samples)
    {
        if (! ph.playing)
        {
            ph.pos = startAt();
            ph.playing = true;
            playStart = ph.pos;
        }
        run (samples);
    }

    // note-ons of a lane, as times from the start of playback, in [from, to)
    std::vector<int> onsets (int lane, int64 from = 0, int64 to = 1LL << 40) const
    {
        std::vector<int> t;
        for (auto& n : out)
            if (n.lane == lane && n.t - playStart >= from && n.t - playStart < to)
                t.push_back ((int) (n.t - playStart));
        return t;
    }
    std::vector<int> notes (int lane, int64 from = 0, int64 to = 1LL << 40) const
    {
        std::vector<int> v;
        for (auto& n : out)
            if (n.lane == lane && n.t - playStart >= from && n.t - playStart < to)
                v.push_back (n.note);
        return v;
    }
    std::vector<int> perBar (int lane, int bars) const
    {
        std::vector<int> c ((size_t) bars, 0);
        for (auto& n : out)
            if (n.lane == lane && n.t >= playStart)
            {
                auto b = (n.t - playStart) / bar;
                if (b < bars) c[(size_t) b]++;
            }
        return c;
    }
    // when the note-off for the note-on at index k of `lane` came, or -1
    int64 offAfter (int lane, int note, int64 onAt) const
    {
        for (auto& o : offs)
            if (o.lane == lane && o.note == note && o.t > onAt) return o.t;
        return -1;
    }
    int cellParam (int lane, int s) const
    {
        return (int) *p->valueTreeState.getRawParameterValue ("block" + String (lane) + String (s));
    }
};

static String str (const std::vector<int>& v)
{
    String s = "[";
    for (size_t i = 0; i < v.size(); i++) s << (i ? "," : "") << v[i];
    return s + "]";
}

static bool near (int a, int b, int tolerance = 20) { return std::abs (a - b) <= tolerance; }

//==============================================================================
static void testBasics()
{
    std::cout << "Basics\n";
    {
        Rig r;
        for (int s : { 0, 4, 8, 12 }) r.step (0, s);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar);
        auto t = r.onsets (0, 0, Rig::bar);
        CHECK (t.size() == 4 && t[0] == 0 && near (t[1], 24000) && near (t[2], 48000) && near (t[3], 72000),
               "four 1/16 cells play on their steps " + str (t));
        CHECK (r.notes (0, 0, Rig::bar) == std::vector<int> ({ 60, 60, 60, 60 }), "lane 1 plays the lowest held note");
        CHECK (! r.out.empty() && r.out[0].vel == 100, "fixed velocity: lane 90 x step 100 -> 100, got " + String (r.out.empty() ? -1 : r.out[0].vel));
    }
    {
        Rig r;
        r.step (0, 0); r.step (1, 0); r.step (2, 0);
        r.hold ({ 67, 60, 64 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (r.notes (0) == std::vector<int> ({ 60 }) && r.notes (1) == std::vector<int> ({ 64 }) && r.notes (2) == std::vector<int> ({ 67 }),
               "held notes are spread low to high over the lanes");
    }
    {
        Rig r;
        r.step (0, 0); r.param ("velGridButton00", 50);
        r.param ("Octave0", 1);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (! r.out.empty() && r.out[0].note == 72 && r.out[0].vel == 50, "octave +1 and a step velocity of 50");
    }
    {
        Rig r;
        r.step (0, 0); r.step (0, 1, 2);   // step 2 is an Event cell
        r.param ("Event0", 100);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar);
        CHECK (r.onsets (0, 0, Rig::bar).size() == 2, "an Event cell at 100 % always plays");
    }
}

//==============================================================================
static void testLatchAndKeyboard()
{
    std::cout << "Latch / on-screen keyboard\n";
    {
        Rig r;
        for (int s = 0; s < 16; s++) r.step (0, s);
        r.hold ({ 60 });
        r.stop();
        r.in (r.startAt() + Rig::bar, MidiMessage::noteOff (1, 60));
        r.play (2 * Rig::bar);
        auto c = r.perBar (0, 2);
        CHECK (c[0] == 16 && c[1] == 0, "latch off: plays while held, stops on release " + str (c));
    }
    {
        Rig r;
        r.param ("latch", 1);
        for (int s = 0; s < 16; s++) r.step (0, s);
        r.hold ({ 60 });
        r.in (r.ph.pos + 100, MidiMessage::noteOff (1, 60));
        r.stop();
        r.play (2 * Rig::bar);
        CHECK (r.perBar (0, 2) == std::vector<int> ({ 16, 16 }), "latch on: keeps playing after release");
    }
    {
        Rig r;
        r.param ("latch", 1);
        r.step (0, 0);
        r.hold ({ 60 });
        r.in (r.ph.pos + 100, MidiMessage::noteOff (1, 60));
        r.in (r.ph.pos + 2000, MidiMessage::noteOn (1, 62, (uint8) 100));   // a new key after all were released
        r.stop();
        r.play (Rig::bar / 4);
        CHECK (r.notes (0) == std::vector<int> ({ 62 }), "latch: the next key replaces the latched chord");
    }
    {
        Rig r;
        r.step (0, 0);
        r.p->toggleScreenNote (62);
        r.stop();
        CHECK (r.p->isScreenNote (62) && r.p->getHeldNoteCount() == 1, "a clicked key is held");
        r.play (Rig::bar / 4);
        CHECK (r.notes (0) == std::vector<int> ({ 62 }), "and played like a MIDI key");
        r.p->releaseScreenNotes();
        r.stop();
        CHECK (r.p->getHeldNoteCount() == 0 && ! r.p->isScreenNote (62), "right-click releases every clicked key");
    }
}

//==============================================================================
static void condCase (int cond, std::vector<int> expected)
{
    Rig r;
    r.step (0, 0);
    r.p->setStepCond (0, 0, cond);
    r.hold ({ 60 });
    r.stop();
    r.play (4 * Rig::bar);
    auto got = r.perBar (0, 4);
    CHECK (got == expected, "cond " + trigCondNames[cond] + " -> " + str (got) + " expected " + str (expected));
}

static void testConditions()
{
    std::cout << "Trig conditions\n";
    condCase (CondNone,    { 1, 1, 1, 1 });
    condCase (Cond1of2,    { 1, 0, 1, 0 });
    condCase (Cond2of2,    { 0, 1, 0, 1 });
    condCase (Cond1of3,    { 1, 0, 0, 1 });
    condCase (Cond3of4,    { 0, 0, 1, 0 });
    condCase (CondFirst,   { 1, 0, 0, 0 });
    condCase (CondNotFirst,{ 0, 1, 1, 1 });
    {
        Rig r;
        r.step (0, 0); r.p->setStepCond (0, 0, Cond1of2);
        r.step (0, 4); r.p->setStepCond (0, 4, CondPre);
        r.step (0, 8); r.p->setStepCond (0, 8, CondNotPre);
        r.hold ({ 60 });
        r.stop(); r.play (4 * Rig::bar);
        CHECK (r.perBar (0, 4) == std::vector<int> ({ 2, 1, 2, 1 }), "PRE follows the condition before it, !PRE the opposite " + str (r.perBar (0, 4)));
    }
    {
        Rig r;
        r.step (0, 0); r.p->setStepCond (0, 0, Cond1of2);
        r.step (1, 4); r.p->setStepCond (1, 4, CondNei);
        r.hold ({ 60, 64 });
        r.stop(); r.play (4 * Rig::bar);
        CHECK (r.perBar (1, 4) == std::vector<int> ({ 1, 0, 1, 0 }), "NEI follows the lane below " + str (r.perBar (1, 4)));
    }
    {
        Rig r;
        r.step (0, 0); r.p->setStepCond (0, 0, CondFill);
        r.step (0, 8); r.p->setStepCond (0, 8, CondNotFill);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar - 2 * Rig::block);
        r.param ("fill", 1);   // held from just before bar 2
        r.play (Rig::bar);
        auto t = r.onsets (0, 0, 2 * Rig::bar);
        CHECK (t.size() == 2 && near (t[0], 48000, 30) && near (t[1], 96000),
               "!FILL plays while Fill is up (bar 1, step 9), FILL while it's held (bar 2, step 1) " + str (t));
    }
}

//==============================================================================
static void testStepTools()
{
    std::cout << "Ratchet / step pitch / scale\n";
    {
        Rig r;
        r.step (0, 0); r.p->setStepRatchet (0, 0, 3);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar / 4);
        auto t = r.onsets (0, 0, 6000);
        CHECK (t.size() == 3 && t[0] == 0 && near (t[1], 2000) && near (t[2], 4000), "x3 ratchet: three hits across the step " + str (t));
        bool gated = true;
        for (int on : t) { auto off = r.offAfter (0, 60, r.playStart + on); gated &= off > 0 && off - (r.playStart + on) <= 1500 + 2; }
        CHECK (gated, "each ratchet hit is gated to 3/4 of its share");
    }
    {
        Rig r;
        CHECK (r.p->pitchedNote (60, 7) == 67, "scale off: step pitch counts semitones");
        r.param ("scaleKey", 0); r.param ("scaleType", 1);   // C major
        CHECK (r.p->pitchedNote (61, 0) == 60, "scale lock: C# snaps to C (ties go down)");
        CHECK (r.p->pitchedNote (60, 2) == 64 && r.p->pitchedNote (60, -1) == 59, "scale on: step pitch counts scale degrees");
        r.step (0, 0); r.p->setStepPitch (0, 0, 4);
        r.hold ({ 62 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (r.notes (0) == std::vector<int> ({ 69 }), "D + 4 degrees in C major plays A");
    }
}

//==============================================================================
static void testScales()
{
    std::cout << "Scales\n";
    bool tableOk = scaleTypeNames.size() == (int) scaleIntervals.size();
    for (size_t i = 1; i < scaleIntervals.size(); i++)
    {
        const auto& sc = scaleIntervals[i];
        tableOk &= ! sc.empty() && sc[0] == 0 && std::is_sorted (sc.begin(), sc.end()) && sc.back() < 12
                   && std::adjacent_find (sc.begin(), sc.end()) == sc.end();
    }
    CHECK (tableOk, "every scale: a name, sorted pitch classes from 0, no repeats (" + String (scaleTypeNames.size()) + " entries)");
    CHECK (scaleTypeNames[1] == "Major" && scaleTypeNames[9] == "Mel. Minor" && scaleTypeNames[12] == "Blues",
           "the scales of 2.6 keep their numbers (projects and presets store them)");
    bool groupsOk = true;
    for (size_t g = 0; g < std::size (scaleGroups); g++)
        groupsOk &= scaleGroups[g].first > 0 && scaleGroups[g].first < scaleTypeNames.size()
                    && (g == 0 || scaleGroups[g].first > scaleGroups[g - 1].first);
    CHECK (groupsOk && scaleGroups[0].first == 1, "menu groups cover the list in order");

    auto scale = [] (const String& name)
    {
        Rig r;
        r.param ("scaleKey", 0);
        r.param ("scaleType", (float) scaleTypeNames.indexOf (name));
        return r;
    };
    {
        auto r = scale ("Hirajoshi");   // C D Eb G Ab
        CHECK (r.p->pitchedNote (61, 0) == 60 && r.p->pitchedNote (64, 0) == 63 && r.p->pitchedNote (66, 0) == 67,
               "Hirajoshi: C# -> C, E -> Eb, F# -> G");
        CHECK (r.p->pitchedNote (60, 1) == 62 && r.p->pitchedNote (60, 5) == 72 && r.p->pitchedNote (60, -1) == 56,
               "Hirajoshi: five degrees to the octave, down past C to Ab");
    }
    {
        auto r = scale ("Messiaen 7");   // ten notes
        CHECK (r.p->pitchedNote (60, 10) == 72 && r.p->pitchedNote (60, 4) == 65 && r.p->pitchedNote (64, 0) == 63,
               "Messiaen 7: ten degrees to the octave, the 4th degree is F, E snaps down to Eb");
    }
    {
        auto r = scale (String (juce::CharPointer_UTF8 ("K\xc3\xbcrdi")));
        r.param ("scaleKey", 2);   // D Kurdi = D Eb F G A Bb C
        CHECK (r.p->pitchedNote (62, 1) == 63 && r.p->pitchedNote (62, 2) == 65, "Kurdi on D: Eb then F");
    }
    {
        Rig a;
        a.param ("scaleType", (float) scaleTypeNames.indexOf ("Hicazkar"));
        MemoryBlock state; a.p->getStateInformation (state);
        Rig b; b.p->setStateInformation (state.getData(), (int) state.getSize());
        CHECK (scaleTypeNames[b.p->getScaleType()] == "Hicazkar", "a new scale survives a project round trip");
        TugMidiSeqProgram prog ("t");
        auto preset = b.p->varToPreset (b.p->presetToVar (prog));
        preset.scaleType = scaleTypeNames.indexOf ("Pelog");
        CHECK (b.p->varToPreset (b.p->presetToVar (preset)).scaleType == preset.scaleType, "and a preset round trip");
    }
}

static void testLanes()
{
    std::cout << "Direction / mutate / mute / solo\n";
    {
        Rig r;
        r.step (0, 0);
        r.param ("Direction0", DirReverse);
        r.hold ({ 60 });
        r.stop(); r.play (Rig::bar);
        auto t = r.onsets (0);
        CHECK (t.size() == 1 && near (t[0], 15 * 6001, 30), "reverse: step 1 plays last " + str (t));
    }
    {
        Rig r;
        r.step (0, 15);
        r.param ("Direction0", DirPingPong);
        r.hold ({ 60 });
        r.stop(); r.play (2 * Rig::bar);
        auto t = r.onsets (0);
        CHECK (t.size() == 2 && near (t[0], 15 * 6001, 30) && near (t[1], 96000, 30), "ping-pong: out and back " + str (t));
    }
    {
        Rig r;
        for (int s : { 0, 4, 8, 12 }) r.step (0, s);
        r.param ("Direction0", DirRandom);
        r.hold ({ 60 });
        r.stop(); r.play (4 * Rig::bar);
        bool onCells = true;
        for (int t : r.onsets (0)) onCells &= ((t % Rig::bar) % 6001) < 40;   // only at step starts (the bar resyncs the lane)
        CHECK (! r.onsets (0).empty() && onCells, "random: plays, always on a step");
    }
    {
        Rig r;
        r.step (0, 0);
        r.param ("Mutate0", 100);
        r.hold ({ 60 });
        r.stop(); r.play (3 * Rig::bar);
        auto c = r.perBar (0, 3);
        CHECK (c[0] == 1 && c[1] == 15 && c[2] == 1, "mutate 100 %: each loop flips every step " + str (c));
        CHECK (r.cellParam (0, 0) == 1 && r.cellParam (0, 1) == 0, "the written pattern is untouched");
    }
    {
        Rig r;
        r.step (0, 0); r.p->setStepCond (0, 0, Cond1of2);
        r.step (1, 4); r.p->setStepCond (1, 4, CondNei);
        r.param ("Mute0", 1);
        r.hold ({ 60, 64 });
        r.stop(); r.play (4 * Rig::bar);
        CHECK (r.onsets (0).empty(), "a muted lane is silent");
        CHECK (r.perBar (1, 4) == std::vector<int> ({ 1, 0, 1, 0 }), "its conditions still count for NEI");
    }
    {
        Rig r;
        r.step (0, 0); r.step (1, 0);
        r.p->setGridSolo (1);
        r.hold ({ 60, 64 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (r.onsets (0).empty() && r.onsets (1).size() == 1, "solo: only the soloed lane plays");
    }
}

//==============================================================================
static std::vector<std::pair<int,int>> firstStep (Rig& r, int lane = 0)
{
    std::vector<std::pair<int,int>> res;
    for (auto& n : r.out)
        if (n.lane == lane && n.t >= r.playStart && n.t - r.playStart < 6000)
            res.push_back ({ (int) (n.t - r.playStart), n.note });
    return res;
}

static void testStrum()
{
    std::cout << "Chord / strum\n";
    struct Case { int spread; std::vector<std::pair<int,int>> expected; const char* what; };
    for (auto& c : std::vector<Case> {
            {  20, { {0,60}, {960,64}, {1920,67} }, "strum +20: up, 20 ms apart" },
            { -20, { {0,67}, {960,64}, {1920,60} }, "strum -20: down" },
            {   0, { {0,60}, {0,64}, {0,67} },      "strum 0: a block chord" } })
    {
        Rig r; r.step (0, 0);
        r.param ("PlayMode0", PlayStrum); r.param ("Spread0", (float) c.spread);
        r.hold ({ 64, 60, 67 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (firstStep (r) == c.expected, c.what);
    }
    for (int spread : { 20, -20 })
    {
        Rig r; r.step (0, 0); r.step (0, 8);
        r.param ("PlayMode0", PlayStrumUpDown); r.param ("Spread0", (float) spread);
        r.hold ({ 60, 64, 67 });
        r.stop(); r.play (Rig::bar);
        const auto first = r.notes (0, 0, 1), second = r.notes (0, 47000, 49000);
        const auto expected = spread > 0 ? std::vector<int> ({ 60, 67 }) : std::vector<int> ({ 67, 60 });
        CHECK (first.size() == 1 && ! second.empty() && std::vector<int> ({ first[0], second[0] }) == expected,
               "up/down " + String (spread) + ": the second hit goes the other way");
    }
    {
        Rig r; r.step (0, 0); r.p->setStepRatchet (0, 0, 2);
        r.param ("PlayMode0", PlayStrum); r.param ("Spread0", 0);
        r.hold ({ 60, 64, 67 });
        r.stop(); r.play (Rig::bar / 4);
        CHECK (r.onsets (0, 0, 6000).size() == 6, "a x2 ratchet replays the whole chord");
    }
    // a strum longer than the time to the next hit is squeezed: every note
    // plays, rings at least a quarter of the note length, and stops in time
    for (int spread : { 35, 100, -100 })
    {
        Rig r;
        for (int st = 0; st < 16; st++) r.step (0, st);
        r.param ("PlayMode0", PlayStrum); r.param ("Spread0", (float) spread);
        r.hold ({ 60, 64, 67, 71, 74 });
        r.stop(); r.play (Rig::bar / 4);
        bool ok = true; String why;
        for (int stepNo = 0; stepNo < 3; stepNo++)
        {
            int count = 0;
            for (auto& n : r.out)
            {
                const auto t = n.t - r.playStart;
                if (n.lane != 0 || t < stepNo * 6001 || t >= (stepNo + 1) * 6001) continue;   // steps are 6001 samples
                count++;
                const auto off = r.offAfter (0, n.note, n.t) - r.playStart;
                if (off < 0 || off - t < 6000 / 4 - 2 || off > (stepNo + 1) * 6001) { ok = false; why << " " << n.note << "@" << (int) t << " off " << (int) off; }
            }
            if (count != 5) { ok = false; why << " step " << stepNo << ": " << count << " notes"; }
        }
        CHECK (ok, "strum " + String (spread) + " ms on back-to-back 1/16 steps keeps all 5 notes" + why);
    }
    {
        Rig r; r.step (0, 0);
        r.param ("PlayMode0", PlayStrum); r.param ("Spread0", 35); r.param ("Dur0", 16);   // 1/32 notes
        r.hold ({ 60, 64, 67, 71, 74 });
        r.stop(); r.play (Rig::bar / 4);
        auto t = r.onsets (0, 0, 12000);
        CHECK (t == std::vector<int> ({ 0, 1680, 3360, 5040, 6720 }), "with room before the next hit the knob's width is kept " + str (t));
    }
    {
        Rig r;
        r.step (0, 0); r.step (0, 2); r.step (1, 0);
        r.param ("PlayMode1", PlayStrum); r.param ("Spread1", 0); r.param ("Dur1", 7);   // lane 2: 1/4 notes
        r.hold ({ 60, 64, 67 });
        r.stop(); r.play (Rig::bar / 2);
        const auto off = r.offAfter (1, 60, r.playStart) - r.playStart;
        CHECK (off >= 23000, "lane 1's C on another channel leaves lane 2's C ringing (off @" + String ((int) off) + ")");
    }

    std::cout << "Strum shape\n";
    auto strum = [] (int shape, int tension, int vel, int human, std::vector<int>* velocities = nullptr)
    {
        Rig r; r.step (0, 0);
        r.param ("PlayMode0", PlayStrum); r.param ("Spread0", 20);
        r.param ("StrumShape0", (float) shape); r.param ("StrumTension0", (float) tension);
        r.param ("StrumVel0", (float) vel); r.param ("StrumHuman0", (float) human);
        r.param ("GlobalInOrFixedVel", 1);   // play the incoming velocity, so only the tilt changes it
        r.hold ({ 60, 64, 67, 71, 74 }, 80);
        r.stop(); r.play (Rig::bar / 4);
        if (velocities != nullptr)
            for (auto& n : r.out) if (n.lane == 0 && n.t - r.playStart < 6000) velocities->push_back (n.vel);
        return r.onsets (0, 0, 6000);
    };
    {
        auto lin = strum (StrumLinear, 80, 0, 0);
        CHECK (lin == std::vector<int> ({ 0, 960, 1920, 2880, 3840 }), "linear ignores tension " + str (lin));
        auto acc = strum (StrumCurve, 100, 0, 0), dec = strum (StrumCurve, -100, 0, 0);
        bool shrinking = acc.size() == 5, growing = dec.size() == 5;
        for (int k = 2; k < 5 && shrinking; k++) shrinking = acc[k] - acc[k - 1] < acc[k - 1] - acc[k - 2];
        for (int k = 2; k < 5 && growing; k++)   growing   = dec[k] - dec[k - 1] > dec[k - 1] - dec[k - 2];
        CHECK (shrinking && acc.back() == 3840, "curve +: gaps shrink, same total length " + str (acc));
        CHECK (growing && dec.back() == 3840, "curve -: gaps grow " + str (dec));
        std::vector<int> v;
        strum (StrumLinear, 0, 100, 0, &v);
        bool rising = v.size() == 5;
        for (int k = 1; k < 5 && rising; k++) rising = v[k] > v[k - 1];
        CHECK (rising && v[0] == 80, "velocity tilt +: each string louder " + str (v));
        bool ordered = true;
        for (int run = 0; run < 5; run++)
        {
            auto h = strum (StrumLinear, 0, 0, 100);
            ordered &= h.size() == 5 && std::is_sorted (h.begin(), h.end()) && h[0] == 0;
        }
        CHECK (ordered, "humanize jitters a strum without reordering it");
    }
}

//==============================================================================
static void testStrumTiming()
{
    std::cout << "Strum timing (Time / Sync)\n";
    auto strumOnsets = [] (double bpm, bool sync, float value, std::vector<int>* notes = nullptr)
    {
        Rig r;
        r.ph.bpm = bpm;
        r.step (0, 0);
        r.param ("PlayMode0", PlayStrum);
        r.param ("StrumSync0", sync ? 1.0f : 0.0f);
        r.param (sync ? "StrumDiv0" : "Spread0", value);
        r.hold ({ 60, 64, 67 });
        r.stop();
        const int64 barLen = (int64) std::llround (48000.0 * 240.0 / bpm);
        r.ph.pos = ((r.ph.pos + barLen - 1) / barLen) * barLen;   // start on a bar line at this tempo
        r.play (barLen / 2);
        if (notes != nullptr) *notes = r.notes (0, 0, barLen / 2);
        return r.onsets (0, 0, barLen / 2);
    };
    const int div32 = strumDivNames.indexOf ("1/32");
    auto t = strumOnsets (120, true, (float) div32);
    CHECK (t == std::vector<int> ({ 0, 3000, 6000 }), "Sync 1/32 at 120 BPM: an eighth of a beat between notes " + str (t));
    t = strumOnsets (60, true, (float) div32);
    CHECK (t == std::vector<int> ({ 0, 6000, 12000 }), "at 60 BPM the same strum is twice as wide " + str (t));
    std::vector<int> notes;
    strumOnsets (120, true, (float) strumDivNames.indexOf ("-1/32"), &notes);
    CHECK (notes == std::vector<int> ({ 67, 64, 60 }), "Sync -1/32 strums down");
    t = strumOnsets (120, true, (float) strumDivCentre);
    CHECK (t == std::vector<int> ({ 0, 0, 0 }), "Sync 0 is a block chord");
    t = strumOnsets (120, false, 200);
    CHECK (t == std::vector<int> ({ 0, 9600, 19200 }), "Time goes up to 250 ms: 200 ms " + str (t));
    {
        Rig r;
        CHECK (r.p->getStrumDirection (0) == 1, "default: Time, +20 ms, up");
        r.param ("StrumSync0", 1); r.param ("StrumDiv0", (float) strumDivNames.indexOf ("-1/16T"));
        CHECK (r.p->getStrumDirection (0) == -1, "Sync below the middle strums down");
    }
    {   // squeezed: back-to-back 1/16 steps leave no room for a 100 ms strum; a sparse lane does
        Rig dense, sparse;
        for (int st = 0; st < 16; st++) dense.step (0, st);
        sparse.step (0, 0);
        for (auto* r : { &dense, &sparse })
        {
            r->param ("PlayMode0", PlayStrum); r->param ("Spread0", 100);
            r->hold ({ 60, 64, 67, 71, 74 });
            r->stop(); r->play (Rig::bar / 4);
        }
        CHECK (dense.p->isStrumSqueezed (0) && ! sparse.p->isStrumSqueezed (0), "the squeezed flag shows when the knob can't be had");
        dense.stop();
        CHECK (! dense.p->isStrumSqueezed (0), "and clears on stop");
    }
    {
        Rig r;
        TugMidiSeqProgram prog ("t");
        auto base = r.p->varToPreset (r.p->presetToVar (prog));
        base.strumSync[2] = 1; base.strumDiv[2] = strumDivNames.indexOf ("1/16T"); base.spread[1] = -230;
        auto back = r.p->varToPreset (r.p->presetToVar (base));
        CHECK (back.strumSync[2] == 1 && back.strumDiv[2] == base.strumDiv[2] && back.spread[1] == -230, "preset round trip: Sync, the note value, a wide spread");
        auto old = r.p->presetToVar (base);
        if (auto* o = old.getDynamicObject()) { o->removeProperty ("StrumSync2"); o->removeProperty ("StrumDiv2"); }
        auto loaded = r.p->varToPreset (old);
        CHECK (loaded.strumSync[2] == 0 && loaded.strumDiv[2] == strumDivNames.indexOf ("1/64"), "an older preset: Time, and 1/64 ready for Sync");
    }
}

static void testEditsAndUndo()
{
    std::cout << "Lane edits / undo\n";
    {
        Rig r;
        r.step (0, 3); r.p->setStepCond (0, 3, Cond2of2); r.p->setStepPitch (0, 3, 5);
        r.p->copyLane (0);
        r.p->pasteLane (1);
        CHECK (r.cellParam (1, 3) == 1 && r.p->getStepCond (1, 3) == Cond2of2 && r.p->getStepPitch (1, 3) == 5, "copy / paste a lane, step data included");
        r.p->shiftLane (1, 1);
        CHECK (r.cellParam (1, 3) == 0 && r.cellParam (1, 4) == 1 && r.p->getStepCond (1, 4) == Cond2of2, "shift right moves cells and their data");
        r.p->undo();
        CHECK (r.cellParam (1, 3) == 1 && r.p->getStepCond (1, 3) == Cond2of2, "undo the shift");
        r.p->clearLane (1);
        CHECK (r.cellParam (1, 3) == 0 && r.p->getStepPitch (1, 3) == 0, "clear lane");
    }
    {
        Rig r;
        r.p->euclidLane (2, 4, 0);
        std::vector<int> on; for (int s = 0; s < 16; s++) if (r.cellParam (2, s)) on.push_back (s);
        CHECK (on == std::vector<int> ({ 0, 4, 8, 12 }), "euclid 4 of 16 " + str (on));
        r.p->euclidLane (2, 3, 1);
        on.clear(); for (int s = 0; s < 16; s++) if (r.cellParam (2, s)) on.push_back (s);
        CHECK (on.size() == 3 && on[0] == 1, "euclid 3 of 16 turned by 1 " + str (on));
    }
    {   // regression: an edit the APVTS hadn't flushed yet landed in the next undo step
        Rig r;
        r.step (0, 0);
        r.p->shiftAllLanes (1);
        r.p->undo();
        CHECK (r.cellParam (0, 0) == 1 && r.cellParam (0, 1) == 0, "undo of shift-all restores an unflushed edit");
    }
    {
        Rig r;
        r.p->setStepCondUndoable (0, 5, CondFirst);
        r.p->setStepRatchetUndoable (0, 5, 3);
        r.p->undo();
        CHECK (r.p->getStepRatchet (0, 5) == 1, "undo a ratchet");
        r.p->undo();
        CHECK (r.p->getStepCond (0, 5) == CondNone, "undo a condition");
        r.p->redo();
        CHECK (r.p->getStepCond (0, 5) == CondFirst, "redo it");
    }
    {
        Rig r;
        r.p->cycleLaneDirection (2, 1, false);
        CHECK (r.p->getDirection (2) == DirReverse && r.p->getDirection (1) == DirForward, "down: lane 3 Forward -> Reverse, others untouched");
        r.p->cycleLaneDirection (2, 1, false); r.p->cycleLaneDirection (2, 1, false);
        CHECK (r.p->getDirection (2) == DirRandom, "twice more: Random");
        r.p->cycleLaneDirection (2, 1, false);
        CHECK (r.p->getDirection (2) == DirForward, "wraps to Forward");
        r.p->cycleLaneDirection (2, -1, false);
        CHECK (r.p->getDirection (2) == DirRandom, "up from Forward: Random");
        r.p->setDirectionOfLanes (3, DirPingPong, false);
        CHECK (r.p->getDirection (3) == DirPingPong && r.p->getDirection (4) == DirForward, "P on lane 4: Ping-Pong only there");
        r.p->setDirectionOfLanes (0, DirReverse, true);
        bool all = true; for (int i = 0; i < numOfLine; i++) all &= r.p->getDirection (i) == DirReverse;
        CHECK (all, "shift + key: every lane");
        r.p->undo();
        CHECK (r.p->getDirection (3) == DirPingPong && r.p->getDirection (2) == DirRandom, "one undo restores every lane");
    }
}

//==============================================================================
static void testPatternSlots()
{
    std::cout << "Pattern slots\n";
    {   // slots are separate patterns; stopped, a request switches at once and the pads follow
        Rig r;
        r.step (0, 0); r.p->setStepCond (0, 0, Cond1of2);
        r.p->requestSlot (1); r.stop (512);
        CHECK (r.p->getActiveSlot (0) == 1 && r.p->getActiveSlot (4) == 1, "stopped: every lane moves to B at once");
        r.p->mirrorActiveSlots();
        CHECK (r.cellParam (0, 0) == 0 && r.p->getStepCond (0, 0) == CondNone, "B starts empty: pad and condition");
        CHECK (! r.p->canUndo(), "switching clears the undo history (its entries name pads, not slots)");
        r.step (0, 4);
        r.p->requestSlot (0); r.stop (512); r.p->mirrorActiveSlots();
        CHECK (r.cellParam (0, 0) == 1 && r.cellParam (0, 4) == 0 && r.p->getStepCond (0, 0) == Cond1of2, "back on A: A's pads, B's edit stayed in B");
        CHECK (! r.p->isSlotEmpty (1) && r.p->isSlotEmpty (2), "B holds its step, C is empty");
    }
    {   // playing: each lane switches when its own loop starts again
        Rig r;
        r.param ("GridNum1", 12);
        r.step (0, 0); r.step (1, 0);
        r.p->requestSlot (1); r.stop (512); r.p->mirrorActiveSlots();
        r.step (0, 8); r.step (1, 6);
        r.p->requestSlot (0); r.stop (512); r.p->mirrorActiveSlots();
        r.hold ({ 60, 64 });
        r.stop();
        r.play (Rig::bar / 4);
        r.p->requestSlot (1);
        r.play (Rig::bar / 4);
        CHECK (r.p->isLaneWaitingForSlot (0) && r.p->isLaneWaitingForSlot (1), "both lanes wait for their loop end");
        r.play (Rig::bar * 2);
        auto l0 = r.onsets (0, 0, 2 * Rig::bar), l1 = r.onsets (1, 0, 2 * Rig::bar);
        CHECK (l0.size() == 2 && l0[0] == 0 && near (l0[1], 144000), "16-step lane: A's step, then B's from the next bar " + str (l0));
        // lane 2 loops at 72012: B from there (A's step 1 isn't played again), then the 1-bar resync restarts it at 96000
        CHECK (l1.size() == 2 && l1[0] == 0 && near (l1[1], 132006), "12-step lane switches at its own loop end " + str (l1));
        CHECK (! r.p->isLaneWaitingForSlot (0) && ! r.p->isLaneWaitingForSlot (1), "nobody waiting afterwards");
    }
    {   // ping-pong waits for the way back
        Rig r;
        r.param ("Direction0", DirPingPong);
        r.p->requestSlot (1); r.stop (512); r.p->mirrorActiveSlots();
        r.step (0, 4);
        r.p->requestSlot (0); r.stop (512); r.p->mirrorActiveSlots();
        r.step (0, 0);
        r.hold ({ 60 });
        r.stop();
        r.play (Rig::bar / 4);
        r.p->requestSlot (1);
        r.play (3 * Rig::bar);
        auto t = r.onsets (0, 0, 3 * Rig::bar);
        // bar 1 out, bar 2 back (A's step 1 last, ~186000), B from bar 3 (its step 5 ~216000)
        CHECK (t.size() == 3 && t[0] == 0 && near (t[1], 186015) && near (t[2], 216004, 30), "ping-pong lane switches after the way back " + str (t));
    }
    {   // saved with the project, per-lane active slot included
        Rig a;
        a.step (0, 3); a.p->setStepPitch (0, 3, 5);
        a.p->requestSlot (2); a.stop (512); a.p->mirrorActiveSlots();
        a.step (2, 7); a.p->setStepRatchet (2, 7, 3);
        a.param ("velGridButton27", 40);
        MemoryBlock state; a.p->getStateInformation (state);
        Rig b; b.p->setStateInformation (state.getData(), (int) state.getSize());
        b.p->mirrorActiveSlots();
        CHECK (b.p->getRequestedSlot() == 2 && b.p->getActiveSlot (2) == 2, "restored on slot C");
        CHECK (b.cellParam (2, 7) == 1 && b.p->getStepRatchet (2, 7) == 3 && b.p->stepVelAt (2, 7) == 40, "C's cell, ratchet and step velocity");
        b.p->requestSlot (0); b.stop (512); b.p->mirrorActiveSlots();
        CHECK (b.cellParam (0, 3) == 1 && b.p->getStepPitch (0, 3) == 5 && b.cellParam (2, 7) == 0, "A's pattern came back too");
    }
    {   // a project from before the slots: its pattern becomes slot A
        Rig a;
        a.step (1, 5); a.p->setStepCond (1, 5, CondFirst);
        MemoryBlock state; a.p->getStateInformation (state);
        auto xml = AudioProcessor::getXmlFromBinary (state.getData(), (int) state.getSize());
        if (auto* n = xml->getChildByName ("patternSlots")) xml->removeChildElement (n, true);
        MemoryBlock legacy; AudioProcessor::copyXmlToBinary (*xml, legacy);
        Rig b; b.p->requestSlot (3); b.stop (512);
        b.p->setStateInformation (legacy.getData(), (int) legacy.getSize());
        CHECK (b.p->getRequestedSlot() == 0 && b.p->getActiveSlot (1) == 0, "old project opens on slot A");
        CHECK (b.p->cellAt (1, 5) == 1 && b.p->getStepCond (1, 5) == CondFirst && b.p->isSlotEmpty (3), "with its pattern in A, D left empty");
    }
    {   // copy / clear are undoable
        Rig r;
        r.step (3, 2); r.p->setStepPitch (3, 2, -7);
        r.p->copySlot (0, 3);
        CHECK (! r.p->isSlotEmpty (3), "copy A to D");
        r.p->requestSlot (3); r.stop (512); r.p->mirrorActiveSlots();
        CHECK (r.cellParam (3, 2) == 1 && r.p->getStepPitch (3, 2) == -7, "D plays A's copy, pitch included");
        r.p->clearSlot (3); r.p->mirrorActiveSlots();
        CHECK (r.p->isSlotEmpty (3) && r.cellParam (3, 2) == 0, "clear D empties it and its pads");
        r.p->undo(); r.p->mirrorActiveSlots();
        CHECK (! r.p->isSlotEmpty (3) && r.cellParam (3, 2) == 1, "undo brings the cleared slot back");
    }
}

//==============================================================================
static void testStateAndExport()
{
    std::cout << "State / presets / MIDI export\n";
    {
        Rig a;
        a.step (0, 2); a.p->setStepCond (0, 2, CondNei); a.p->setStepRatchet (0, 2, 4); a.p->setStepPitch (0, 2, -3);
        a.param ("Direction1", DirPingPong); a.param ("Mute3", 1); a.param ("PlayMode4", PlayStrumUpDown); a.param ("Spread4", -40);
        MemoryBlock state; a.p->getStateInformation (state);
        Rig b; b.p->setStateInformation (state.getData(), (int) state.getSize());
        CHECK (b.cellParam (0, 2) == 1 && b.p->getStepCond (0, 2) == CondNei && b.p->getStepRatchet (0, 2) == 4 && b.p->getStepPitch (0, 2) == -3,
               "project round trip: cell and step data");
        CHECK (b.p->getDirection (1) == DirPingPong && b.p->isLaneMuted (3) && b.p->getPlayMode (4) == PlayStrumUpDown && b.p->getSpread (4) == -40,
               "project round trip: lane settings");
    }
    {
        Rig r;
        TugMidiSeqProgram prog ("test");
        auto v = r.p->presetToVar (prog);   // start from a complete preset, then change some fields
        auto base = r.p->varToPreset (v);
        base.stepCond[1][3] = CondFill; base.stepRatchet[2][4] = 2; base.stepPitch[0][7] = -12;
        base.direction[3] = DirRandom; base.mutate[4] = 25; base.playMode[2] = PlayStrum; base.spread[2] = -35;
        base.strumShape[2] = StrumCurve; base.strumTension[2] = 40;
        auto back = r.p->varToPreset (r.p->presetToVar (base));
        CHECK (back.stepCond[1][3] == CondFill && back.stepRatchet[2][4] == 2 && back.stepPitch[0][7] == -12
               && back.direction[3] == DirRandom && back.mutate[4] == 25 && back.playMode[2] == PlayStrum && back.spread[2] == -35
               && back.strumShape[2] == StrumCurve && back.strumTension[2] == 40,
               "preset JSON round trip keeps the new fields");
        auto old = r.p->presetToVar (base);
        if (auto* o = old.getDynamicObject()) { o->removeProperty ("Direction3"); o->removeProperty ("cond13"); }
        auto loaded = r.p->varToPreset (old);
        CHECK (loaded.direction[3] == DirForward && loaded.stepCond[1][3] == CondNone, "an older preset without them loads with defaults");
    }
    {
        Rig r;
        for (int s : { 0, 4, 8, 12 }) r.step (0, s);
        auto seq = r.p->renderPattern (1, juce::Array<int> { 60 });
        std::vector<int> ticks;
        for (auto* e : seq) if (e->message.isNoteOn()) ticks.push_back ((int) std::lround (e->message.getTimeStamp()));
        CHECK (ticks == std::vector<int> ({ 0, 960, 1920, 2880 }), "MIDI export: one bar at 960 PPQ " + str (ticks));
    }
    {
        Rig r;
        r.step (0, 0);
        r.p->requestSlot (1); r.stop (512); r.p->mirrorActiveSlots();
        r.step (0, 0); r.step (0, 8);
        auto seq = r.p->renderPattern (1, juce::Array<int> { 60 });
        int ons = 0; for (auto* e : seq) if (e->message.isNoteOn()) ons++;
        CHECK (ons == 2, "MIDI export plays the active slot (B: 2 notes), got " + String (ons));
    }
}

//==============================================================================
static void pump (int ms) { MessageManager::getInstance()->runDispatchLoopUntil (ms); }

static void testEditor()
{
    std::cout << "Editor\n";
    Rig r;
    r.step (0, 0);
    r.hold ({ 60 });
    r.stop(); r.play (Rig::bar / 2);
    std::unique_ptr<AudioProcessorEditor> ed (r.p->createEditor());
    pump (100);
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    CHECK (img.isValid() && img.getWidth() == ed->getWidth(), "the editor opens and paints while playing");
    int boxes = 0, focusable = 0;
    std::function<void (Component&)> walk = [&] (Component& c)
    {
        if (auto* b = dynamic_cast<ComboBox*> (&c)) { boxes++; if (b->getWantsKeyboardFocus()) focusable++; }
        for (auto* ch : c.getChildren()) walk (*ch);
    };
    walk (*ed);
    CHECK (boxes > 10 && focusable == 0, "no combo box takes the keyboard focus, so arrows reach the shortcuts");
   #if JucePlugin_IsMidiEffect
    CHECK (r.p->isMidiEffect() && r.p->getTotalNumOutputChannels() == 0 && r.p->producesMidi(),
           "MIDI FX build: a MIDI effect, no audio out, MIDI out");
   #else
    CHECK (! r.p->isMidiEffect() && r.p->getTotalNumOutputChannels() == 2, "instrument build: stereo out for the synth");
   #endif
}

//==============================================================================
// Screenshots

static void savePng (Component& c, Rectangle<int> area, const File& f, float scale = 2.0f)
{
    auto img = c.createComponentSnapshot (area, true, scale);
    f.deleteFile();
    FileOutputStream os (f);
    PNGImageFormat().writeImageToStream (img, os);
    std::cout << "wrote " << f.getFullPathName() << " (" << img.getWidth() << "x" << img.getHeight() << ")\n";
}

// A busy, playing scene: every lane doing something different.
static void buildScene (Rig& r)
{
    for (int s : { 0, 3, 6, 8, 12, 14 }) r.step (0, s);
    r.p->setStepRatchet (0, 8, 3); r.p->setStepPitch (0, 0, 7); r.p->setStepPitch (0, 12, -5);
    r.p->setStepCond (0, 6, Cond1of2); r.p->setStepCond (0, 14, CondPre);
    r.param ("Octave0", 1);
    for (int s : { 0, 4, 8, 11, 14 }) r.step (1, s);
    r.step (1, 11, 2);
    r.p->setStepCond (1, 14, CondFill);
    r.param ("PlayMode1", PlayStrum); r.param ("Spread1", 35);
    r.param ("StrumShape1", StrumCurve); r.param ("StrumTension1", 60); r.param ("StrumVel1", -40); r.param ("StrumHuman1", 25);
    r.param ("GridNum2", 32);
    for (int s = 0; s < 32; s += 3) r.step (2, s);
    r.p->setStepPitch (2, 9, 3); r.p->setStepPitch (2, 21, -2); r.p->setStepRatchet (2, 15, 2);
    r.param ("Direction2", DirPingPong); r.param ("Speed2", 16);
    for (int s : { 1, 5, 7, 9, 13 }) r.step (3, s);
    r.p->setStepCond (3, 7, CondNei); r.p->setStepCond (3, 13, Cond3of4);
    r.param ("Direction3", DirReverse);
    r.param ("GridNum4", 12);
    for (int s : { 0, 2, 5, 7, 9 }) r.step (4, s);
    r.step (4, 5, 2); r.step (4, 9, 2);
    r.p->setStepPitch (4, 7, 12);
    r.param ("Octave4", -1);
    r.param ("scaleKey", 9); r.param ("scaleType", 2);   // A minor
    r.param ("latch", 1);
    r.hold ({ 45, 57, 60, 64 });
    r.stop();
    r.p->toggleScreenNote (67);
    r.stop();
    r.p->copySlot (0, 1);
    r.p->copySlot (0, 2);
}

// One image per process: a second editor in the same run doesn't get its GUI
// timers going, so its note boxes would stay empty.
static int renderOne (const File& dir, int which)
{
    dir.createDirectory();
    Rig r;
    buildScene (r);
    if (which == 3)
    {
        StrumShapePanel panel (*r.p, 1);
        pump (200);
        savePng (panel, panel.getLocalBounds(), dir.getChildFile ("04-strum-shape.png"), 3.0f);
        return 0;
    }
    if (which == 1) r.play (Rig::bar * 5 / 4 + 9000);
    if (which == 2) { r.play (Rig::bar / 4 + 3000); r.p->requestSlot (1); r.play (512); }
    std::unique_ptr<AudioProcessorEditor> ed (r.p->createEditor());
    pump (800);
    if (which == 1)
    {
        savePng (*ed, ed->getLocalBounds(), dir.getChildFile ("01-cover.png"));
        savePng (*ed, { 3, 23 + 2 * 40, 600, 3 * 40 }, dir.getChildFile ("03-steps.png"), 3.0f);   // lanes 1-3, close up
    }
    if (which == 2)
    {
        while ((Time::getMillisecondCounter() / 250) % 2 != 0) Thread::sleep (5);   // the waiting rails' blink: on
        savePng (*ed, { 0, 0, 600, 263 }, dir.getChildFile ("02-pattern-slots.png"), 3.0f);
    }
    return 0;
}

static int renderMedia (const File& dir)
{
    const auto self = File::getSpecialLocation (File::currentExecutableFile).getFullPathName();
    for (int which : { 1, 2, 3 })
    {
        ChildProcess child;
        if (! child.start (StringArray { self, "--media-one", dir.getFullPathName(), String (which) })) return 1;
        std::cout << child.readAllProcessOutput();
        if (child.getExitCode() != 0) return 1;
    }
    return 0;
}

static int snapshot (const File& png)
{
    Rig r;
    buildScene (r);
    r.play (Rig::bar * 5 / 4 + 9000);
    std::unique_ptr<AudioProcessorEditor> ed (r.p->createEditor());
    pump (800);
    savePng (*ed, ed->getLocalBounds(), png);
    return 0;
}

//==============================================================================
int main (int argc, char** argv)
{
    ScopedJuceInitialiser_GUI init;
    const StringArray args (argv + 1, argc - 1);

    if (args[0] == "--snapshot" && args.size() > 1)    return snapshot (File::getCurrentWorkingDirectory().getChildFile (args[1]));
    if (args[0] == "--media" && args.size() > 1)       return renderMedia (File::getCurrentWorkingDirectory().getChildFile (args[1]));
    if (args[0] == "--media-one" && args.size() > 2)   return renderOne (File (args[1]), args[2].getIntValue());
    if (args.size() > 0)
    {
        std::cout << "usage: EngineTest                     run the tests\n"
                     "       EngineTest --snapshot <png>    render the editor while playing\n"
                     "       EngineTest --media <dir>       render the product-page images\n";
        return 2;
    }

    testBasics();
    testLatchAndKeyboard();
    testConditions();
    testStepTools();
    testScales();
    testLanes();
    testStrum();
    testStrumTiming();
    testEditsAndUndo();
    testPatternSlots();
    testStateAndExport();
    testEditor();

    std::cout << (failures == 0 ? "\nALL PASSED\n" : "\nFAILURES: " + std::to_string (failures) + "\n");
    return failures == 0 ? 0 : 1;
}
