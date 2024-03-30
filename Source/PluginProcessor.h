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

#include "SynthVoice.h"


const juce::StringArray valueTreeNames = 
{
    "block","Speed","Dur","GridNum","Octave","Vel","GlobalRestncBar","GlobalInOrFixedVel","inBuiltSynth","sortedOrFirstEmptySelect","Event","Shuffle","gridshuffle"
};
enum valueTreeNamesEnum
{
    BLOCK,SPEEED,DUR,GRIDNUM,OCTAVE,VEL,GLOBALRESTBAR,GLOABLINORFIXVEL,INBUILTSYNTH,SORTEDORFIRST,EVENT,SHUFFLE,GRIDSHUFFLE
};

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
    int numOfGrid[numOfLine];
    int octave[numOfLine];
    int gridsSpeed[numOfLine];
    int gridsDuration[numOfLine];
    int gridsVel[numOfLine];;
    int gridsEvent[numOfLine];
    int gridsShuffle[numOfLine];
    int globalResyncBar;
    bool GlobalInOrFixedVel;
    bool inBuiltSynth;
    bool sortedOrFirst;
    int shuffle;
    
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
    int steps[5] = {};
    std::atomic<float> *numOfGrid[5];
    std::atomic<float> *octave[5];
    std::atomic<float> *globalResyncBar;
    float myBpm;
    float myBps;
    int measureSample = 0;
    int measureBar = 0;
    double  mySampleRate = 0;
    int  sampleNumber[5];
    int stepLoopResetInterval[5];
    bool midiState[5] = {};
    bool myIsPlaying  = false;
    
    
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
        return sampleNumber[line]*1.0/(stepLoopResetInterval[line]);
        
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
        return (float)(stepmidStopSampleIntervalForShuffle[line][step]) /stepmidStopSampleInterval[line];
        
    }
    
    void setShuffle()
    {

    }
    
    void initPrepareValue();
    
private:
    int stpSample[5] = {};
    juce::AudioPlayHead::CurrentPositionInfo positionInfo;
    int stepResetInterval[5] = {};
    int stepmidStopSampleInterval[5] = {-1,-1,-1,-1,-1};
    
    int stepmidStopSampleIntervalForShuffle[5][numOfStep];
    int stepResetIntervalForShuffle[5][numOfStep];

    int stepmidStopSampleCounter[5] = {-1,-1,-1,-1,-1};
    int  ppq = 0;
    
    int  midiEffectSampelDiffBitweenCall;
    double  prevtimeInSamples = 0;
    
    std::atomic<float> * gridsSpeedAtomic[numOfLine];
    std::atomic<float> * gridsDurationAtomic[numOfLine];
    
    std::atomic<float> *gridsVelAtomic[numOfLine];
    std::atomic<float> *gridsEventAtomic[numOfLine];
    std::atomic<float> *gridsShuffleAtomic[numOfLine];
    std::atomic<float> * GlobalInOrFixedAtomic;;
    std::atomic<float> *inBuiltSynthAtomic;
    std::atomic<float> *sortedOrFirstEmptySelectAtomic;
    std::atomic<float> *shuffleAtomic;
    
    //std::atomic<float> *numOfGrid[5];
    juce::UndoManager undoManager;
    juce::MidiBuffer myInnmidiBuffer;
    std::list<juce::MidiMessage> inMidiNoteList;
    std::vector<juce::MidiMessage> inMidiNoteListVector;
    
    struct RealMidiNoteList
    {
        juce::MidiMessage sentMidi;
        int durationsample;
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
    //juce::AudioProcessorValueTreeState::ParameterLayout createAllParameters();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessor)
};
