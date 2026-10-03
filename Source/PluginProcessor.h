/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include <bitset>
using namespace juce;
#define numOfStep  32
#define numOfLine  5

extern ChangeBroadcaster updateMidiPort;;

#include "SynthVoice.h"

extern ChangeBroadcaster myGridChangeListener;

//const String myVirtualMidiName = "TMS Midi";

const juce::StringArray channelNames =  {"off","1","2","3","4","5","6","7","8","9","10","11","12","13","14", "15","15"};

const juce::StringArray valueTreeNames = 
{
    "block","Speed","Dur","GridNum","Octave","Vel","GlobalRestncBar","GlobalInOrFixedVel","inBuiltSynth","sortedOrFirstEmptySelect","Event","Shuffle","gridshuffle","griddelay","velGridButton","gridMidiRoute","channon","latch","Direction","fill","scaleKey","scaleType","Mutate","PlayMode","Spread","Mute","StrumShape","StrumTension","StrumVel","StrumHuman"
};
enum valueTreeNamesEnum
{
    BLOCK,SPEEED,DUR,GRIDNUM,OCTAVE,VEL,GLOBALRESTBAR,GLOABLINORFIXVEL,INBUILTSYNTH,SORTEDORFIRST,EVENT,SHUFFLE,GRIDSHUFFLE,GRIDDELAY,VELGRIDBUTTON,GRIDMIDIROUTE,CHANNON,LATCH,DIRECTION,FILL,SCALEKEY,SCALETYPE,MUTATE,PLAYMODE,SPREAD,MUTE,STRUMSHAPE,STRUMTENSION,STRUMVEL,STRUMHUMAN
};

// Lane play direction. Time still runs forward (shuffle, delay and note
// durations stay on the time slots); only which grid step a slot plays changes.
enum LaneDirection { DirForward = 0, DirReverse, DirPingPong, DirRandom };
const juce::StringArray directionNames = { "Forward", "Reverse", "Ping-Pong", "Random" };

// What a lane plays on its steps: its own voice of the chord (the original
// behaviour), or the whole held chord. `Spread<lane>` (-100..+100) is both the
// strum's direction and its width: + strums up (low to high), - strums down,
// the size is the milliseconds between notes, and 0 plays the chord at once.
// Strum Up/Down alternates on every hit, starting the way the sign says.
enum LanePlayMode { PlayVoice = 0, PlayStrum, PlayStrumUpDown };
const juce::StringArray playModeNames = { "Voice", "Strum", "Strum Up/Down" };
constexpr int maxStrumNotes = 16;

// Strum shape (per lane, edited in the STRM knob's popup). The strum keeps
// the length STRM gives it, |spread| ms per gap; the shape only moves the
// notes inside it. Curve: StrumTension + makes the gaps shrink (fast start),
// - makes them grow. StrumVel tilts velocity across the strum (+ later notes
// louder), StrumHuman adds random timing and velocity to each strum.
enum StrumShapeKind { StrumLinear = 0, StrumCurve };
const juce::StringArray strumShapeNames = { "Linear", "Curve" };

// Where note k of an n-note strum lands, 0..1 of the strum's length, and its
// velocity factor. Shared by the engine and the popup's preview.
inline void strumNotePlacement (int k, int n, int shape, float tension, float velTilt,
                                float& position, float& velocityFactor)
{
    const float t = n > 1 ? (float) k / (float) (n - 1) : 0.0f;
    position = t;
    if (shape == StrumCurve && tension != 0.0f)
    {
        const float exponent = 1.0f + 2.0f * std::abs (tension);   // at +-100 the last gap is still ~1/40 of the first
        position = tension > 0.0f ? 1.0f - std::pow (1.0f - t, exponent)   // gaps shrink
                                  : std::pow (t, exponent);                 // gaps grow
    }
    velocityFactor = juce::jlimit (0.05f, 2.0f, 1.0f + 0.6f * velTilt * t);
}

// Elektron-style trig conditions, one per step. A step only fires when its
// condition passes (and, for an Event cell, its probability roll too).
// "Loop" counts how many times the lane's own pattern has wrapped since play.
// These are kept out of the APVTS on purpose: as host parameters they would
// add another numOfLine * numOfStep (160) automatable entries.
enum TrigCond
{
    CondNone = 0,
    Cond1of2, Cond2of2, Cond1of3, Cond2of3, Cond3of3, Cond1of4, Cond2of4, Cond3of4, Cond4of4,
    CondFirst, CondNotFirst,   // first loop since play / every loop but the first
    CondPre,   CondNotPre,     // this lane's previous condition result
    CondNei,   CondNotNei,     // the lane below's most recent condition result
    CondFill,  CondNotFill,    // while the Fill button / "fill" parameter is held
    // new conditions go here, at the end: presets and projects store these numbers
    NumTrigConds
};
const juce::StringArray trigCondNames =
{
    "", "1:2","2:2","1:3","2:3","3:3","1:4","2:4","3:4","4:4", "1ST","!1ST", "PRE","!PRE", "NEI","!NEI", "FILL","!FILL"
};
// preset-JSON / state-tree key prefix for step conditions (cond<lane><step>)
const juce::String stepCondKey = "cond";

