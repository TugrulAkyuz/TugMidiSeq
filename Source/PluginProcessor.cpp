/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::File *resourceJsonFile;

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

    String filePath =  File::getSpecialLocation(File::currentApplicationFile).getChildFile ("TugSynth.txt").getFullPathName();
   // File pajLogoFile = filePath
 
#if JUCE_MAC

    filePath =  File::getSpecialLocation(File::currentApplicationFile).getChildFile ("2RuleSynthPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);
#elif JUCE_WINDOWS
    auto a = File::getSpecialLocation(File::currentApplicationFile);
    auto b = a.getParentDirectory();
    filePath =  b.getChildFile("2RuleSynthPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);
#endif
    juce::String  tmp_s;
    for(int j = 0 ; j <  numOfLine; j++)
    {
        for(int i = 0 ; i < numOfStep ; i++)
        {
            juce::String  grid_block;
            grid_block <<valueTreeNames[BLOCK]<< j << i;
            valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(grid_block, grid_block,false));
            
            gridsArr[j][i] = valueTreeState.getRawParameterValue(grid_block);
            
        }
        tmp_s.clear();
        tmp_s << valueTreeNames[SPEEED] << j;
        //myNotetUnitSA
        
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(tmp_s,tmp_s,myNotetUnitSA,13));
        gridsSpeedAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        tmp_s.clear();
        tmp_s << valueTreeNames[DUR] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(tmp_s,tmp_s,myNotetUnitSA,13));
        gridsDurationAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDNUM] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,1,32,16));
        numOfGrid[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[OCTAVE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,-2,2,0));
        octave[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[VEL] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,0,127,90));
        gridsVelAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        
      
        
        //*numOfGrid[j] = 16;
        //*gridsSpeedAtomic[j] = 16;
        //*gridsDurationAtomic[j] = 16;
        gridsSpeed[j] = 16;
        gridsDuration[j] = 16;
        
    }
    tmp_s.clear();
    tmp_s << "GlobalRestncBar";
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterInt>(tmp_s, tmp_s,1,32,1));
    globalResyncBar = valueTreeState.getRawParameterValue(tmp_s);
    
    tmp_s.clear();
    tmp_s << "GlobalInOrFixedVel";
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(tmp_s, tmp_s,false));
    GlobalInOrFixedAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
    
    valueTreeState.state = juce::ValueTree("midiSeq"); // do not forget for valuetree
    
    
    readPresetToFileJSON();
    
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
    return 0;
}

