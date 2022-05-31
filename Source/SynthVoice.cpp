/*
 ==============================================================================
 
 SynthVoice.cpp
 Created: 30 May 2022 8:29:19am
 Author:  Tuğrul Akyüz
 
 ==============================================================================
 */

#include "SynthVoice.h"

OscWaves::OscWaves(WaveTableList * wt,int lenght,bool lfo_f)
{
    waveTableList = wt;
    phase = 0;
    lfo_flag = lfo_f;
    sampleLen = lenght;
    mPhase = 0;
    
    
}
double OscWaves::get_nextSample(double freq, float ph, int waveType)
{
    mPhase = mPhase + 0.01*(ph - mPhase);
    // mPhase = ph;
    return get_nextSample( freq, waveType);
    
}
double OscWaves::get_nextSample(double freq,int waveType)
{
    double t;
    if(lfo_flag ==  true && waveType == 0)
    {
        increment = freq / sampleRate;
        //phase += mPhase;
        phase += increment;
        // DBG(" aaa " + std::to_string(cos(mPhase + 2*3.14*increment)));
        return cos(2*3.14*(mPhase + phase));;
        
    }
    increment = freq * sampleLen /sampleRate;
    //phase += mPhase;
    phase += increment;
    if( phase >= sampleLen)  phase -= sampleLen;
    if( phase < 0)  phase += sampleLen;
    t = phase/sampleLen;
    int  index  = (int)phase;
    float t2 = phase - index;
    double value;
    int next_index = (index +1)%(int)sampleLen;
    if(waveType == 1)
    {
        value = waveTableList->wavetable[waveType][index];
        value -= poly_blep(t);
        return value;
        
    }
    if(waveType == 3)
    {
        value = waveTableList->wavetable[waveType][index];
        value += poly_blep(t);
        value -= poly_blep(fmod(t + 0.5, 1.0));
        return value;
    }
    
    if(waveType == 4)
    {
        
        value = random.nextDouble() * 0.25f - 0.125f;
        return value;
    }
    
    value = linearInterpol(waveTableList->wavetable[waveType][index],waveTableList->wavetable[waveType][next_index],t2);
    return value;
    
}
double OscWaves::poly_blep(double t)
{
    if(lfo_flag == true)  return (0.0f);
    double dt = increment / sampleLen;
    // 0 <= t < 1
    if (t < dt) {
        t /= dt;
        return t+t - t*t - 1.0;
    }
    // -1 < t < 0
    else if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    // 0 otherwise
    else return 0.0;
}

void  OscWaves::setSampleRate(int smprt)
{
    // std::cout << endl <<  "Osc Waves Sample rate Set  : "  <<  smprt  << endl  ;
    sampleRate =  smprt;
    
    
}



SynthVoice::SynthVoice(int index) : waveTable(1024), oscWaves(&waveTable,1024,false)

{
    
    oscWaves.setSampleRate(getSampleRate());
    level = 0;
    frequency = 0;
    
}

SynthVoice::~SynthVoice()
{
    
    
}
void SynthVoice::prepare_to_play(double sampleRate,int samplesPerBlock)
{
    
    oscWaves.setSampleRate(sampleRate);
    smoothLevel.reset(sampleRate, 0.01f);
    
}
bool SynthVoice::canPlaySound (SynthesiserSound *sound)
{
    
    return dynamic_cast<SynthSound*>(sound) != nullptr;
    
    
}

void SynthVoice::startNote (int midiNoteNumber,
                            float velocity,
                            SynthesiserSound* sound,
                            int currentPitchWheelPosition)
{
    
    
    myMidiNoteNumber = midiNoteNumber;
    level = velocity/5;
    frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    
    smoothLevel.setTargetValue(level);
    
}
void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    if(!allowTailOff)
        clearCurrentNote();
    level = velocity;
    smoothLevel.setTargetValue(0.0f);
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    
    
    
}
void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    
}



void SynthVoice::renderNextBlock (AudioBuffer<float>& outputBuffer,
                                  int startSample,
                                  int numSamples)
{
    
    
    for(int sample = 0 ; sample  <  numSamples; sample++)
    {
        
        auto theSound = smoothLevel.getNextValue()*oscWaves.get_nextSample(frequency,0);
        
        for(int channel = 0 ; channel  <  outputBuffer.getNumChannels(); channel++)
        {
            outputBuffer.addSample(channel, startSample,  theSound);
            
            
        }
        startSample++;
        
    }
    
}
