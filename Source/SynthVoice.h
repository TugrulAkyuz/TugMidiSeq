/*
 ==============================================================================
 
 SynthVoice.h
 Created: 30 May 2022 8:29:19am
 Author:  Tuğrul Akyüz
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

using namespace juce;

class SynthSound : public SynthesiserSound
{
    
public:
    bool appliesToNote (int)
    {
        return true;
        
    }
    bool appliesToChannel (int)
    {
        return true;
        
    }
    
    
private:
    
    
};

class WaveTableList {
    
public:
    WaveTableList(int lenght)
    {
        
        
        for(int i = 0 ; i  < lenght;i++) //sinus
        wavetable[0].insert(i, (double) sin(2.0*i*double_Pi/lenght));
        
        for(int i = 0 ; i  < lenght;i++) // saw
        {
            wavetable[1].set(i, ((double)(i)*2.0 / lenght) - 1);
        }
        
        for(int i = 0 ; i  < lenght;i++)  // triangle
        wavetable[2].insert(i, 4*fabs( ((double)i-lenght/2)/lenght)-1);
        
        for(int i = 0 ; i  < lenght;i++) // square
        if(i < lenght/2)
            wavetable[3].insert(i,1);
        else
            wavetable[3].insert(i,-1);
        
        for(int i = 0 ; i  < lenght;i++)  // random
        wavetable[4].set(i,random.nextDouble() - 0.5f);
    }
    
    
    Array< double> wavetable[5];
    Random random;
    
    
};


class OscWaves
{
public:
    OscWaves(WaveTableList * wt,int lenght,bool lfo_f);
    double get_nextSample(double freq, float ph, int waveType);
    double get_nextSample(double freq,int waveType);
    double poly_blep(double t);
    
    void  setSampleRate(int smprt);
    double linearInterpol(float v0, float v1, float t)
    {
        
        return (1 - t) * v0 + t * v1;
    }
private:
    Array< double> wavetable[5];
    Random random;
    double frequency;
    double increment;
    double phase;
    
    double sampleLen;
    double sampleRate;
    bool lfo_flag;
    float mPhase;
    WaveTableList *waveTableList;
    
};


class SynthVoice : public SynthesiserVoice
{
    
public:
    
    SynthVoice(int index);
    
    ~SynthVoice();
    void prepare_to_play(double sampleRate,int samplesPerBlock);
    bool canPlaySound (SynthesiserSound *sound);
    
    void startNote (int midiNoteNumber,
                    float velocity,
                    SynthesiserSound* sound,
                    int currentPitchWheelPosition);
    void stopNote (float velocity, bool allowTailOff);
    
    void pitchWheelMoved (int newPitchWheelValue);
    void controllerMoved (int controllerNumber, int newControllerValue);
    
    
    void renderNextBlock (AudioBuffer<float>& outputBuffer,
                          int startSample,
                          int numSamples);
    
private:
    //maxiOsc osc;
    double frequency;
    WaveTableList waveTable;
    int myMidiNoteNumber;
    double level;
    OscWaves oscWaves;
    bool startNoteFlag;
    SmoothedValue<float> smoothLevel;
    
    double calculatePitch(int pitch)
    {
        
        int tmp_max = 8192*2;
        float ratio =  ((double)(pitch -  tmp_max/2))/(tmp_max/2);
        float m =pow(2,ratio);
        
        return(frequency * m);
        
    }
    
    //     void handleMidiEvent    (    const MidiMessage &         )
    //    {
    //
    //    }
    
};