void TugMidiSeqAudioProcessor::setCurrentProgram (int index)
{
    if (index == 0) return;
    int program = index;
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
            int i = 0;
            for (auto  &it: inMidiNoteList)
            {
                //                if(it.getNoteNumber() == currentMessage.getNoteNumber())
                //                    it.setNoteNumber(*octave[i] + currentMessage.getNoteNumber());
                //                i++;
                DBG(it.getNoteNumber());
            }
            
            DBG("");
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
  

        
        stepResetInterval[0] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[1] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[2] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[3] =  1*(60*mySampleRate/ mBpm);
        stepResetInterval[4] =  1*(60*mySampleRate/ mBpm);
        
        
        for(int i = 0 ; i< 5 ;i++)
        {
            double first = 1.5*4*(60*mySampleRate/ mBpm);
            
            //int index =  valueTreeState.getParameterAsValue(tmp_s).getValue();
            int index = *gridsSpeedAtomic[i] ;
            //            if(*gridsSpeedAtomic[i] <= 0)
            //                return;
            //index--;
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            stepResetInterval[i] = first / pow(2,index);
            
            stepLoopResetInterval[i] = stepResetInterval[i]**numOfGrid[i];
            
            first = 1.5*4*(60*mySampleRate/ mBpm);
            index = *gridsDurationAtomic[i];
            //            if(gridsDuration[i] == 0)
            //                return;
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            
            stepmidStopSampleInterval[i] = first / pow(2,index);
            
            
        }
        if (myIsPlaying == false &&  positionInfo.isPlaying == true )
        {
            measureBar = *globalResyncBar-1;
            measureSample = (int)(4*mySampleRate/mBps)-1;
        }
        myIsPlaying = positionInfo.isPlaying;
        
        
        for(int s = 0 ; s < buffer.getNumSamples();  s++)
        {
            measureSample++;
            measureSample %= (int)(4*mySampleRate/mBps);
            if(measureSample == 0)
            {
                measureBar++;
                measureBar %= (int)(*globalResyncBar);
                if(measureBar == 0)
                {
                    for(int i =  0; i  < 5 ; i++)
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
            
            
            for(int i =  0; i  < 5 ; i++)
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
                    //                   auto it1 = std::next(inMidiNoteList.begin(), i);
                    //                   MidiMessage it = *it1;
                    //                   it.setVelocity(0.0f);
                    //                   midiMessages.addEvent(it, 0);
                    stepmidStopSampleCounter[i] = -1;
                    midiState[i] = false;
                }
                if(stpSample[i] == 0)
                {
                    
                    steps[i]++;
                    steps[i] %= (int)(*numOfGrid[i]);
                    
                    if(steps[i] == 0)
                        sampleNumber[i] = 0;
                    
                    if(*gridsArr[i][steps[i]] == 1)
                    {
                        if(inMidiNoteList.size() > i)
                        {
                            
                            
                            auto it1 = std::next(inMidiNoteList.begin(), i);
                            
                            MidiMessage it = *it1;
                            //                        float vel = it.getFloatVelocity();
                            //                        it.setVelocity(0.0f);
                            //                        midiMessages.addEvent(it, 0);
                            //                        it.setVelocity(vel);
                            
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


void TugMidiSeqAudioProcessor::resetAllParam()
{
    String tmp_s;
    for(int j = 0 ; j <  numOfLine; j++)
    {
        for(int i = 0 ; i < numOfStep ; i++)
        {
            tmp_s.clear();
            tmp_s << valueTreeNames[BLOCK] << j << i;
            valueTreeState.getParameterAsValue(tmp_s).setValue(0);
        }
        tmp_s.clear();
        tmp_s << valueTreeNames[SPEEED] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(13);
        tmp_s.clear();
        tmp_s << valueTreeNames[DUR] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(13);
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDNUM] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(16);
        tmp_s.clear();
        tmp_s << valueTreeNames[OCTAVE] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(0);
        tmp_s.clear();
        tmp_s << valueTreeNames[VEL] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(90);
        tmp_s.clear();
        tmp_s << valueTreeNames[GLOBALRESTBAR] << j;
        valueTreeState.getParameterAsValue(tmp_s).setValue(1);
        
        
      
    }
    
}

void  TugMidiSeqAudioProcessor::writePresetToFileJSON()
{
    String strName;
    
    
    juce::FileOutputStream outputStream (*resourceJsonFile);
    outputStream.setPosition(0);
    
    DynamicObject* tree = new DynamicObject();
    Array<var> arr;
    juce::String  tmp_s;
    for(auto p = 0 ; p < myProgram.size(); p++)
    {
        var newObj ( new DynamicObject() );
        String ss = myProgram.at(p).myProgramname;
        newObj.getDynamicObject()->setProperty("PresetName",ss);
        //------------
        for(int i = 0 ; i < numOfLine; i++)
        {
            for(int j = 0 ; j < numOfStep ; j++)
          {
              tmp_s.clear();
              tmp_s <<valueTreeNames[BLOCK]<< i << j;
              auto v = myProgram.at(p).grids[i][j];
              newObj.getDynamicObject()->setProperty(tmp_s,v);
          }
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[SPEEED]<< i;
            auto v = myProgram.at(p).gridsSpeed[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[DUR]<< i;
             v = myProgram.at(p).gridsDuration[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[GRIDNUM]<< i;
             v = myProgram.at(p).numOfGrid[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[OCTAVE]<< i;
             v = myProgram.at(p).octave[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[VEL]<< i;
             v = myProgram.at(p).gridsVel[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            
        }
        arr.add(newObj);
    }
    tree->setProperty("Presets",arr);
    
    var json (tree);
    String sil_string = JSON::toString(json);
    DBG(sil_string);
 
    outputStream.writeString(sil_string);
    outputStream.flush();
    
}

void  TugMidiSeqAudioProcessor::readPresetToFileJSON()
{
    juce::FileInputStream inputStream (*resourceJsonFile);
    if (inputStream.failedToOpen())
          return;
    String sil_string = inputStream.readString();
    juce::var parsedJson;
 
    var jsonReply = JSON::parse(sil_string);
    Array<var>* presetArray= jsonReply.getProperty("Presets", var()).getArray();
    juce::String  tmp_s;
  
    for (auto& preset : *presetArray)
    {
        String strName = preset.getProperty("PresetName", var()).toString();
        preset_idex++;
        
        float v;
        
        TugMidiSeqProgram p (strName);
        
        for(int i = 0 ; i < numOfLine; i++)
        {
            for(int j = 0 ; j < numOfStep ; j++)
          {
              tmp_s.clear();
              tmp_s <<valueTreeNames[BLOCK]<< i << j;
              v = preset.getProperty(tmp_s, var());
              p.grids[i][j] = v;
          }
            tmp_s.clear();
            tmp_s <<valueTreeNames[SPEEED]<< i;
            v = preset.getProperty(tmp_s, var());
            p.gridsSpeed[i] = v;
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[DUR]<< i;
            v = preset.getProperty(tmp_s, var());
            p.gridsDuration[i] = v;
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[GRIDNUM]<< i;
            v = preset.getProperty(tmp_s, var());
            p.numOfGrid[i] = v;
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[OCTAVE]<< i;
            v = preset.getProperty(tmp_s, var());
            p.octave[i] = v;
    
            tmp_s.clear();
            tmp_s <<valueTreeNames[VEL]<< i;
            v = preset.getProperty(tmp_s, var());
            p.gridsVel[i] = v;
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[VEL]<< i;
            v = preset.getProperty(tmp_s, var());
            p.gridsVel[i] = v;
        }
        myProgram.push_back(p);
    }

}
void TugMidiSeqAudioProcessor::createPrograms(juce::String preset_name )
{
        
        preset_idex++;
        String strName;
    juce::String  tmp_s;
         strName = preset_name;
        TugMidiSeqProgram paramProg (strName);
    for(int i = 0 ; i < numOfLine; i++)
    {
        for(int j = 0 ; j < numOfStep ; j++)
      {
          tmp_s.clear();
          tmp_s <<valueTreeNames[BLOCK]<< i << j;
          
          paramProg.grids[i][j] = *valueTreeState.getRawParameterValue(tmp_s);
      }
        tmp_s.clear();
        tmp_s <<valueTreeNames[SPEEED]<< i;
    
        paramProg.gridsSpeed[i] = *valueTreeState.getRawParameterValue(tmp_s);;
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[DUR]<< i;
 
        paramProg.gridsDuration[i] = *valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDNUM]<< i;
 
        paramProg.numOfGrid[i] = *valueTreeState.getRawParameterValue(tmp_s);;
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[OCTAVE]<< i;
 
        paramProg.octave[i] = *valueTreeState.getRawParameterValue(tmp_s);;

        tmp_s.clear();
        tmp_s <<valueTreeNames[VEL]<< i;
    
        paramProg.gridsVel[i] = *valueTreeState.getRawParameterValue(tmp_s);;
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[VEL]<< i;
 
        paramProg.gridsVel[i] = *valueTreeState.getRawParameterValue(tmp_s);;
    }
    myProgram.push_back(paramProg);
    writePresetToFileJSON();
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

