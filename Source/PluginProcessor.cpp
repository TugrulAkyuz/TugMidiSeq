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
    // we're on Windows

    String filePath;
 
#if JUCE_MAC

    filePath =  File::getSpecialLocation(File::currentApplicationFile).getChildFile ("TugMidiSeqPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);
#elif JUCE_WINDOWS
    auto a = File::getSpecialLocation(File::currentApplicationFile);
    auto b = a.getParentDirectory();
    filePath =  b.getChildFile("TugMidiSeqPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);
#endif
    juce::String  tmp_s;
    for(int j = 0 ; j <  numOfLine; j++)
    {
        for(int i = 0 ; i < numOfStep ; i++)
        {
            juce::String  grid_block;
            grid_block <<valueTreeNames[BLOCK]<< j << i;
            valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{grid_block,1}, grid_block,0,2,0));
            
            gridsArr[j][i] = valueTreeState.getRawParameterValue(grid_block);
            
        }
        tmp_s.clear();
        tmp_s << valueTreeNames[SPEEED] << j;
        //myNotetUnitSA
        
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1},tmp_s,myNotetUnitSA,13));
        gridsSpeedAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        tmp_s.clear();
        tmp_s << valueTreeNames[DUR] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1},tmp_s,myNotetUnitSA,13));
        gridsDurationAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDNUM] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,1,32,16));
        numOfGrid[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[OCTAVE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,-2,2,0));
        octave[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[VEL] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,0,127,90));
        gridsVelAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[EVENT] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,0,100,100));
        gridsEventAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        
        //*numOfGrid[j] = 16;
        //*gridsSpeedAtomic[j] = 16;
        //*gridsDurationAtomic[j] = 16;
        gridsSpeed[j] = 16;
        gridsDuration[j] = 16;
        
    }
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,1,32,1));
    globalResyncBar = valueTreeState.getRawParameterValue(tmp_s);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    GlobalInOrFixedAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
   
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    GlobalInOrFixedAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    inBuiltSynthAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    sortedOrFirstEmptySelectAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
    
    valueTreeState.state = juce::ValueTree("midiSeq"); // do not forget for valuetree
    
    
    readPresetToFileJSON();
    mySynth.setCurrentPlaybackSampleRate(mySampleRate);
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
    for(auto i = 0; i < numOfLine; i++)
    {
        mySynth.addVoice(new SynthVoice(i));
    }
    inMidiNoteListVector.resize(numOfLine);
    for(auto i = 0 ; i < numOfLine ; i++)
    inMidiNoteListVector.at(i).setVelocity(0.0f);
    
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
    return  myProgram.size();   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int TugMidiSeqAudioProcessor::getCurrentProgram()
{
    return program;
}

void TugMidiSeqAudioProcessor::setCurrentProgram (int index)
{
    if (index == 0) return;
     program = index;
//    if (hasEditor() == true)
//    {
//        auto x = getActiveEditor();
//        ((_2ruleSynthAudioProcessorEditor*)x)->myControlPanel->presetMenu.setSelectedId(index);
//    }
    String tmp_s;
   
        for(int i = 0 ; i <  numOfLine; i++)
        {
            for(int j = 0 ; j < numOfStep ; j++)
            {
                tmp_s.clear();
                tmp_s << valueTreeNames[BLOCK] << i << j;
                valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).grids[i][j]);
            }
            tmp_s.clear();
            tmp_s << valueTreeNames[SPEEED] << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).gridsSpeed[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[DUR] << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).gridsDuration[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[GRIDNUM] << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).numOfGrid[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[OCTAVE] << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).octave[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[VEL] << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).gridsVel[i]);

        }
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).globalResyncBar);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).GlobalInOrFixedVel);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).inBuiltSynth);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    valueTreeState.getParameterAsValue(tmp_s).setValue(myProgram.at(program -1).sortedOrFirst);
    
}