// Ratchet: a step fires 1..maxRatchet times, spread evenly over its slot.
// Stored like the conditions (not a host parameter); preset key ratchet<lane><step>.
constexpr int maxRatchet = 4;
const juce::String stepRatchetKey = "ratchet";

// Step pitch: an offset added to the lane's note — in semitones, or in scale
// degrees while a scale is set. Stored like the ratchets; preset key pitch<lane><step>.
constexpr int maxStepPitch = 24;
const juce::String stepPitchKey = "pitch";

// Scale lock (global): every lane's output note is snapped to Key + Scale.
// Index 0 is "Off". Each entry lists the scale's pitch classes from the key.
const juce::StringArray scaleKeyNames  = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
const juce::StringArray scaleTypeNames = { "Off", "Major", "Minor", "Dorian", "Phrygian", "Lydian", "Mixolydian",
                                           "Locrian", "Harm. Minor", "Mel. Minor", "Major Pent.", "Minor Pent.", "Blues" };
const std::vector<std::vector<int>> scaleIntervals =
{
    {},                         // Off
    { 0, 2, 4, 5, 7, 9, 11 },   // Major
    { 0, 2, 3, 5, 7, 8, 10 },   // Minor
    { 0, 2, 3, 5, 7, 9, 10 },   // Dorian
    { 0, 1, 3, 5, 7, 8, 10 },   // Phrygian
    { 0, 2, 4, 6, 7, 9, 11 },   // Lydian
    { 0, 2, 4, 5, 7, 9, 10 },   // Mixolydian
    { 0, 1, 3, 5, 6, 8, 10 },   // Locrian
    { 0, 2, 3, 5, 7, 8, 11 },   // Harmonic minor
    { 0, 2, 3, 5, 7, 9, 11 },   // Melodic minor
    { 0, 2, 4, 7, 9 },          // Major pentatonic
    { 0, 3, 5, 7, 10 },         // Minor pentatonic
    { 0, 3, 5, 6, 7, 10 }       // Blues
};

extern juce::CriticalSection midiOutputMutex;

const std::vector <juce::String> myNotetUnit =
{ "1nd","1n", "1nt",
    "2nd","2n","2nt",
    "4nd","4n","4nt",
    "8nd","8n","8nt",
    "16nd","16n","16nt",
    "32nd","32n","32nt",
    "64nd","64n","64nt",
    "128nd","128n","128nt" };

const juce::StringArray myNotetUnitSA  = { "1nd","1n", "1nt",
    "2nd","2n","2nt",
    "4nd","4n","4nt",
    "8nd","8n","8nt",
    "16nd","16n","16nt",
    "32nd","32n","32nt",
    "64nd","64n","64nt",
    "128nd","128n","128nt" };
class TugMidiSeqProgram
{
    //  MDAEPianoProgram();
public:
    TugMidiSeqProgram(juce::String name)
    {
        myProgramname = name;
        for (auto& lane : stepRatchet)
            for (auto& r : lane)
                r = 1;
    }
    
    juce::String myProgramname;
    // where this preset lives on disk:
    //  - empty File()  -> legacy bundle (TugMidiSeqPresets.json)
    //  - otherwise     -> its own single-preset JSON file (TugMorpho style)
    juce::File sourceFile;
    int grids[numOfLine][numOfStep];
    int gridVelArr[numOfLine][numOfStep];
    int stepCond[numOfLine][numOfStep] = {};
    int stepRatchet[numOfLine][numOfStep];   // 1..maxRatchet, set to 1 in the constructor
    int stepPitch[numOfLine][numOfStep] = {};
    int scaleKey = 0, scaleType = 0;
    int direction[numOfLine] = {};
    int mutate[numOfLine] = {};
    int playMode[numOfLine] = {};
    int spread[numOfLine] = { 20, 20, 20, 20, 20 };
    int strumShape[numOfLine] = {}, strumTension[numOfLine] = {}, strumVel[numOfLine] = {}, strumHuman[numOfLine] = {};
    int numOfGrid[numOfLine];
    int octave[numOfLine];
    int gridsSpeed[numOfLine];
    int gridsDuration[numOfLine];
    int gridsVel[numOfLine];;
    int gridsEvent[numOfLine];
    int gridsShuffle[numOfLine];
    int gridsDelay[numOfLine];
    int gridsMidiRoute[numOfLine];
    int globalResyncBar = 1;
    bool GlobalInOrFixedVel = false;
    bool inBuiltSynth = false;
    bool sortedOrFirst = false;
    int shuffle = 0;
    bool channelOn = false;
    
};


