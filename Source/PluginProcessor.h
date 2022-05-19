/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define numOfStep  32
#define numOfLine  5



const std::vector <juce::String> myNotetUnit =
{ "1nd","1n", "1nt",
  "2nd","2n","2nt",
  "4nd","4n","4nt",
    "8nd","8n","8nt",
    "16nd","16n","16nt",
    "32nd","32n","32nt",
    "64nd","64n","64nt",
    "128nd","128n","128nt" };
                   
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
    
    juce::AudioProcessorValueTreeState valueTreeState;
    int getSteps(int i)
    {
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

private:
    int stpSample[5] = {};
    int steps[5] = {};
    int stepResetInterval[5] = {};
    int stepmidStopSampleInterval[5] = {-1,-1,-1,-1,-1};
    int stepmidStopSampleCounter[5] = {-1,-1,-1,-1,-1};
    int  ppq;
    double  mySampleRate;
    
    std::atomic<float> * gridsArr[numOfLine][numOfStep];
    std::atomic<float> * gridsSpeedAtomic[numOfLine];
    std::atomic<float> * gridsDurationAtomic[numOfLine];
    juce::UndoManager undoManager;
    juce::MidiBuffer myInnmidiBuffer;
    std::list<juce::MidiMessage> inMidiNoteList;
    double gridsSpeed[numOfLine];
    double gridsDuration[numOfLine];
    //juce::AudioProcessorValueTreeState::ParameterLayout createAllParameters();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TugMidiSeqAudioProcessor)
};