const juce::String TugMidiSeqAudioProcessor::getProgramName (int index)
{
    String s ;
    if(index == 0)  return   s;
    
    s = myProgram.at(index -1 ).myProgramname;
    return   s;
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
    mySynth.setCurrentPlaybackSampleRate(mySampleRate);
    for(auto i = 0 ; i <  mySynth.getNumVoices()  ; i++)
    {
        SynthVoice *v =  (SynthVoice *)mySynth.getVoice(i);
        v->prepare_to_play(sampleRate,samplesPerBlock);
    }
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
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//        // ..do something to the data...
//    }
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
          
            for (auto  &it: inMidiNoteList)
            {
                //                if(it.getNoteNumber() == currentMessage.getNoteNumber())
                //                    it.setNoteNumber(*octave[i] + currentMessage.getNoteNumber());
                //                i++;
                DBG(it.getNoteNumber());
            }
           
            for (auto i = 0 ; i < inMidiNoteListVector.size() ;i++)
            {
                if(inMidiNoteListVector.at(i).getVelocity() != 0) continue;
                
                inMidiNoteListVector.at(i) = currentMessage;
                break;;
            }
            for( auto it : inMidiNoteListVector)
            {
                DBG("ekl "+std::to_string(it.getVelocity()) );
            }
        }
        if(currentMessage.isNoteOff())
        {
            //myInnmidiBuffer.addEvent(currentMessage,samplePos );
            //  DBG(currentMessage.getNoteNumber());
            auto midiNote = [&](MidiMessage l){ return l.getNoteNumber() == currentMessage.getNoteNumber(); };
            auto it = std::find_if(inMidiNoteList.begin(), inMidiNoteList.end(), midiNote);
            if(it == inMidiNoteList.end()) return;
            //  DBG(it->getNoteNumber());
            inMidiNoteList.erase(it);
            for (auto const &it: inMidiNoteList)
                DBG(it.getNoteNumber());
            DBG("");
            erasedMidi.addEvent(currentMessage, 0);
            //            for(int j = 0 ; j <  numOfLine; j++)
            //              for(int i = 0 ; i < numOfStep ; i++)
            //            DBG(*gridsArr[j][i]);
            
            for (auto i = 0 ; i < inMidiNoteListVector.size() ;i++)
            {
                if(inMidiNoteListVector.at(i).getNoteNumber() != currentMessage.getNoteNumber()) continue;
                
                inMidiNoteListVector.at(i).setVelocity(0.0f);
                break;;
            }
            
            for( auto it : inMidiNoteListVector)
            {
                DBG("sil "+std::to_string(it.getVelocity()) );
            }
        }
        
        
        
    }
    //midiMessages.swapWith(processedMidiBuffer);
    
    midiMessages.clear();
    midiMessages.swapWith(erasedMidi);
    if (positionInfo.bpm == 0) return;
    if (positionInfo.bpm)
    {
        double mBpm =  positionInfo.bpm;
        double mBps = (float)mBpm/60;
        myBpm = mBpm;
        //  std::cout << mBps << " " ;
        
        ppq = positionInfo .ppqPosition;
  


       // in  t x = positionInfo.ppqPosition;
         
        
        stepResetInterval[0] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[1] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[2] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[3] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[4] =  1*(60*mySampleRate/ mBpm);
        
        
        for(int i = 0 ; i< numOfLine ;i++)
        {
            double first = 1.5*4*(60*mySampleRate/ mBpm); // first value in combos  number of sample musical durations and speeds
            
            int index = *gridsSpeedAtomic[i] ;
     
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            stepResetInterval[i] = first / pow(2,index); // dviding  "first" you get number of sample  for musical note time values
            
            stepLoopResetInterval[i] = stepResetInterval[i]**numOfGrid[i];
            
            first = 1.5*4*(60*mySampleRate/ mBpm);
            index = *gridsDurationAtomic[i];
       
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            
            stepmidStopSampleInterval[i] = first / pow(2,index);
            
            
        }
        
        if (myIsPlaying == false &&  positionInfo.isPlaying == true ) // stop to start
        {
            measureBar = *globalResyncBar-1;  // resync var number
            measureSample = (int)(4*mySampleRate/mBps)-1; // mBps Beat per second  ---> 1 bar  numof sample
            
           prevtimeInSamples = positionInfo.ppqPosition;; /**ppq ye bakma code*/
        }
        auto  x = positionInfo.ppqPosition; /**ppq ye bakma code*/
        midiEffectSampelDiffBitweenCall = 60*mySampleRate*( x- prevtimeInSamples)/mBpm;/**ppq ye bakma code*/
        prevtimeInSamples = positionInfo.ppqPosition;;/**ppq ye bakma code*/
        
        myIsPlaying = positionInfo.isPlaying;
        
        if(myIsPlaying == false)/**ppq ye bakma code*/
        {
            std::list<RealMidiNoteList>::iterator it;
            for (it = inRealMidiNoteList.begin(); it != inRealMidiNoteList.end(); )
            {
                it->sentMidi.setVelocity(0.0f);
                midiMessages.addEvent(it->sentMidi, 0);
                it = inRealMidiNoteList.erase(it);
            }
        }/**ppq ye bakma code*/
        
        if(midiEffectSampelDiffBitweenCall < 0) midiEffectSampelDiffBitweenCall = 0;
        
        for(int s = 0 ; s < buffer.getNumSamples();  s++)/**ppq ye bakma code*/
        { 
            measureSample++;
            measureSample %= (int)(4*mySampleRate/mBps);
            if(measureSample == 0)
            {
                measureBar++;
                measureBar %= (int)(*globalResyncBar);
                if(measureBar == 0)
                {
                    for(int i =  0; i  < numOfLine ; i++)
                    {
                        steps[i] = (int)(*numOfGrid[i]) -1;
                        sampleNumber[i] = 0;
                        stpSample[i] = stepResetInterval[i] -1;
                    }
                    
                }
            }
            
            std::list<RealMidiNoteList>::iterator it;
            for (it = inRealMidiNoteList.begin(); it != inRealMidiNoteList.end(); )
            {
                if(it->durationsample != 0) { it->durationsample--; it++ ; continue;}
                it->sentMidi.setVelocity(0.0f);
                midiMessages.addEvent(it->sentMidi, 0);
                it = inRealMidiNoteList.erase(it);
            }
            
            
            for(int i =  0; i  < numOfLine ; i++)
            {
                sampleNumber[i]++;
                stpSample[i]++;
                stpSample[i] %=stepResetInterval[i];
                if(stepmidStopSampleCounter[i] != -1)
                {
                    stepmidStopSampleCounter[i]++;
                    stepmidStopSampleCounter[i] %= stepmidStopSampleInterval[i]; // byrada uçuyor bir bak
                }
                if( stepmidStopSampleCounter[i]== 0)
                {

                    stepmidStopSampleCounter[i] = -1;
                    midiState[i] = false;
                }
                if(stpSample[i] == 0)
                {
                    
                    steps[i]++;
                    steps[i] %= (int)(*numOfGrid[i]);
                    
                    if(steps[i] == 0)
                        sampleNumber[i] = 0;
                    
                    if(*gridsArr[i][steps[i]] != 0)
                    {
                        bool  sortedofirs_Bool = inMidiNoteList.size() > i;
                        if(*sortedOrFirstEmptySelectAtomic == true)
                            sortedofirs_Bool = true;
                        
                        if(sortedofirs_Bool)
                        {
                            MidiMessage it;
                            if(*sortedOrFirstEmptySelectAtomic == false)
                            {
                               auto it1 = std::next(inMidiNoteList.begin(), i);
                                it = *it1;
                            }
                            else
                            {
                                auto it1 = &inMidiNoteListVector.at(i);
                                it = *it1;
                                if(it1->getVelocity() == 0)
                                    continue;
                            }
      
                            
                            it.setNoteNumber( it.getNoteNumber() + *octave[i]*12 );
                            
                            std::list<RealMidiNoteList>::iterator it2;
                            auto midiNote = [&](RealMidiNoteList l){ return l.sentMidi.getNoteNumber() == it.getNoteNumber(); };
                            it2 = std::find_if(inRealMidiNoteList.begin(), inRealMidiNoteList.end(),midiNote);
                            if(it2 != inRealMidiNoteList.end())
                            {
                                it2->sentMidi.setVelocity(0.0f);
                                midiMessages.addEvent(it2->sentMidi, 0);
                                it2 = inRealMidiNoteList.erase(it2);
                            }
                            
                            if(*GlobalInOrFixedAtomic == 0)
                                it.setVelocity(*gridsVelAtomic[i]/ 127.0f);
                            
                            if (myIsPlaying == false) return;
                            midiMessages.addEvent(it, 0);
                            stepmidStopSampleCounter[i] = 1;
                            RealMidiNoteList tmp;
                            tmp.sentMidi = it;
                            tmp.durationsample = stepmidStopSampleInterval[i] -1;
                            inRealMidiNoteList.push_back(tmp);
                            
                            DBG(" Channel: "  << it.getChannel());
                            midiState[i] = true;
                        }
                    }
                    
                    
                    
                    
                }
                
            }
        }
        
    }

    mySynth.renderNextBlock(buffer, midiMessages, 0 , buffer.getNumSamples());
    if(*inBuiltSynthAtomic == false)
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
        
    
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
    auto state = valueTreeState.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TugMidiSeqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (valueTreeState.state.getType()))
            valueTreeState.replaceState (ValueTree::fromXml (*xmlState));
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