class MidiProcessor
{
public:
    MidiProcessor()
    {
#if JUCE_MAC
    //    handleVirtualOwnMidiPort();
       // midiInput = juce::MidiInput::createNewDevice("TMS midi ", this);
#endif
        
    }

    void setMidiPort(String s)
    {
        const juce::ScopedLock lock(midiOutputMutex);
        /*
        if(myVirtualMidiName == s &&  midiOutput != nullptr)
        {
            if(midiOutput->getName() == s)
               handleVirtualOwnMidiPort();
            return;
        }
         */
        
        auto  devices = juce::MidiOutput::getAvailableDevices();
         
        for (auto device : devices)
        {

            if (device.name == s)
            {
                if (midiOutput != nullptr)
                    midiOutput->clearAllPendingMessages();
                midiOutput = juce::MidiOutput::openDevice(device.identifier);
                if (midiOutput != nullptr)
                {
                    if (midiOutput->isBackgroundThreadRunning() == false)
                        midiOutput->startBackgroundThread();
                }
                return;
            }
        }
        
    }

    ~MidiProcessor()
    {
        if (midiOutput != nullptr)
        { 
            midiOutput->stopBackgroundThread(); // Stop the MIDI output thread if needed
           
        }
       
    }
   void  handleVirtualOwnMidiPort()
    {
       /*
       midiOutput = juce::MidiOutput::openDevice(myVirtualMidiName);
       if (midiOutput == nullptr)
           midiOutput = juce::MidiOutput::createNewDevice(myVirtualMidiName);
       if (midiOutput != nullptr)
       {
           if (midiOutput->isBackgroundThreadRunning() == false)
               midiOutput->startBackgroundThread();
       }
        */
        
    }
    
    void sendMidiBuffer(const MidiBuffer &buffer, int samplerate)
    {
        const juce::ScopedTryLock lock(midiOutputMutex);
        if (lock.isLocked() && midiOutput != nullptr)
        {
            for (const auto metadata : buffer)
                midiOutput->sendMessageNow(metadata.getMessage());
        }
    }
    
    void sendMidiMessage( juce::MidiMessage& message,int ch)
    {
        
        message.setChannel(ch + 1);
        if (midiOutput != nullptr)
            midiOutput->sendMessageNow(message);

       // MidiMessage message2 = MidiMessage::noteOn( 1, 64, 1.0f );
       // midiOutput->sendMessageNow( message2 );
    }
/*
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        // Process the incoming MIDI message here (if needed)
 
        
    }
*/

private:


    // Define the pointer to MidiInputCallback
    MidiInputCallback* midiInputCallback;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    std::unique_ptr<juce::MidiInput> midiInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiProcessor)
};



//==============================================================================
/**
 */
class TugMidiSeqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TugMidiSeqAudioProcessor();
    ~TugMidiSeqAudioProcessor() override;
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    bool subComputrFunc(int i,juce::MidiBuffer& midiMessages ,int sample);
    
    void  writePresetToFileJSON();
    void  writeSinglePresetToFileJSON(TugMidiSeqProgram& prg);

    void  readPresetToFileJSON();
    void  readSinglePresetFilesJSON();
    void createPrograms(juce::String preset_name );

    // active folder that single-preset JSON files are read from / written to;
    // persisted across sessions via appProperties
    juce::File presetFolder;
    juce::ApplicationProperties appProperties;
    void setPresetFolder(const juce::File& dir);
    
    juce::AudioProcessorValueTreeState valueTreeState;
    // the grid step lane i is on (after its play direction), -1 when stopped
    int getSteps(int i)
    {
        if (myIsPlaying == false) return -1;
        return playStep[i];
    }
    int getDirection (int line) const { return (int) *gridsDirectionAtomic[line]; }

    // Mutate (per lane, 0..100 %): each time the lane completes a loop, every
    // step flips with that chance. The flips accumulate in a mask that sits on
    // top of the written pattern without changing it (an Off step plays, an
    // On / Event step rests); 0 % freezes the current mask, and every play,
    // or "Reset mutations", starts again from the written pattern.
    int  getMutate (int line) const          { return (int) *gridsMutateAtomic[line]; }
    int  getPlayMode (int line) const        { return (int) *gridsPlayModeAtomic[line]; }

    // Mute (per lane, automatable, saved with the project but not in presets):
    // the lane keeps running (steps, conditions, mutations) but sends no notes,
    // so lanes whose NEI / PRE conditions read it behave the same. Mute wins
    // over solo.
    bool isLaneMuted (int line) const        { return *gridsMuteAtomic[line] > 0.5f; }
    void setLaneMute (int line, bool muted); // one undo step
    int  getSpread (int line) const          { return (int) *gridsSpreadAtomic[line]; }
    int   getStrumShape (int line) const     { return (int) *gridsStrumShapeAtomic[line]; }
    float getStrumTension (int line) const   { return *gridsStrumTensionAtomic[line] / 100.0f; }   // -1..1
    float getStrumVelTilt (int line) const   { return *gridsStrumVelAtomic[line] / 100.0f; }       // -1..1
    float getStrumHumanize (int line) const  { return *gridsStrumHumanAtomic[line] / 100.0f; }     // 0..1
    void setLanePlayMode (int line, int mode);   // one undo step
    void setLaneMutate (int line, int percent);   // one undo step
    void requestMutationReset (int line)     { mutateResetRequest[line].store (true); }
    bool isStepMutated (int line, int step) const
    {
        return ((pubMutateMask[line].load (std::memory_order_relaxed) >> step) & 1u) != 0;
    }

    // Playhead for the GUI, published once per block (see publishNoteMap): the
    // grid step being played (-1 when stopped), how far through its time slot,
    // and whether the lane is travelling right-to-left (Reverse, or the
    // backward pass of Ping-Pong).
    int   getPlayheadStep (int line) const      { return pubPlayStep[line].load (std::memory_order_relaxed); }
    float getPlayheadFraction (int line) const  { return pubPlayFrac[line].load (std::memory_order_relaxed); }
    bool  isPlayheadBackward (int line) const   { return pubBackward[line].load (std::memory_order_relaxed); }
    void setSpeedofLine(int index, int line)
    {
        gridsSpeed[line] = index;
    }
    void setDurationofLine(int index, int line)
    {
        gridsDuration[line] = index;
    }
    
    // GUI-side reads of the held-note state go through the snapshot that
    // publishNoteMap() writes once per block, never the audio thread's vectors.
    int getMidi(int line)
    {
        return laneInNote[line].load (std::memory_order_relaxed);
    }
    // note the lane is sounding right now (input note + octave), or -1
    int getLaneOutNote (int line) const
    {
        return laneOutNote[line].load (std::memory_order_relaxed);
    }
    bool isNoteHeld (int note) const
    {
        return ((heldMask[note >> 6].load (std::memory_order_relaxed) >> (note & 63)) & 1) != 0;
    }
    int getHeldNoteCount() const
    {
        int n = 0;
        for (auto& m : heldMask) n += (int) std::bitset<64> (m.load (std::memory_order_relaxed)).count();   // portable popcount
        return n;
    }
    bool isNotePhysicallyHeld (int note) const
    {
        return ((physMask[note >> 6].load (std::memory_order_relaxed) >> (note & 63)) & 1) != 0;
    }

    // Notes played by clicking the on-screen keyboard (NoteMap). Each click
    // toggles a note that then stays down like a held key. Message thread only;
    // the events reach processBlock through a lock-free FIFO.
    void toggleScreenNote (int note)
    {
        const bool on = ! screenHeld[(size_t) note];
        screenHeld[(size_t) note] = on;
        pushScreenNote (note, on);
    }
    void releaseScreenNotes()
    {
        for (int n = 0; n < 128; n++)
            if (screenHeld[(size_t) n]) { screenHeld[(size_t) n] = false; pushScreenNote (n, false); }
    }
    bool isScreenNote (int note) const { return screenHeld[(size_t) note]; }

    int getStepCond (int line, int step) const
    {
        return stepCond[line][step].load (std::memory_order_relaxed);
    }
    // Safe from any thread. Notifies the host only when asked to, because that
    // must happen on the message thread (GUI edits) — preset loads don't need it.
    void setStepCond (int line, int step, int cond, bool notifyHost = false)
    {
        stepCond[line][step].store (jlimit (0, NumTrigConds - 1, cond), std::memory_order_relaxed);
        if (notifyHost)
            updateHostDisplay (ChangeDetails{}.withNonParameterStateChanged (true));
    }
    void clearStepData()
    {
        for (auto& lane : stepCond)
            for (auto& c : lane)
                c.store (CondNone, std::memory_order_relaxed);
        for (auto& lane : stepRatchet)
            for (auto& r : lane)
                r.store (1, std::memory_order_relaxed);
        for (auto& lane : stepPitch)
            for (auto& p : lane)
                p.store (0, std::memory_order_relaxed);
    }

    int getStepRatchet (int line, int step) const
    {
        return stepRatchet[line][step].load (std::memory_order_relaxed);
    }
    void setStepRatchet (int line, int step, int hits)
    {
        stepRatchet[line][step].store (jlimit (1, maxRatchet, hits), std::memory_order_relaxed);
    }
    void setStepRatchetUndoable (int line, int step, int hits);
    bool isFillOn() const { return *fillAtomic > 0.5f; }

    int getStepPitch (int line, int step) const
    {
        return stepPitch[line][step].load (std::memory_order_relaxed);
    }
    void setStepPitch (int line, int step, int offset)
    {
        stepPitch[line][step].store (jlimit (-maxStepPitch, maxStepPitch, offset), std::memory_order_relaxed);
    }
    void setStepPitchUndoable (int line, int step, int offset);

    // scale lock
    int  getScaleKey() const  { return (int) *scaleKeyAtomic; }
    int  getScaleType() const { return (int) *scaleTypeAtomic; }
    bool isScaleOn() const    { return getScaleType() > 0; }
    bool isInScale (int note) const;
    // the lane's note after the step's pitch offset and the scale lock
    int  pitchedNote (int note, int stepOffset) const;
    // GUI edit of a step condition, recorded in the undo history
    void setStepCondUndoable (int line, int step, int cond);
    void notifyStateChanged();

    //==========================================================================
    // Undo / redo (message thread only). Parameter changes are recorded by the
    // APVTS itself (it was created with undoManager); these group them into one
    // step per user action and add the non-parameter edits (step conditions).
    // Flushing first matters as much as flushing after: a change that's still
    // waiting for the APVTS timer would otherwise be recorded inside the new
    // step, with the wrong "before" value, and undoing it would wipe it.
    void beginUndoStep() { flushToUndo(); undoManager.beginNewTransaction(); }
    // closes a step opened with beginUndoStep() whose edits went through
    // parameters (flushes them into the tree first, like undoableEdit)
    void endUndoStep() { flushToUndo(); undoManager.beginNewTransaction(); }
    // Runs `edit` as a single undo step. The APVTS copies parameter values into
    // its tree (where undo records them) on a timer, so flush before returning:
    // otherwise the changes could land in whatever step the next click starts.
    template <typename Fn> void undoableEdit (Fn&& edit)
    {
        beginUndoStep();
        edit();
        endUndoStep();
    }
    void flushToUndo() { (void) valueTreeState.copyState(); }   // copyState() flushes parameters to the tree
    bool canUndo() const { return undoManager.canUndo(); }
    bool canRedo() const { return undoManager.canRedo(); }
    void undo() { (void) valueTreeState.copyState(); undoManager.undo(); notifyStateChanged(); myGridChangeListener.sendChangeMessage(); }
    void redo() { (void) valueTreeState.copyState(); undoManager.redo(); notifyStateChanged(); myGridChangeListener.sendChangeMessage(); }

    //==========================================================================
    // Lane edits (message thread, each one undo step)
    void copyLane (int line);
    void pasteLane (int line);
    bool hasLaneClipboard() const;
    void shiftLane (int line, int delta);                 // rotate within the lane length
    void shiftAllLanes (int delta);                       // every lane, as one undo step
    void euclidLane (int line, int hits, int rotation);   // without its own undo step: see EuclidPanel
    void clearLane (int line);
    void setLaneDirection (int line, int dir);
    int getLoopMeasure()
    {
        if (myIsPlaying == false) return + 1;
        return  measureBar + 1;
    }
    // Thread-safe parameter write. getParameterAsValue().setValue() goes through
    // juce::Value/ValueTree, which is message-thread only: a temporary Value also
    // registers and unregisters a ValueTree listener, so calling it off the message
    // thread corrupts the listener list that APVTS's own timer walks in flushToTree().
    // Hosts call setCurrentProgram() from arbitrary threads, so route writes here.
    void setParamValue (const juce::String& paramID, float rawValue)
    {
        if (auto* p = valueTreeState.getParameter (paramID))
            p->setValueNotifyingHost (p->convertTo0to1 (rawValue));
    }
    void setAllValue( juce::String s,int v)
    {
        for(int i = 0 ; i < 5 ; i++)
        {
            setParamValue (s + std::to_string(i), (float) v);
        }
        myGridChangeListener.sendChangeMessage();
    }
    void randomizeGrids(int index)
    {

        for(int i = 0 ; i < 32 ; i++)
        {
            juce::String grid_block;
            grid_block <<  "block" << index << i;
            int x = juce::Random::getSystemRandom().nextInt(2);
            setParamValue (grid_block, (float) x);
        }
    }
    
    void resetAllParam();

    void deletePreset(int);

    // single preset <-> JSON object (shared by bundle and single-file IO)
    juce::var presetToVar(const TugMidiSeqProgram& prg);
    TugMidiSeqProgram varToPreset(const juce::var& preset);
    
    std::atomic<float> * gridsArr[numOfLine][numOfStep];
    std::atomic<float> * gridVelArrAtomic[numOfLine][numOfStep];
    
    int steps[5] = {};      // time slot each lane is in
    int playStep[5] = {};   // grid step that slot plays (see LaneDirection)
    int new_steps[5] = {};
    
    std::atomic<float> *numOfGrid[5];
    std::atomic<float> *octave[5];
    std::atomic<float> *globalResyncBar;
    float myBpm;
    double myBps;
    int measureSample = 0;
    int measureBar = 0;
    double  mySampleRate = 0;
    int  sampleNumber[5] ={};
    int  baseSampleNumber[5] ={};
    int stepLoopResetInterval[5];
    bool midiState[5] = {};
    bool myIsPlaying  = false;
    
    int delaySampleNumberForQuarter;
    
    float getVelButton(int line, int step, bool total = false)
    {
        if(*GlobalInOrFixedAtomic != 0)
            return 1.0f;
        if(total == false)
            return *gridVelArrAtomic[line][step]/ 127.0f;
        float velTmp = *gridsVelAtomic[line]/ 90.0f;
        velTmp = jlimit(0.0f,1.0f,velTmp**gridVelArrAtomic[line][step]/ 127.0f);
        return velTmp;
    }
    float getGridButtonState(int line, int step)
    {
        
        return *gridsArr[line][step];
        
    }

    // Whether any step the lane plays is an Event cell, i.e. the Event knob does something
    bool laneHasEventStep (int line) const
    {
        const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
        for (int st = 0; st < n; st++)
            if ((int) *gridsArr[line][st] == 2) return true;
        return false;
    }
    
    float getDelayRatio(int index)
    {
        if (stepLoopResetInterval[index] == 0) return 0;
        return  (*gridsDelayAtomic[index]*100)/stepLoopResetInterval[index];
        
    }
    
    float getDurAngle(int index)
    {
        if (stepResetInterval[index] == 0) return 0;
        return ( stepmidStopSampleInterval[index] *2.0*juce::double_Pi/stepResetInterval[index])/(*numOfGrid[index]);
        
    }
    float getGridSampleLen(int line)
    {
        
        if (stepLoopResetInterval[line] == 0) return 0;
        return stepmidStopSampleInterval[line]*1.0f/stepLoopResetInterval[line];
        
    }
    float getGridContinousRatio(int line)
    {
        if (myIsPlaying == false) return -1;
        if (stepLoopResetInterval[line] == 0) return 0;
      
       // auto x = -*gridsDelayAtomic[line]*100 + sampleNumber[line] ;
       // x = circularRange(x, 0, stepLoopResetInterval[line]);
        
        return sampleNumber[line]*1.0/(stepLoopResetInterval[line]);
        
    }

    float  circularRange(int value, int lowerBound, int upperBound)
    {
        int rangeSize = upperBound - lowerBound + 1;
        int normalizedValue = (value - lowerBound) % rangeSize;
        return (normalizedValue + rangeSize) % rangeSize + lowerBound;
    }
    float getEventRandom(int line)
    {
        // if (myIsPlaying == false) return -1;
        return *gridsEventAtomic[line]/100;;
        
    }
    
    // Shuffle ratio of the time slot a grid step is played in, for drawing.
    // Reverse (and Ping-Pong's way back) plays step k in slot n-1-k, so the
    // pads, note strips and Satellite arcs mirror the slot lengths and the
    // playhead sweeps at an even speed. Random has no fixed slot: own ratio.
    float getStepDisplayRatio (int line, int step)
    {
        const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
        const int slot = isPlayheadBackward (line) ? n - 1 - step : step;
        const float r = getSfuffleRatios (line, jlimit (0, numOfStep - 1, slot));
        return std::isfinite (r) ? jlimit (0.0f, 2.0f, r) : 0.0f;
    }

    float getSfuffleRatios(int line, int step)
    {
        // if (myIsPlaying == false) return -1;
        if (stepmidStopSampleInterval[line] == 0) return 0;
        float x = stepResetIntervalForShuffle[line][step];
        float y = stepResetInterval[line];
        float test = x / y;

      //  DBG(std::to_string(test));
        return test;
     //   return (float)(stepmidStopSampleIntervalForShuffle[line][step]) /stepmidStopSampleInterval[line];
    }
    
    void setShuffle()
    {

    }
    
    void initPrepareValue();
    
    void calculateAndUpdateSetup(int myLine);
    
    bool  getChannelStatus()
    {
        return *channelOnAtamic;
    }

    String getMidiPortNameFromXml()
    {
        
        auto x = valueTreeState.state.getOrCreateChildWithName("midiPort", nullptr);
        auto name =  x.getProperty("nameOfMidiPort", "").toString();
       return name;
    }

    void setMidiPortNameToXml(String midiPort)
    {
        auto x = valueTreeState.state.getOrCreateChildWithName("midiPort", nullptr);
        x.setProperty("nameOfMidiPort", midiPort, nullptr);

    }
    void setMidiPortName(String midiPort)
    {

        setMidiPortNameToXml( midiPort);
        midiPortName = midiPort;
        midiProcessor->setMidiPort(midiPort);
        updateMidiPort.sendChangeMessage();
    }

    String getMidiPortName()
    {
        return  midiPortName;
    }
    void setGridSolo(int lane_no)
    {
         
         if(soloLane == lane_no)
         {
             soloLane = -1;
             return;
         }
        soloLane = lane_no;
    }
    
    int getSoloState()
    {
        return soloLane;
    }

    // Persisted editor width — lives in the state tree, so it is saved with the
    // DAW project and restored on GUI reopen, in both AU and VST3 (both formats
    // round-trip through get/setStateInformation).
    void setEditorWidth (int w)
    {
        auto x = valueTreeState.state.getOrCreateChildWithName ("editor", nullptr);
        x.setProperty ("width", w, nullptr);
    }
    int getEditorWidth (int defaultW)
    {
        auto x = valueTreeState.state.getChildWithName ("editor");
        if (x.isValid())
            return (int) x.getProperty ("width", defaultW);
        return defaultW;
    }

    //==========================================================================
    // MIDI export (message thread). Plays the current pattern offline, in a
    // private copy of this processor, for `bars` 4/4 bars with the notes
    // currently held (or a default chord when none are) and writes a type-1
    // MIDI file at 960 PPQ. Each lane keeps its own MIDI channel.
    juce::File renderPatternToMidiFile (int bars);
    juce::MidiMessageSequence renderPattern (int bars, const juce::Array<int>& notes);
    juce::Array<int> notesForExport() const;
    int  getExportBars() const   { return (int) valueTreeState.state.getChildWithName ("editor").getProperty ("exportBars", 4); }
    void setExportBars (int bars) { valueTreeState.state.getOrCreateChildWithName ("editor", nullptr).setProperty ("exportBars", bars, nullptr); }

    // set on the private copy used by renderPattern(): no external MIDI port,
    // no in-built synth
    bool offlineRender = false;

