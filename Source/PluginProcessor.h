/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
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
    "block","Speed","Dur","GridNum","Octave","Vel","GlobalRestncBar","GlobalInOrFixedVel","inBuiltSynth","sortedOrFirstEmptySelect","Event","Shuffle","gridshuffle","griddelay","velGridButton","gridMidiRoute","channon"
};
enum valueTreeNamesEnum
{
    BLOCK,SPEEED,DUR,GRIDNUM,OCTAVE,VEL,GLOBALRESTBAR,GLOABLINORFIXVEL,INBUILTSYNTH,SORTEDORFIRST,EVENT,SHUFFLE,GRIDSHUFFLE,GRIDDELAY,VELGRIDBUTTON,GRIDMIDIROUTE,CHANNON
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
    }
    
    juce::String myProgramname;
    int grids[numOfLine][numOfStep];
    int gridVelArr[numOfLine][numOfStep];
    int numOfGrid[numOfLine];
    int octave[numOfLine];
    int gridsSpeed[numOfLine];
    int gridsDuration[numOfLine];
    int gridsVel[numOfLine];;
    int gridsEvent[numOfLine];
    int gridsShuffle[numOfLine];
    int gridsDelay[numOfLine];
    int gridsMidiRoute[numOfLine];
    int globalResyncBar;
    bool GlobalInOrFixedVel;
    bool inBuiltSynth;
    bool sortedOrFirst;
    int shuffle;
    bool channelOn;
    
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
        if (midiOutput != nullptr)
            midiOutput->sendBlockOfMessages(buffer, Time::getMillisecondCounter(), samplerate);
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
    
    
    void  readPresetToFileJSON();
    void createPrograms(juce::String preset_name );
    
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
    
    int getMidi(int line)
    {
        if(*sortedOrFirstEmptySelectAtomic == false)
        {
            std::list<juce::MidiMessage>::iterator it = inMidiNoteList.begin();
            if(inMidiNoteList.size() <= line ) return -1;
            std::advance(it, line);
            return it->getNoteNumber();
        }
        if(inMidiNoteListVector.at(line).getVelocity() != 0)
            return     inMidiNoteListVector.at(line).getNoteNumber();
        else return -1;
    }
    int getLoopMeasure()
    {
        if (myIsPlaying == false) return + 1;
        return  measureBar + 1;
    }
    void setAllValue( juce::String s,int v)
    {
        for(int i = 0 ; i < 5 ; i++)
        {
            valueTreeState.getParameterAsValue(s+std::to_string(i)).setValue(v);
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
            valueTreeState.getParameterAsValue(grid_block).setValue(x);
        }
    }
    
    void resetAllParam();
    
    void deletePreset(int);
    
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
    
private:
    String midiPortName = "Press To Select Midi Port";
    void midiHandling(juce::MidiBuffer& midiMessages,int sampleOffset, bool sampelBased = true);
    
    std::unique_ptr<MidiProcessor> midiProcessor;
    
    int stpSample[5] = {};
    juce::AudioPlayHead::CurrentPositionInfo positionInfo;
    int stepResetInterval[5] = {};
    int stepmidStopSampleInterval[5] = {-1,-1,-1,-1,-1};
    
    int stepmidStopSampleIntervalForShuffle[5][numOfStep];
    int stepResetIntervalForShuffle[5][numOfStep];
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
    
    //std::atomic<float> *numOfGrid[5];
    juce::UndoManager undoManager;
    juce::MidiBuffer myInnmidiBuffer;
    std::list<juce::MidiMessage> inMidiNoteList;
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
    std::list<RealMidiNoteList> inRealMidiNoteList;
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
