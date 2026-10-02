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
    "block","Speed","Dur","GridNum","Octave","Vel","GlobalRestncBar","GlobalInOrFixedVel","inBuiltSynth","sortedOrFirstEmptySelect","Event","Shuffle","gridshuffle","griddelay","velGridButton","gridMidiRoute","channon","latch"
};
enum valueTreeNamesEnum
{
    BLOCK,SPEEED,DUR,GRIDNUM,OCTAVE,VEL,GLOBALRESTBAR,GLOABLINORFIXVEL,INBUILTSYNTH,SORTEDORFIRST,EVENT,SHUFFLE,GRIDSHUFFLE,GRIDDELAY,VELGRIDBUTTON,GRIDMIDIROUTE,CHANNON,LATCH
};

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
    NumTrigConds
};
const juce::StringArray trigCondNames =
{
    "", "1:2","2:2","1:3","2:3","3:3","1:4","2:4","3:4","4:4", "1ST","!1ST", "PRE","!PRE", "NEI","!NEI"
};
// preset-JSON / state-tree key prefix for step conditions (cond<lane><step>)
const juce::String stepCondKey = "cond";

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
    }
    
    juce::String myProgramname;
    // where this preset lives on disk:
    //  - empty File()  -> legacy bundle (TugMidiSeqPresets.json)
    //  - otherwise     -> its own single-preset JSON file (TugMorpho style)
    juce::File sourceFile;
    int grids[numOfLine][numOfStep];
    int gridVelArr[numOfLine][numOfStep];
    int stepCond[numOfLine][numOfStep] = {};
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
    int getSteps(int i)
    {
        if (myIsPlaying == false) return -1;
        return steps[i];
    }
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
    void clearStepConds()
    {
        for (auto& lane : stepCond)
            for (auto& c : lane)
                c.store (CondNone, std::memory_order_relaxed);
    }
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
    
    int steps[5] = {};
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

    // Trig-condition engine state (audio thread only).
    std::atomic<int> stepCond[numOfLine][numOfStep];
    int  loopCount[numOfLine] = {};
    int  lastStep[numOfLine] = {};
    bool lastCondResult[numOfLine] = {};
    void resetTrigCondState();
    bool evaluateTrigCond (int line, int step) const;
    void writeStepCondsTo (juce::ValueTree& state) const;
    void readStepCondsFrom (const juce::ValueTree& state);

    // Snapshot for the GUI's note map, published once per block.
    int lastOutNote[numOfLine] = {};
    std::atomic<int> laneInNote[numOfLine];
    std::atomic<int> laneOutNote[numOfLine];
    std::atomic<uint64_t> heldMask[2];
    std::atomic<uint64_t> physMask[2];
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
