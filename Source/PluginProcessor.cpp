/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TugMidiSeqAudioProcessor::TugMidiSeqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
valueTreeState(*this, &undoManager)
#endif
{
   // valueTreeState.createAndAddParameter(<#std::unique_ptr<RangedAudioParameter> parameter#>);
    for(int j = 0 ; j <  numOfLine; j++)
    {
      for(int i = 0 ; i < numOfStep ; i++)
      {
        juce::String  grid_block;
        grid_block << "block" << j << i;
         valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(grid_block, grid_block,false));
        
         gridsArr[j][i] = valueTreeState.getRawParameterValue(grid_block);
        
      }
        juce::String  tmp_s; tmp_s << "Speed" << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,1,32,12));
        //gridsSpeed[j] = valueTreeState.getRawParameterValue(tmp_s);
        tmp_s.clear();
        tmp_s << "Dur" << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,1,32,12));
        gridsDurationAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
    }
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>("GridNum", "GridNum",1,32,8));
   
    
    valueTreeState.state = juce::ValueTree("midiSeq"); // do not forget for valuetree
}

TugMidiSeqAudioProcessor::~TugMidiSeqAudioProcessor()
{
}

//==============================================================================
const juce::String TugMidiSeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TugMidiSeqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TugMidiSeqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TugMidiSeqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TugMidiSeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TugMidiSeqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TugMidiSeqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TugMidiSeqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TugMidiSeqAudioProcessor::getProgramName (int index)
{
    return {};
}

void TugMidiSeqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TugMidiSeqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mySampleRate = sampleRate;
}

void TugMidiSeqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TugMidiSeqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TugMidiSeqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    juce::AudioPlayHead* playHead = getPlayHead();
   if (playHead == nullptr) return;
    juce::AudioPlayHead::CurrentPositionInfo positionInfo;
    playHead->getCurrentPosition(positionInfo);
    
    MidiBuffer::Iterator it(midiMessages);
    
    MidiMessage currentMessage;
    MidiBuffer erasedMidi;
    int samplePos;
    //myInnmidiBuffer.clear();
    while(it.getNextEvent(currentMessage,samplePos))
    {
        if(currentMessage.isNoteOn())
        {
            //myInnmidiBuffer.addEvent(currentMessage,samplePos );
            inMidiNoteList.push_back(currentMessage);
            auto comp = [](MidiMessage &l1,MidiMessage &l2){ return l1.getNoteNumber() < l2.getNoteNumber(); };
            inMidiNoteList.sort(comp);
            for (auto const &it: inMidiNoteList)
                DBG(it.getNoteNumber());
            
            DBG("");
        }
        if(currentMessage.isNoteOff())
        {
            //myInnmidiBuffer.addEvent(currentMessage,samplePos );
          //  DBG(currentMessage.getNoteNumber());
            auto midiNote = [&](MidiMessage l){ return l.getNoteNumber() == currentMessage.getNoteNumber(); };
            auto it = std::find_if(inMidiNoteList.begin(), inMidiNoteList.end(), midiNote);
          //  DBG(it->getNoteNumber());
            inMidiNoteList.erase(it);
            for (auto const &it: inMidiNoteList)
                DBG(it.getNoteNumber());
            DBG("");
            erasedMidi.addEvent(currentMessage, 0);
//            for(int j = 0 ; j <  numOfLine; j++)
//              for(int i = 0 ; i < numOfStep ; i++)
//            DBG(*gridsArr[j][i]);
        }
        
            
        
    }
    //midiMessages.swapWith(processedMidiBuffer);
    
    midiMessages.clear();
    midiMessages.swapWith(erasedMidi);
    
    if (positionInfo.bpm)
    {
        double mBpm =  positionInfo.bpm;
        double mBps = (float)mBpm/60;
         
        //  std::cout << mBps << " " ;
 
        ppq = positionInfo .ppqPosition;

        stepResetInterval[0] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[1] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[2] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[3] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[4] =  1*(60*mySampleRate/ mBpm);
        
        
        for(int i = 0 ; i< 5 ;i++)
        {
            double first = 1.5*4*(60*mySampleRate/ mBpm);
 
            //int index =  valueTreeState.getParameterAsValue(tmp_s).getValue();
            int index = gridsSpeed[i] -1 ;
            if(gridsSpeed[i] == 0) return;
            //index--;
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            stepResetInterval[i] = first / pow(2,index);
            
             first = 1.5*4*(60*mySampleRate/ mBpm);
            index = gridsDuration[i] -1 ;
            if(gridsDuration[i] == 0) return;
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            
            stepmidStopSampleInterval[i] = first / pow(2,index);
         
        }
       
    
 
        for(int s = 0 ; s < buffer.getNumSamples();  s++)
        {
            for(int i =  0; i  < 5 ; i++)
            {
                stpSample[i]++;
                stpSample[i] %=stepResetInterval[i];
                if(stepmidStopSampleCounter[i] != -1)
                    stepmidStopSampleCounter[i]++;
                stepmidStopSampleCounter[i] %= stepmidStopSampleInterval[i];
               if( stepmidStopSampleCounter[i]== 0)
               {
                   auto it1 = std::next(inMidiNoteList.begin(), i);
                   MidiMessage it = *it1;
                   it.setVelocity(0.0f);
                   midiMessages.addEvent(it, 0);
                   stepmidStopSampleCounter[i] = -1;
               }
                if(stpSample[i] == 0)
                {
                    int prevstep = steps[i];
                    steps[i]++;
                    steps[i] %= 8;
                    

                    
                    if(*gridsArr[i][steps[i]] == 1)
                    {
                        if(inMidiNoteList.size() <= i)
                            break;;
                        
                           
                        auto it1 = std::next(inMidiNoteList.begin(), i);
                        
                        MidiMessage it = *it1;
//                        float vel = it.getFloatVelocity();
//                        it.setVelocity(0.0f);
//                        midiMessages.addEvent(it, 0);
//                        it.setVelocity(vel);
                        midiMessages.addEvent(it, 0);
                        stepmidStopSampleCounter[i] = 1;
                        DBG(" Line: "  << i << " strep: " << steps[i]);
                    }

                 
//                  else
//                    {
//                        if(*gridsArr[i][prevstep] == 1)
//                        {
//                            if(inMidiNoteList.size() <= i)
//                                break;
//                            auto it1 = std::next(inMidiNoteList.begin(), i);
//                            MidiMessage it = *it1;
//                            it.setVelocity(0.0f);
//                            midiMessages.addEvent(it, 0);
//                        }
//                    }
                  
                }
            }
        }
        
    }

    
    
}

//==============================================================================
bool TugMidiSeqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TugMidiSeqAudioProcessor::createEditor()
{
    
    return  new TugMidiSeqAudioProcessorEditor (*this);
    
}

//==============================================================================
void TugMidiSeqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TugMidiSeqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TugMidiSeqAudioProcessor();
}

//juce::AudioProcessorValueTreeState::ParameterLayout createAllParameters()
//{
//std::vector <std::unique_ptr <juce::AudioProcessorValueTreeState::ParameterLayout>> params;
//    
//    return {params.begin(),params.end()};
//}