private:
    int soloLane = -1;
    
    String midiPortName = "Press To Select Midi Port";
    void midiHandling(juce::MidiBuffer& midiMessages,int sampleOffset, bool sampelBased = true);
    
    std::unique_ptr<MidiProcessor> midiProcessor;
    
    int stpSample[5] = {};
    juce::AudioPlayHead::CurrentPositionInfo positionInfo;
    int stepResetInterval[5] = {};
    int stepmidStopSampleInterval[5] = {-1,-1,-1,-1,-1};
    
    // Zero-initialised: only the first numOfGrid entries of a lane are ever
    // written, but the GUI reads up to the *current* #Grid, which can be ahead
    // of the audio thread right after an automation change.
    int stepmidStopSampleIntervalForShuffle[5][numOfStep] = {};
    int stepResetIntervalForShuffle[5][numOfStep] = {};
   // int forGuiStepResetIntervalForShuffle[5][numOfStep];

    int stepmidStopSampleCounter[5] = {-1,-1,-1,-1,-1};
    int  ppq = 0;
    
    int  midiEffectSampelDiffBitweenCall;
    double  prevtimeInSamples = 0;
    
    std::atomic<float> * gridsSpeedAtomic[numOfLine];
    std::atomic<float> * gridsDurationAtomic[numOfLine];
    
    std::atomic<float> *gridsVelAtomic[numOfLine];
    std::atomic<float> *gridsEventAtomic[numOfLine];
    std::atomic<float> *gridsShuffleAtomic[numOfLine];
    std::atomic<float> *gridsDelayAtomic[numOfLine];
    std::atomic<float> *gridsMidiRouteAtomic[numOfLine];
    std::atomic<float> *gridsDirectionAtomic[numOfLine];
    std::atomic<float> *gridsMutateAtomic[numOfLine];
    std::atomic<float> *gridsPlayModeAtomic[numOfLine];
    std::atomic<float> *gridsSpreadAtomic[numOfLine];
    std::atomic<float> *gridsMuteAtomic[numOfLine];
    std::atomic<float> *gridsStrumShapeAtomic[numOfLine], *gridsStrumTensionAtomic[numOfLine];
    std::atomic<float> *gridsStrumVelAtomic[numOfLine], *gridsStrumHumanAtomic[numOfLine];

    // Strum: notes of the current strum still waiting for their turn (audio
    // thread, fixed size, no allocation).
    struct StrumNote { int countdown; int duration; juce::MidiMessage note; };
    StrumNote strumQueue[numOfLine][maxStrumNotes];
    int  strumCount[numOfLine] = {};
    bool strumFlipped[numOfLine] = {};   // Strum Up/Down: next hit goes against the sign
    void playChord (int line, int duration, int window, juce::MidiBuffer& midiMessages, int sample);
    int  samplesToNextHit (int line) const;
    void tickStrum (int line, juce::MidiBuffer& midiMessages, int sample);
    uint32_t mutateMask[numOfLine] = {};                // audio thread
    std::atomic<uint32_t> pubMutateMask[numOfLine];     // for the pads
    std::atomic<bool> mutateResetRequest[numOfLine];
    void mutateLane (int line);
    void rotateLaneSteps (int line, int delta);
    int directedStep (int line, int slot) const;
    bool cellActiveAtSlot (int line, int slot) const;
    std::atomic<float> *GlobalInOrFixedAtomic;;
    std::atomic<float> *inBuiltSynthAtomic;
    std::atomic<float> *sortedOrFirstEmptySelectAtomic;
    std::atomic<float> *shuffleAtomic;
    std::atomic<float> *channelOnAtamic;
    std::atomic<float> *latchAtomic;

    // Latch: which keys are physically down right now (the held-note lists keep
    // latched notes after release, so they can't answer that themselves).
    bool physHeld[128] = {};
    int  physHeldCount = 0;
    bool prevLatch = false;
    void clearHeldNotes();
    void releaseUnheldNotes();

    std::atomic<float> *fillAtomic;

    // Trig-condition engine state (audio thread only).
    std::atomic<int> stepCond[numOfLine][numOfStep];
    std::atomic<int> stepRatchet[numOfLine][numOfStep];
    std::atomic<int> stepPitch[numOfLine][numOfStep];
    std::atomic<float> *scaleKeyAtomic, *scaleTypeAtomic;

    // Ratchet repeats still to play in the current step, per lane (audio thread).
    int ratchetLeft[numOfLine] = {}, ratchetCountdown[numOfLine] = {}, ratchetInterval[numOfLine] = {};
    int ratchetDuration[numOfLine] = {};
    juce::MidiMessage ratchetNote[numOfLine];
    void emitLaneNote (int line, juce::MidiMessage note, int durationSamples, juce::MidiBuffer& midiMessages, int sample);
    int  loopCount[numOfLine] = {};
    int  lastStep[numOfLine] = {};
    bool lastCondResult[numOfLine] = {};
    void resetTrigCondState();
    bool evaluateTrigCond (int line, int step) const;
    void writeStepDataTo (juce::ValueTree& state) const;   // conditions, ratchets, pitches
    void readStepDataFrom (const juce::ValueTree& state);

    // Snapshot for the GUI's note map, published once per block.
    int lastOutNote[numOfLine] = {};
    std::atomic<int> laneInNote[numOfLine];
    std::atomic<int> laneOutNote[numOfLine];
    std::atomic<uint64_t> heldMask[2];
    std::atomic<uint64_t> physMask[2];
    std::atomic<int>   pubPlayStep[numOfLine];
    std::atomic<float> pubPlayFrac[numOfLine];
    std::atomic<bool>  pubBackward[numOfLine];
    void publishNoteMap();

    // On-screen keyboard -> audio thread (single producer, single consumer).
    struct ScreenNoteEvent { int note; bool on; };
    static constexpr int screenFifoSize = 256;
    juce::AbstractFifo screenFifo { screenFifoSize };
    ScreenNoteEvent screenEvents[screenFifoSize] = {};
    std::bitset<128> screenHeld;   // message thread only
    void pushScreenNote (int note, bool on);
    void drainScreenNotes (juce::MidiBuffer& midiMessages);

    //std::atomic<float> *numOfGrid[5];
    juce::UndoManager undoManager;
    juce::MidiBuffer myInnmidiBuffer;
    juce::MidiBuffer midiMessagesStack;
    std::vector<juce::MidiMessage> inMidiNoteList;
    std::vector<juce::MidiMessage> inMidiNoteListVector;
    //std::list<juce::MidiMessage> inMidiNoteListTmp;
    //std::vector<juce::MidiMessage> inMidiNoteListVectorTmp;
    void initForVariables();
    struct RealMidiNoteList
    {
        juce::MidiMessage sentMidi;
        int durationsample;
        int lineNo;
    };
    std::vector<RealMidiNoteList> inRealMidiNoteList;
    int preset_idex = 0;
    
    double gridsSpeed[numOfLine];
    double gridsDuration[numOfLine];
    std::vector <TugMidiSeqProgram >myProgram;
    int program;
    juce::File *resourceJsonFile;
    Synthesiser   mySynth;
    SynthVoice*  myVoice;
    SynthSound    *synthSound;
    float remaining[numOfLine] = {};
    float gauge[numOfLine] = {};
    double prevppq = 0;
    //juce::AudioProcessorValueTreeState::ParameterLayout createAllParameters();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessor)
};
