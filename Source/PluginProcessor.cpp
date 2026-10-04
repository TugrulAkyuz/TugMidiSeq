/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

ChangeBroadcaster myGridChangeListener;
ChangeBroadcaster updateMidiPort;;
juce::CriticalSection midiOutputMutex;

// AudioParameterInt does not report isDiscrete(), so hosts/validators (e.g.
// pluginval's state-restoration test) treat coarse integer parameters as
// continuous and flag them as "not restored" when a raw normalized test value
// rounds more than the tolerance away from the nearest legal step. These
// parameters are genuinely discrete, so advertise that.
class DiscreteAudioParameterInt : public juce::AudioParameterInt
{
public:
    using juce::AudioParameterInt::AudioParameterInt;
    bool isDiscrete() const override { return true; }
};
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
    // macOS: Standard Audio Presets location (~/Library/Audio/Presets/)
    auto userHome = File::getSpecialLocation(File::userHomeDirectory);
    File presetDir = userHome.getChildFile("Library")
                           .getChildFile("Audio")
                           .getChildFile("Presets")
                           .getChildFile("2Rule")
                           .getChildFile("TugMidiSeq");

    if (!presetDir.exists())
        presetDir.createDirectory();

    filePath = presetDir.getChildFile("TugMidiSeqPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);

#elif JUCE_WINDOWS
    // Windows: Standard AppData location
    auto appSupport = File::getSpecialLocation(File::userApplicationDataDirectory);
    File presetDir = appSupport.getChildFile("2Rule")
        .getChildFile("TugMidiSeq");

    if (!presetDir.exists())
        presetDir.createDirectory();

    filePath = presetDir.getChildFile("TugMidiSeqPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);

#elif JUCE_LINUX
    // Linux: XDG config location (~/.config/2Rule/TugMidiSeq)
    auto configDir = File::getSpecialLocation(File::userApplicationDataDirectory);
    File presetDir = configDir.getChildFile("2Rule")
        .getChildFile("TugMidiSeq");

    if (!presetDir.exists())
        presetDir.createDirectory();

    filePath = presetDir.getChildFile("TugMidiSeqPresets.json").getFullPathName();
    resourceJsonFile = new File(filePath);

#endif
    // persist the user-chosen single-preset folder across sessions
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "TugMidiSeq";
        opts.filenameSuffix      = "settings";
        opts.osxLibrarySubFolder = "Application Support/2Rule/TugMidiSeq";
        opts.folderName          = "2Rule/TugMidiSeq";
        appProperties.setStorageParameters(opts);
    }
    presetFolder = resourceJsonFile->getParentDirectory();  // default
    if (auto* props = appProperties.getUserSettings())
    {
        String savedPath = props->getValue("presetFolder", "");
        if (savedPath.isNotEmpty())
        {
            File saved(savedPath);
            if (saved.isDirectory()) presetFolder = saved;
        }
    }

    juce::String  tmp_s;
    for(int j = 0 ; j <  numOfLine; j++)
    {
        for(int i = 0 ; i < numOfStep ; i++)
        {
            juce::String  grid_block;
            grid_block <<valueTreeNames[BLOCK]<< j << i;
            valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{grid_block,1}, grid_block,0,2,0));
            
            gridsArr[j][i] = valueTreeState.getRawParameterValue(grid_block);
            
            grid_block.clear();
            grid_block <<valueTreeNames[VELGRIDBUTTON]<< j << i;
            valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{grid_block,1}, grid_block,0,127,100));
            gridVelArrAtomic[j][i] = valueTreeState.getRawParameterValue(grid_block);
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
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,2,32,16));
        numOfGrid[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[OCTAVE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,-2,2,0));
        octave[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[VEL] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,0,127,90));
        gridsVelAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[EVENT] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,1,100,50));
        gridsEventAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
       
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDSHUFFLE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,-75,75,0));
        gridsShuffleAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDDELAY] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,-99,99,0));
        gridsDelayAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
       
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDMIDIROUTE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,1,16,j + 10));
        gridsMidiRouteAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);

        //*numOfGrid[j] = 16;
        //*gridsSpeedAtomic[j] = 16;
        //*gridsDurationAtomic[j] = 16;
        gridsSpeed[j] = 16;
        gridsDuration[j] = 16;
        
    }
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,1,32,1));
    globalResyncBar = valueTreeState.getRawParameterValue(tmp_s);
    
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
    
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s,-75,75,0));
    shuffleAtomic = valueTreeState.getRawParameterValue(tmp_s);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[CHANNON];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    channelOnAtamic = valueTreeState.getRawParameterValue(tmp_s);

    tmp_s.clear();
    tmp_s << valueTreeNames[LATCH];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    latchAtomic = valueTreeState.getRawParameterValue(tmp_s);

    // appended after everything else so existing parameter indices don't move
    for (int j = 0; j < numOfLine; j++)
    {
        tmp_s.clear();
        tmp_s << valueTreeNames[DIRECTION] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1}, tmp_s, directionNames, DirForward));
        gridsDirectionAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
    }

    // held by the Fill button; steps with a FILL / !FILL condition follow it
    tmp_s.clear();
    tmp_s << valueTreeNames[FILL];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s,false));
    fillAtomic = valueTreeState.getRawParameterValue(tmp_s);

    // scale lock (Scale "Off" by default, so nothing changes until it's set)
    tmp_s.clear();
    tmp_s << valueTreeNames[SCALEKEY];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1}, tmp_s, scaleKeyNames, 0));
    scaleKeyAtomic = valueTreeState.getRawParameterValue(tmp_s);
    tmp_s.clear();
    tmp_s << valueTreeNames[SCALETYPE];
    valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1}, tmp_s, scaleTypeNames, 0));
    scaleTypeAtomic = valueTreeState.getRawParameterValue(tmp_s);

    for (int j = 0; j < numOfLine; j++)
    {
        tmp_s.clear();
        tmp_s << valueTreeNames[MUTATE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s, 0, 100, 0));
        gridsMutateAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);

        tmp_s.clear();
        tmp_s << valueTreeNames[PLAYMODE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(ParameterID{tmp_s,1}, tmp_s, playModeNames, PlayVoice));
        gridsPlayModeAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);

        tmp_s.clear();
        tmp_s << valueTreeNames[SPREAD] << j;
        valueTreeState.createAndAddParameter(std::make_unique<DiscreteAudioParameterInt>(ParameterID{tmp_s,1}, tmp_s, -100, 100, 20));
        gridsSpreadAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
        pubMutateMask[j].store (0);
        mutateResetRequest[j].store (false);
    }

    for (int j = 0; j < numOfLine; j++)
    {
        tmp_s.clear();
        tmp_s << valueTreeNames[MUTE] << j;
        valueTreeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool>(ParameterID{tmp_s,1}, tmp_s, false));
        gridsMuteAtomic[j] = valueTreeState.getRawParameterValue(tmp_s);
    }

    for (int j = 0; j < numOfLine; j++)
    {
        const juce::String n (j);
        auto addInt = [&] (int base, int lo, int hi, int def)
        {
            const auto id = valueTreeNames[base] + n;
            valueTreeState.createAndAddParameter (std::make_unique<DiscreteAudioParameterInt> (ParameterID { id, 1 }, id, lo, hi, def));
            return valueTreeState.getRawParameterValue (id);
        };
        const auto shapeId = valueTreeNames[STRUMSHAPE] + n;
        valueTreeState.createAndAddParameter (std::make_unique<juce::AudioParameterChoice> (ParameterID { shapeId, 1 }, shapeId, strumShapeNames, StrumLinear));
        gridsStrumShapeAtomic[j]   = valueTreeState.getRawParameterValue (shapeId);
        gridsStrumTensionAtomic[j] = addInt (STRUMTENSION, -100, 100, 0);
        gridsStrumVelAtomic[j]     = addInt (STRUMVEL,     -100, 100, 0);
        gridsStrumHumanAtomic[j]   = addInt (STRUMHUMAN,      0, 100, 0);
    }

    {
        const auto slotId = valueTreeNames[PATTERNSLOT];
        valueTreeState.createAndAddParameter (std::make_unique<juce::AudioParameterChoice> (ParameterID { slotId, 1 }, slotId, slotNames, 0));
        patternSlotAtomic = valueTreeState.getRawParameterValue (slotId);
    }

    // C++17: std::atomic members start uninitialised
    for (int i = 0; i < numOfLine; i++)
    {
        activeSlot[i].store (0);
        mirrorDirty[i].store (false);
    }
    resetSlots();
    for (int i = 0; i < numOfLine; i++)   // every pad edit lands in its lane's active slot
        for (int j = 0; j < numOfStep; j++)
            for (bool vel : { false, true })
            {
                cellMirrors.push_back (std::make_unique<CellMirror> (*this, i, j, vel));
                valueTreeState.addParameterListener (valueTreeNames[vel ? VELGRIDBUTTON : BLOCK] + juce::String (i) + juce::String (j),
                                                     cellMirrors.back().get());
            }
    for (int i = 0; i < numOfLine; i++)
    {
        laneInNote[i].store (-1);
        laneOutNote[i].store (-1);
        pubPlayStep[i].store (-1);
        pubPlayFrac[i].store (0.0f);
        pubBackward[i].store (false);
    }
    for (int i = 0; i < 2; i++)
    {
        heldMask[i].store (0);
        physMask[i].store (0);
    }
    resetTrigCondState();

    valueTreeState.state = juce::ValueTree("midiSeq"); // do not forget for valuetree

    
    readPresetToFileJSON();        // 1) legacy bundle file (backward compat)
    readSinglePresetFilesJSON();   // 2) single-preset files from presetFolder
    mySynth.setCurrentPlaybackSampleRate(mySampleRate);
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
    for(auto i = 0; i < numOfLine; i++)
    {
        mySynth.addVoice(new SynthVoice(i));
    }
    inMidiNoteListVector.resize(numOfLine);
    //inMidiNoteListVectorTmp.resize(numOfLine);
    for(auto i = 0 ; i < numOfLine ; i++)
    {
        inMidiNoteListVector.at(i).setVelocity(0.0f);
      //  inMidiNoteListVectorTmp.at(i).setVelocity(0.0f);
    }
    inMidiNoteList.reserve(128);
    inRealMidiNoteList.reserve(32);
    midiMessagesStack.ensureSize(2048);
    
    program = 1;
    positionInfo.bpm = 120;
    mySampleRate = 44100;
    initPrepareValue();
    

    midiProcessor = std::make_unique<MidiProcessor>();
    mirrorTimer.startTimer (30);

}

TugMidiSeqAudioProcessor::~TugMidiSeqAudioProcessor()
{
    mirrorTimer.stopTimer();
    for (auto& m : cellMirrors)
        valueTreeState.removeParameterListener (valueTreeNames[m->velocity ? VELGRIDBUTTON : BLOCK] + juce::String (m->line) + juce::String (m->step), m.get());
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
    if(index > myProgram.size()) return;
     program = index;
//    if (hasEditor() == true)
//    {
//        auto x = getActiveEditor();
//        ((_2ruleSynthAudioProcessorEditor*)x)->myControlPanel->presetMenu.setSelectedId(index);
//    }
    // Hosts may call this from any thread, so every write goes through
    // setParamValue() rather than the message-thread-only juce::Value API.
    const auto& prog = myProgram.at(program - 1);
    String tmp_s;
    // The undo history is message-thread only; a load from the GUI is undoable
    // (GlobalPanel wraps it), one from a host thread just sets the conditions.
    const bool undoable = juce::MessageManager::existsAndIsCurrentThread();

        for(int i = 0 ; i <  numOfLine; i++)
        {
            for(int j = 0 ; j < numOfStep ; j++)
            {
                tmp_s.clear();
                tmp_s << valueTreeNames[BLOCK] << i << j;
                setParamValue(tmp_s, prog.grids[i][j]);

                tmp_s.clear();
                tmp_s << valueTreeNames[VELGRIDBUTTON] << i << j;
                setParamValue(tmp_s, prog.gridVelArr[i][j]);

                if (undoable)
                {
                    setStepCondUndoable (i, j, prog.stepCond[i][j]);
                    setStepRatchetUndoable (i, j, prog.stepRatchet[i][j]);
                    setStepPitchUndoable (i, j, prog.stepPitch[i][j]);
                }
                else
                {
                    setStepCond (i, j, prog.stepCond[i][j]);
                    setStepRatchet (i, j, prog.stepRatchet[i][j]);
                    setStepPitch (i, j, prog.stepPitch[i][j]);
                }
            }
            tmp_s.clear();
            tmp_s << valueTreeNames[SPEEED] << i;
            setParamValue(tmp_s, prog.gridsSpeed[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[DUR] << i;
            setParamValue(tmp_s, prog.gridsDuration[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[GRIDNUM] << i;
            setParamValue(tmp_s, prog.numOfGrid[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[OCTAVE] << i;
            setParamValue(tmp_s, prog.octave[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[VEL] << i;
            setParamValue(tmp_s, prog.gridsVel[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[EVENT] << i;
            setParamValue(tmp_s, prog.gridsEvent[i]);
            tmp_s.clear();
            tmp_s << valueTreeNames[GRIDSHUFFLE] << i;
            setParamValue(tmp_s, prog.gridsShuffle[i]);

            tmp_s.clear();
            tmp_s << valueTreeNames[GRIDDELAY] << i;
            setParamValue(tmp_s, prog.gridsDelay[i]);

            tmp_s.clear();
            tmp_s << valueTreeNames[GRIDMIDIROUTE] << i;
            setParamValue(tmp_s, prog.gridsMidiRoute[i]);

            tmp_s.clear();
            tmp_s << valueTreeNames[DIRECTION] << i;
            setParamValue(tmp_s, prog.direction[i]);

            tmp_s.clear();
            tmp_s << valueTreeNames[MUTATE] << i;
            setParamValue(tmp_s, prog.mutate[i]);

            setParamValue(valueTreeNames[PLAYMODE] + juce::String (i), prog.playMode[i]);
            setParamValue(valueTreeNames[SPREAD] + juce::String (i), prog.spread[i]);
            setParamValue(valueTreeNames[STRUMSHAPE] + juce::String (i), prog.strumShape[i]);
            setParamValue(valueTreeNames[STRUMTENSION] + juce::String (i), prog.strumTension[i]);
            setParamValue(valueTreeNames[STRUMVEL] + juce::String (i), prog.strumVel[i]);
            setParamValue(valueTreeNames[STRUMHUMAN] + juce::String (i), prog.strumHuman[i]);

        }
    setParamValue(valueTreeNames[GLOBALRESTBAR],  prog.globalResyncBar);
    setParamValue(valueTreeNames[GLOABLINORFIXVEL], prog.GlobalInOrFixedVel);
    setParamValue(valueTreeNames[INBUILTSYNTH],   prog.inBuiltSynth);
    setParamValue(valueTreeNames[SORTEDORFIRST],  prog.sortedOrFirst);
    setParamValue(valueTreeNames[SHUFFLE],        prog.shuffle);
    setParamValue(valueTreeNames[CHANNON],        prog.channelOn);
    setParamValue(valueTreeNames[SCALEKEY],       prog.scaleKey);
    setParamValue(valueTreeNames[SCALETYPE],      prog.scaleType);

    myGridChangeListener.sendChangeMessage();
}

const juce::String TugMidiSeqAudioProcessor::getProgramName (int index)
{
    String s;
    if (index == 0)  return   "Init";
    
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
    juce::AudioPlayHead* playHead = getPlayHead();
    if (playHead == nullptr) return;

    playHead->getCurrentPosition(positionInfo);
    initPrepareValue();
  
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
    // first, so on-screen keys also pass straight through when there's no
    // playhead or the transport is stopped (auditioning)
    drainScreenNotes (midiMessages);
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
    midiMessagesStack.clear();
    midiMessagesStack.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
    playHead->getCurrentPosition(positionInfo);

    // pattern slots: while stopped a lane moves to the requested slot at once
    const int wantedSlot = getRequestedSlot();
    if (! positionInfo.isPlaying)
        for (int i = 0; i < numOfLine; i++)
            if (getActiveSlot (i) != wantedSlot)
                switchLaneSlot (i, wantedSlot);

    // Latch switched off: drop every note that is only being held by the latch.
    const bool latch = *latchAtomic > 0.5f;
    if (prevLatch && ! latch)
        releaseUnheldNotes();
    prevLatch = latch;


    //midiMessages.swapWith(processedMidiBuffer);

    if(positionInfo.isPlaying == true)
    {
        midiMessages.clear();
        //midiMessages.swapWith(erasedMidi);
    }
    
    if (positionInfo.bpm == 0) return;

    delaySampleNumberForQuarter = mySampleRate*0.5/(positionInfo.bpm*1.0/60);
    double ppq =  std::fmod(positionInfo.ppqPosition,4.f);
    if (positionInfo.bpm)
    {
        initPrepareValue();
        
        if (myIsPlaying == false &&  positionInfo.isPlaying == true ) // stop to start
        {
            measureBar = *globalResyncBar-1;  // resync var number
            measureSample = (int)(4*mySampleRate/myBps)-1; // mBps Beat per second  ---> 1 bar  numof sample
            prevppq = ppq;;
           prevtimeInSamples = positionInfo.ppqPosition;; /**ppq ye bakma code*/
        }
        auto  x = positionInfo.ppqPosition; /**ppq ye bakma code*/
        midiEffectSampelDiffBitweenCall = 60*mySampleRate*( x- prevtimeInSamples)/myBpm;/**ppq ye bakma code*/
        prevtimeInSamples = positionInfo.ppqPosition;;/**ppq ye bakma code*/
        if(positionInfo.isPlaying == false)
        {
            initForVariables();
            resetTrigCondState();   // conditions count loops from the next play
            for (auto& r : ratchetLeft) r = 0;   // no leftover repeats on the next play
            for (auto& c : strumCount) c = 0;    // ... or strums
            for (auto& f : strumFlipped) f = false;   // Up/Down starts the way its sign says
            for (auto& m : mutateMask) m = 0;    // every play starts from the written pattern
        }
        if(myIsPlaying == true &&  positionInfo.isPlaying == false )/**ppq ye bakma code*/
        {
            for (auto it = inRealMidiNoteList.begin(); it != inRealMidiNoteList.end(); )
            {
                it->sentMidi.setVelocity(0.0f);
                it->sentMidi.setChannel(it->lineNo);
                midiMessages.addEvent(it->sentMidi, 0);
                //midiProcessor->sendMidiMessage(it->sentMidi,it->lineNo);
                //midiProcessor->sendMidiBuffer(midiMessages,mySampleRate);
                it = inRealMidiNoteList.erase(it);
            }
           
             
            // A latched chord survives a transport stop, and so do keys that are
            // still down (including ones held on the on-screen keyboard), so the
            // next play picks them straight back up.
            if (! latch)
                releaseUnheldNotes();

        }/**ppq ye bakma code*/
            
        myIsPlaying = positionInfo.isPlaying;
        /**ppq ye bakma code*/
        
        if(midiEffectSampelDiffBitweenCall < 0) midiEffectSampelDiffBitweenCall = 0;
        if(myIsPlaying == false)
        {
            midiHandling(midiMessagesStack,0,false);
            if(*channelOnAtamic == true && ! offlineRender)
            {
                midiProcessor->sendMidiBuffer(midiMessagesStack, mySampleRate);
            }
            publishNoteMap();
            return;
        }
        
        auto xx = 4.0/(double)((4*mySampleRate)/myBps);
        for(int s = 0 ; s < buffer.getNumSamples();  s++)/**ppq ye bakma code*/
        {

            measureSample++;
            if( ppq <= prevppq)
            {
                measureSample = 0;
            }
            prevppq = ppq ;
            ppq = std::fmod(ppq + xx,4.f);

            
            //measureSample %= (int)ceil((double)(4*mySampleRate)/myBps);
            if(measureSample == 0)
            {
                measureBar++;
                measureBar %= (int)(*globalResyncBar);
                if(measureBar == 0)
                {
                    initForVariables();
                    
                }
            }
           
               
            for (auto it = inRealMidiNoteList.begin(); it != inRealMidiNoteList.end(); )
            {
                // ">" not "!=": a negative duration would never reach 0, so the note
                // would never get its note-off and would sit in the list forever.
                if(it->durationsample > 0) { it->durationsample--; it++ ; continue;}
                it->sentMidi.setVelocity(0.0f);
                it->sentMidi.setChannel(it->lineNo);
                midiMessages.addEvent(it->sentMidi, s);
                //midiProcessor->sendMidiMessage(it->sentMidi,it->lineNo);
                //midiProcessor->sendMidiBuffer(midiMessages,mySampleRate);
                it = inRealMidiNoteList.erase(it);
            }
      
            midiHandling(midiMessagesStack,s);
            for(int i =  0; i  < numOfLine ; i++)
            {

                calculateAndUpdateSetup(i);

                // the slot index going backwards means the lane's pattern wrapped
                const bool newSlot = steps[i] != lastStep[i];
                if (steps[i] < lastStep[i])
                {
                    ++loopCount[i];
                    if (loopCount[i] > 0)   // a loop was completed (0 is the first one starting)
                        mutateLane (i);
                }
                lastStep[i] = steps[i];
                if (mutateResetRequest[i].exchange (false))
                    mutateMask[i] = 0;
                // a slot change waits for the lane's loop to start again (ping-pong: after the way back)
                if (stpSample[i] == 0 && steps[i] == 0 && getActiveSlot (i) != wantedSlot
                    && ((int) *gridsDirectionAtomic[i] != DirPingPong || jmax (0, loopCount[i]) % 2 == 0))
                {
                    switchLaneSlot (i, wantedSlot);
                    initPrepareValue();   // note lengths look ahead to the next step that's on
                }
                if (newSlot)
                    playStep[i] = directedStep (i, steps[i]);   // after the loop count: ping-pong reads it

                if(stepmidStopSampleCounter[i] != -1)
                {
                    stepmidStopSampleCounter[i]++;
                    int stopInterval = stepmidStopSampleIntervalForShuffle[i][steps[i]];
                    if (stopInterval != 0)                       // guard: was a divide-by-zero crash point
                        stepmidStopSampleCounter[i] %= stopInterval;
                }
                if( stepmidStopSampleCounter[i]== 0)
                {

                    stepmidStopSampleCounter[i] = -1;
                    midiState[i] = false;
                }
                // muted while a strum or ratchet was still going: drop the rest
                if (isLaneMuted (i)) { strumCount[i] = 0; ratchetLeft[i] = 0; }

                // ratchet repeats of the step that fired last
                tickStrum (i, midiMessages, s);
                if (ratchetLeft[i] > 0 && --ratchetCountdown[i] <= 0)
                {
                    if (getPlayMode (i) == PlayVoice)
                        emitLaneNote (i, ratchetNote[i], ratchetDuration[i], midiMessages, s);
                    else
                        playChord (i, ratchetDuration[i], ratchetInterval[i], midiMessages, s);   // the whole chord / strum again
                    if (--ratchetLeft[i] > 0)
                        ratchetCountdown[i] = ratchetInterval[i];
                }

                if(stpSample[i] == 0)
                {
                    const int st   = playStep[i];
                    int cell = cellAt (i, st);   // 0 off, 1 on, 2 event
                    if ((mutateMask[i] >> st) & 1u)
                        cell = cell == 0 ? 1 : 0;              // mutated: an off step plays, a playing one rests
                    if (cell != 0)
                    {
                        const int cond = getStepCond (i, st);
                        bool fire = evaluateTrigCond (i, st);
                        if (fire && cell == 2)
                            fire = juce::Random::getSystemRandom().nextInt(100) < *gridsEventAtomic[i];

                        // Conditional and probabilistic steps record their outcome for
                        // PRE / NEI. PRE itself only reads it, so a run of PRE steps all
                        // follow the condition that led them.
                        if ((cond != CondNone || cell == 2) && cond != CondPre && cond != CondNotPre)
                            lastCondResult[i] = fire;

                        if (fire)
                            subComputrFunc( i,midiMessages, s);
                    }
                }
                baseSampleNumber[i]++;

                if (stepLoopResetInterval[i] != 0)              // guard against modulo-by-zero
                    baseSampleNumber[i] %= stepLoopResetInterval[i];
 
                int idx = (-*gridsDelayAtomic[i]*0.01*delaySampleNumberForQuarter  + baseSampleNumber[i] );
  
                idx = circularRange(idx, 0, stepLoopResetInterval[i]);
                sampleNumber[i] = idx  ;
                //DBG("baseSampleNumber " + tmps + " idx = " + tmps2idx + " after idx = " + tmps3idx);
                
            }
        }
        
    }

    mySynth.renderNextBlock(buffer, midiMessages, 0 , buffer.getNumSamples());
    if(*inBuiltSynthAtomic == false)
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    if(*channelOnAtamic == true && ! offlineRender)
    {

        midiProcessor->sendMidiBuffer(midiMessages, mySampleRate);
    }
    publishNoteMap();
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
    state.setProperty ("currentProgram", program, nullptr);
    writeStepDataTo (state);
    writeSlotsTo (state);
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
        {
            auto restored = ValueTree::fromXml (*xmlState);

            // pattern slots: each lane's active slot first, so the step data and
            // pad parameters restored below land in it (older projects: slot A)
            resetSlots();
            const auto slotsNode = restored.getChildWithName ("patternSlots");
            const auto actives = juce::StringArray::fromTokens (slotsNode.getProperty ("active").toString(), ",", "");
            for (int i = 0; i < numOfLine; i++)
                activeSlot[i].store (i < actives.size() ? jlimit (0, numSlots - 1, actives[i].getIntValue()) : 0);

            readStepDataFrom (restored);
            valueTreeState.replaceState (restored);

            // Force every parameter to reflect the restored value-tree value.
            // replaceState only re-syncs a parameter when its (quantized) tree
            // value changes, so bool / coarse parameters can otherwise keep a
            // stale raw value if the restored value matches the pre-restore one
            // (flagged by pluginval as "not restored").
            for (auto* param : getParameters())
            {
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
                {
                    const auto child = valueTreeState.state.getChildWithProperty ("id", ranged->paramID);
                    if (child.isValid())
                        ranged->setValueNotifyingHost (ranged->convertTo0to1 ((float) child.getProperty ("value")));
                }
            }

            if (! readSlotsFrom (restored))   // a project from before the slots plays slot A
                if (auto* slotParam = valueTreeState.getParameter (valueTreeNames[PATTERNSLOT]))
                    slotParam->setValueNotifyingHost (0.0f);
            for (auto& d : mirrorDirty)       // the pads show what the slots hold
                d.store (true);
        }

    program = valueTreeState.state.getProperty ("currentProgram", program);

    midiPortName = getMidiPortNameFromXml();
    if (! offlineRender)   // the export copy must not open the user's MIDI port
        midiProcessor->setMidiPort(midiPortName);
}

void TugMidiSeqAudioProcessor::initPrepareValue()
{
    

    
    if (positionInfo.bpm)
    {
         myBpm =  positionInfo.bpm;
         myBps = (float)myBpm/60;
   
        //  std::cout << mBps << " " ;
        
        ppq = positionInfo .ppqPosition;
        
        
        
        // in  t x = positionInfo.ppqPosition;
        
       /*
        stepResetInterval[0] =  1*(60*mySampleRate/ myBpm);
        stepResetInterval[1] =  1*(60*mySampleRate/ myBpm);
        stepResetInterval[2] =  1*(60*mySampleRate/ myBpm);
        stepResetInterval[3] =  1*(60*mySampleRate/ myBpm);
        stepResetInterval[4] =  1*(60*mySampleRate/ myBpm);
       */
        
        for(int i = 0 ; i< numOfLine ;i++)
        {
            double first = 1.5*4*(60*mySampleRate/ myBpm); // first value in combos  number of sample musical durations and speeds
            
            int index = *gridsSpeedAtomic[i] ;
            
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            
            double tmp = (first / pow(2,index));
            stepResetInterval[i] = jmax (1, (int) (tmp + 1)); // dviding  "first" you get number of sample  for musical note time values
            remaining[i] =  tmp + 1 - stepResetInterval[i];
            stepLoopResetInterval[i] = jmax (1, (int) (tmp**numOfGrid[i] + 1));
            float shuffleTmp = (*gridsShuffleAtomic[i] + *shuffleAtomic)/100;
            shuffleTmp = juce::jlimit(-0.99f, 0.99f, shuffleTmp);
            long tmpTotal = stepLoopResetInterval[i];
            for(int s = 0 ; s < *numOfGrid[i] ; s++)
            {
                if(s%2 == 0 )
                {
                    stepResetIntervalForShuffle[i][s] = stepResetInterval[i]*(1 + shuffleTmp);
                    tmpTotal -= stepResetIntervalForShuffle[i][s];
                    if(tmpTotal <0 )
                        stepResetIntervalForShuffle[i][s]  =  stepResetIntervalForShuffle[i][s]  + tmpTotal  ;
                }
                else
                {
                    stepResetIntervalForShuffle[i][s] = stepResetInterval[i]*(1 - shuffleTmp );
                    tmpTotal -= stepResetIntervalForShuffle[i][s];
                    if(tmpTotal <0 )
                        stepResetIntervalForShuffle[i][s]  =  stepResetIntervalForShuffle[i][s]  + tmpTotal  ;
                }
            }
            
            if(tmpTotal > 0)
            {
                int tmp = *numOfGrid[i];
                tmp = tmp/2;
                int addTmp = tmpTotal / tmp;
                for(int s = 1 ; s < *numOfGrid[i] ; s = s + 2)
                {
                    stepResetIntervalForShuffle[i][s] =  stepResetIntervalForShuffle[i][s]+ addTmp;
                }

            }

            // Every step must consume at least one sample. Extreme shuffle values and
            // the tmpTotal corrections above can drive an interval to 0 or below, which
            // makes calculateAndUpdateSetup() never advance the step: the lane then
            // re-fires on every single sample and inRealMidiNoteList grows without
            // bound until processBlock effectively stops making progress.
            for(int s = 0 ; s < *numOfGrid[i] ; s++)
                stepResetIntervalForShuffle[i][s] = jmax (1, stepResetIntervalForShuffle[i][s]);

            //for(int s = 0 ; s < *numOfGrid[i] ; s++)
            //    forGuiStepResetIntervalForShuffle[i][s] = stepResetIntervalForShuffle[i][s] ;
            
            first = 1.5*4*(60*mySampleRate/ myBpm);
            index = *gridsDurationAtomic[i];
            
            if(index%3 == 0) {  index = (index+1) / 3;}
            else if((index -1)%3 == 0) { index = (index) / 3;first = 2*first/3;}
            else if((index -2)%3 == 0) { index = (index-1) / 3;first = 4*first/9;}
            
            stepmidStopSampleInterval[i] = first / pow(2,index);
            /*
            tmpTotal = stepLoopResetInterval[i];
            for(int s = 0 ; s < *numOfGrid[i] ; s++)
            {
                if(s%2 == 0 )
                {
                    stepmidStopSampleIntervalForShuffle[i][s] = stepmidStopSampleInterval[i]*(1 + shuffleTmp);
                    tmpTotal -= stepmidStopSampleIntervalForShuffle[i][s];
                    if(tmpTotal <0 )
                        stepmidStopSampleIntervalForShuffle[i][s]  =  stepmidStopSampleIntervalForShuffle[i][s]  + tmpTotal  ;
                }
                else
                {
                    stepmidStopSampleIntervalForShuffle[i][s] = stepmidStopSampleInterval[i]*(1 - shuffleTmp);
                    tmpTotal -= stepmidStopSampleIntervalForShuffle[i][s];
                    if(tmpTotal <0 )
                        stepmidStopSampleIntervalForShuffle[i][s]  =  stepmidStopSampleIntervalForShuffle[i][s]  + tmpTotal  ;
                }
            }
             */
            int prevmidistep = -1;
            float  shuffCoef[2];
            shuffCoef[0] = 1 + shuffleTmp;
            shuffCoef[1] = 1 - shuffleTmp;
            
            int sum = 0;
            for(int s = 0 ; s < *numOfGrid[i] ; s++)
            {
   
                stepmidStopSampleIntervalForShuffle[i][s] = stepmidStopSampleInterval[i]*shuffCoef[s%2];
                sum =  sum + stepResetIntervalForShuffle[i][s];
                if(prevmidistep != -1)
                {
                    if( cellActiveAtSlot (i, s))
                    {
                        
                        if(stepmidStopSampleIntervalForShuffle[i][prevmidistep] > sum)
                            stepmidStopSampleIntervalForShuffle[i][prevmidistep] = sum - stepResetIntervalForShuffle[i][prevmidistep];
                        sum = stepResetIntervalForShuffle[i][s];
                        prevmidistep = s;
                        continue;
                    }
                }
                
                if(cellActiveAtSlot (i, s))
                {
                    prevmidistep = s;
                    sum = stepResetIntervalForShuffle[i][s];
                   
                }
                    
           
                
            }
            
        }
    }
    
    
}

bool TugMidiSeqAudioProcessor::subComputrFunc(int i,juce::MidiBuffer& midiMessages,int s)
{
    if(soloLane != -1 && soloLane != i ) return true;
    if (isLaneMuted (i)) return true;   // the step still counted for PRE / NEI, it just isn't played

    if (getPlayMode (i) != PlayVoice)
    {
        if (! myIsPlaying || inMidiNoteList.empty()) return true;
        int duration = jmax (0, stepmidStopSampleIntervalForShuffle[i][steps[i]] - 1);
        int window = samplesToNextHit (i);
        const int hits = getStepRatchet (i, playStep[i]);
        ratchetLeft[i] = 0;
        if (hits > 1)   // as below, but each repeat replays the chord
        {
            const int interval = jmax (1, stepResetIntervalForShuffle[i][steps[i]] / hits);
            duration = jmax (1, jmin (duration, interval * 3 / 4));
            window = interval;
            ratchetLeft[i]      = hits - 1;
            ratchetInterval[i]  = interval;
            ratchetCountdown[i] = interval;
            ratchetDuration[i]  = duration;
        }
        playChord (i, duration, window, midiMessages, s);
        return true;
    }

    int midRouteIndex = *gridsMidiRouteAtomic[i];
    bool  sortedofirs_Bool = inMidiNoteList.size() > i;
    if(*sortedOrFirstEmptySelectAtomic == true)
        sortedofirs_Bool = true;
    
    if(sortedofirs_Bool)
    {
        MidiMessage it;
        if(*sortedOrFirstEmptySelectAtomic == false)
        {
            it = inMidiNoteList[(size_t)i];
        }
        else
        {
            auto it1 = &inMidiNoteListVector.at(i);
            it = *it1;
            if(it1->getVelocity() == 0)
                return true;
        }
        
        
        it.setNoteNumber (pitchedNote (it.getNoteNumber() + (int) *octave[i] * 12,
                                       getStepPitch (i, playStep[i])));
        
        auto midiNote = [&](const RealMidiNoteList& l){ return l.sentMidi.getNoteNumber() == it.getNoteNumber() && l.lineNo == midRouteIndex; };
        auto it2 = std::find_if(inRealMidiNoteList.begin(), inRealMidiNoteList.end(),midiNote);
        if(it2 != inRealMidiNoteList.end())
        {
            it2->sentMidi.setVelocity(0.0f);
            it2->sentMidi.setChannel(it2->lineNo);
            midiMessages.addEvent(it2->sentMidi, s);
            //midiProcessor->sendMidiMessage(it2->sentMidi,it2->lineNo);
           // midiProcessor->sendMidiBuffer(midiMessages,mySampleRate);
            it2 = inRealMidiNoteList.erase(it2);
        }
        
        float velTmp = *gridsVelAtomic[i]/ 90.0f;
        velTmp = jlimit(0.0f,1.0f,velTmp*(float) stepVelAt (i, playStep[i])/ 127.0f);
        if(*GlobalInOrFixedAtomic == 0)
           it.setVelocity(velTmp);
    
        
        
        if (myIsPlaying == false) return false;
        it.setChannel(midRouteIndex);

        // Shuffle/duration maths can drive this interval to 0 or below, which would
        // otherwise produce a never-expiring note.
        int duration = jmax (0, stepmidStopSampleIntervalForShuffle[i][steps[i]] - 1);

        // Ratchet: the remaining hits are spread evenly over this slot and played
        // by the per-sample countdown in processBlock. Each hit is gated to at
        // most 3/4 of its share so repeats stay audibly separate.
        const int hits = getStepRatchet (i, playStep[i]);
        ratchetLeft[i] = 0;
        if (hits > 1)
        {
            const int interval = jmax (1, stepResetIntervalForShuffle[i][steps[i]] / hits);
            duration = jmax (1, jmin (duration, interval * 3 / 4));
            ratchetLeft[i]      = hits - 1;
            ratchetInterval[i]  = interval;
            ratchetCountdown[i] = interval;
            ratchetDuration[i]  = duration;
            ratchetNote[i]      = it;
        }
        emitLaneNote (i, it, duration, midiMessages, s);
    }
    return true;
}

// Samples from the start of the current slot to the next slot whose step is
// on, i.e. the most time this hit has before the lane plays again.
int TugMidiSeqAudioProcessor::samplesToNextHit (int line) const
{
    const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
    const int slot = jlimit (0, n - 1, steps[line]);
    int gap = stepResetIntervalForShuffle[line][slot];
    for (int j = 1; j < n; j++)
    {
        const int next = (slot + j) % n;
        if (cellActiveAtSlot (line, next)) break;
        gap += stepResetIntervalForShuffle[line][next];
    }
    return jmax (1, gap);
}

// Chord / strum: every held note (low to high, max maxStrumNotes), each through
// the lane's octave, the step's pitch and the scale lock. A strum puts Spread ms
// between notes and lets them all end together, like strings that were hit one
// after another. `window` is the time until the lane's next hit: a strum too
// long for it is squeezed so every note is played and the last one still rings
// a quarter of the note length, and the chord may ring past the note length
// (never past the window) to give it that.
void TugMidiSeqAudioProcessor::playChord (int line, int duration, int window, juce::MidiBuffer& midiMessages, int sample)
{
    const int mode = getPlayMode (line);
    int notes[maxStrumNotes];
    float velocities[maxStrumNotes];
    int count = 0;

    float laneVel = jlimit (0.0f, 1.0f, (*gridsVelAtomic[line] / 90.0f) * ((float) stepVelAt (line, playStep[line]) / 127.0f));
    for (const auto& held : inMidiNoteList)
    {
        if (count == maxStrumNotes) break;
        const int n = pitchedNote (held.getNoteNumber() + (int) *octave[line] * 12, getStepPitch (line, playStep[line]));
        bool dup = false;   // the scale lock can fold two keys onto one note
        for (int k = 0; k < count; k++) dup |= notes[k] == n;
        if (dup) continue;
        notes[count] = n;
        velocities[count] = *GlobalInOrFixedAtomic == 0 ? laneVel : held.getFloatVelocity();
        count++;
    }
    if (count == 0) return;

    const int spread = getSpread (line);
    bool downward = spread < 0;
    if (mode == PlayStrumUpDown)
    {
        downward = downward != strumFlipped[line];
        strumFlipped[line] = ! strumFlipped[line];
    }
    const int spreadSamples = (int) (std::abs (spread) * mySampleRate / 1000.0);
    const int channel = (int) *gridsMidiRouteAtomic[line];

    // the strum's length is |spread| per gap, squeezed to fit the window; its
    // shape places the notes inside it
    const bool shaped   = spreadSamples > 0 && count > 1;
    window              = jmax (1, window - 1);   // a note-off lands a sample after its length, like the steps' notes
    const int  ring     = jlimit (1, window, duration / 4);
    const int  total    = jmin (spreadSamples * (count - 1), jmax (0, window - ring));
    const int  end      = jmin (window, jmax (duration, total + ring));   // all notes stop here
    const float gap     = count > 1 ? (float) total / (float) (count - 1) : 0.0f;
    const int  shape    = getStrumShape (line);
    const float tension = getStrumTension (line), velTilt = getStrumVelTilt (line);
    const float human   = shaped ? getStrumHumanize (line) : 0.0f;
    auto& rng = juce::Random::getSystemRandom();

    strumCount[line] = 0;
    int previous = 0;
    for (int k = 0; k < count; k++)
    {
        const int idx = downward ? count - 1 - k : k;
        float position = 0.0f, velFactor = 1.0f;
        strumNotePlacement (k, count, shaped ? shape : StrumLinear, tension, shaped ? velTilt : 0.0f, position, velFactor);

        int delay = (int) std::lround (position * (float) total);
        if (k > 0 && human > 0.0f)   // up to half a gap early or late, never before the previous note
            delay = jlimit (previous, total, delay + (int) ((rng.nextFloat() - 0.5f) * human * gap));
        previous = delay;

        float velocity = velocities[idx] * velFactor;
        if (human > 0.0f)
            velocity *= 1.0f + (rng.nextFloat() - 0.5f) * 0.4f * human;   // up to +-20 %
        auto note = juce::MidiMessage::noteOn (channel, notes[idx], jlimit (0.02f, 1.0f, velocity));

        const int length = jmax (1, end - delay);
        if (delay == 0)
            emitLaneNote (line, note, length, midiMessages, sample);
        else
            strumQueue[line][strumCount[line]++] = { delay, length, note };
    }
}

void TugMidiSeqAudioProcessor::tickStrum (int line, juce::MidiBuffer& midiMessages, int sample)
{
    for (int k = 0; k < strumCount[line];)
    {
        auto& q = strumQueue[line][k];
        if (--q.countdown > 0) { k++; continue; }
        emitLaneNote (line, q.note, q.duration, midiMessages, sample);
        q = strumQueue[line][--strumCount[line]];   // swap-remove; order no longer matters
    }
}

// Sends one note for lane `line` (cutting the same note still sounding on the
// same channel; other lanes' channels are left alone) and books its note-off.
void TugMidiSeqAudioProcessor::emitLaneNote (int line, juce::MidiMessage note, int durationSamples,
                                             juce::MidiBuffer& midiMessages, int sample)
{
    auto same = [&] (const RealMidiNoteList& l) { return l.sentMidi.getNoteNumber() == note.getNoteNumber()
                                                         && l.lineNo == note.getChannel(); };
    auto sounding = std::find_if (inRealMidiNoteList.begin(), inRealMidiNoteList.end(), same);
    if (sounding != inRealMidiNoteList.end())
    {
        sounding->sentMidi.setVelocity (0.0f);
        sounding->sentMidi.setChannel (sounding->lineNo);
        midiMessages.addEvent (sounding->sentMidi, sample);
        inRealMidiNoteList.erase (sounding);
    }

    midiMessages.addEvent (note, sample);
    lastOutNote[line] = note.getNoteNumber();
    stepmidStopSampleCounter[line] = 1;

    RealMidiNoteList tmp;
    tmp.sentMidi = note;
    tmp.lineNo = note.getChannel();
    tmp.durationsample = jmax (0, durationSamples);
    inRealMidiNoteList.push_back (tmp);
    midiState[line] = true;
}

void TugMidiSeqAudioProcessor::calculateAndUpdateSetup(int myLine)
{
    
 
    {
        int resetInterval = stepResetIntervalForShuffle[myLine][new_steps[myLine]];
        if (resetInterval != 0)                                 // guard against modulo-by-zero
            stpSample[myLine] %= resetInterval;
    }
    int left = 0;
    if(sampleNumber[myLine] == 0)
    {
        
    }
          
    for(int i = 0; i < *numOfGrid[myLine] ; i++ )
    {
        stpSample[myLine] =   sampleNumber[myLine] - left;
        if(left == sampleNumber[myLine])
        {
            //auto is= std::to_string(i);
            //DBG("fire " + is);
            steps[myLine] = i;
            stpSample[myLine] = 0;
            return;
        }
        if( (left < sampleNumber[myLine])  && ((left +  stepResetIntervalForShuffle[myLine][i]) > sampleNumber[myLine]))
        {
            //auto is= std::to_string(i);
           // DBG("fire " + is);
            steps[myLine] = i;
            
            return;
        }
           
        left = left + stepResetIntervalForShuffle[myLine][i];
    }
   
}
void TugMidiSeqAudioProcessor::midiHandling(juce::MidiBuffer& midiMessages, int sampleOffset, bool sampleBased )
{
    MidiBuffer::Iterator it(midiMessages);
    
    MidiMessage currentMessage;

    int samplePos;
    //myInnmidiBuffer.clear();
    while(it.getNextEvent(currentMessage,samplePos))
    {
        bool loopFound = false;

        for (int i = 0; i < numOfLine; ++i) {
            if (currentMessage.getChannel() == *gridsMidiRouteAtomic[i]) {
                loopFound =  true;
                break;
            }
        }
        if(loopFound == true) continue;
         
        
        if(sampleBased == false) { samplePos = 0 ; sampleOffset = 0;}
        // Note-offs are handled at their own sample position too, like note-ons.
        // (They used to be handled all at sample 0, so a note pressed and released
        // within one block was released before it was pressed and got stuck.)
        if(samplePos != sampleOffset) continue;

        const int noteNumber = currentMessage.getNoteNumber();
        auto sameNote = [noteNumber](const MidiMessage& l){ return l.getNoteNumber() == noteNumber; };

        if(currentMessage.isNoteOn())
        {
            // Latch: the first key of a new chord replaces the latched one;
            // keys pressed while others are still down add to it.
            if (*latchAtomic > 0.5f && physHeldCount == 0)
                clearHeldNotes();
            if (! physHeld[noteNumber]) { physHeld[noteNumber] = true; physHeldCount++; }

            // a latched note pressed again is already in the lists
            if (std::find_if(inMidiNoteList.begin(), inMidiNoteList.end(), sameNote) != inMidiNoteList.end())
                continue;

            inMidiNoteList.push_back(currentMessage);
            auto comp = [](const MidiMessage &l1, const MidiMessage &l2){ return l1.getNoteNumber() < l2.getNoteNumber(); };
            std::sort(inMidiNoteList.begin(), inMidiNoteList.end(), comp);

            for (auto i = 0 ; i < inMidiNoteListVector.size() ;i++)
            {
                if(inMidiNoteListVector.at(i).getVelocity() != 0) continue;

                inMidiNoteListVector.at(i) = currentMessage;

                break;;
            }
        }
        else if(currentMessage.isNoteOff())
        {
            if (physHeld[noteNumber]) { physHeld[noteNumber] = false; physHeldCount--; }
            if (*latchAtomic > 0.5f) continue;   // latched: keep playing it

            auto it = std::find_if(inMidiNoteList.begin(), inMidiNoteList.end(), sameNote);
            if(it == inMidiNoteList.end()) continue;   // was 'return' — skipped the rest of the buffer
            inMidiNoteList.erase(it);

            for (auto i = 0 ; i < inMidiNoteListVector.size() ;i++)
            {
                if(inMidiNoteListVector.at(i).getNoteNumber() != noteNumber) continue;
                if(inMidiNoteListVector.at(i).getVelocity() == 0) continue;   // stale, already-freed slot

                inMidiNoteListVector.at(i).setVelocity(0.0f);
                break;;
            }
        }
  
    }
    /*
if(myIsPlaying == false)
{
    MidiBuffer temMidiBuffer;
    if(*sortedOrFirstEmptySelectAtomic == 1)
    {
        for (auto i = 0 ; i < numOfLine;i++)
        {
            inMidiNoteListVector.at(i).setChannel(*gridsMidiRouteAtomic[i]);
            temMidiBuffer.addEvent(inMidiNoteListVector.at(i), 0);
        }
    }
    else{

        for (auto i = 0 ; i < numOfLine;i++)
        {
            auto it = std::next(inMidiNoteList.begin(), i);
            if(it == inMidiNoteList.end()) break;
            it->setChannel(*gridsMidiRouteAtomic[i]);
            temMidiBuffer.addEvent(*it, 0);
                
        }
   
    }
    midiMessages.swapWith(temMidiBuffer);
    midiMessages.addEvents(erasedMidi, 0,0, 0);
    
}
    */

    
    
}

void TugMidiSeqAudioProcessor::initForVariables()
{
    
    for(int i =  0; i  < numOfLine ; i++)
    {
        steps[i] = (int)(*numOfGrid[i]) -1;
        baseSampleNumber[i] = 0;
        //int idx = (-*gridsDelayAtomic[i]*0.01f*stepResetIntervalForShuffle[i][steps[i]] + baseSampleNumber[i]);
        int idx = (-*gridsDelayAtomic[i]*0.01*delaySampleNumberForQuarter  + baseSampleNumber[i] );
        idx = circularRange(idx, 0, stepLoopResetInterval[i]);
        sampleNumber[i] = idx;
       
/*
        gauge[i] = gauge[i] + *numOfGrid[i]*remaining[i];
        int c = 0;
        if(gauge[i] >= 1)
            {
                c = gauge[i] ;
                gauge[i] = gauge[i]  - c;
                
               
            }
      
         */
        
        stpSample[i] = stepResetIntervalForShuffle[i][steps[i]] ;
    }
}

//==============================================================================
// Latch

void TugMidiSeqAudioProcessor::clearHeldNotes()
{
    inMidiNoteList.clear();
    for (auto& slot : inMidiNoteListVector)
        slot.setVelocity (0.0f);
}

void TugMidiSeqAudioProcessor::releaseUnheldNotes()
{
    inMidiNoteList.erase (std::remove_if (inMidiNoteList.begin(), inMidiNoteList.end(),
                                          [this] (const MidiMessage& m) { return ! physHeld[m.getNoteNumber()]; }),
                          inMidiNoteList.end());
    for (auto& slot : inMidiNoteListVector)
        if (slot.getVelocity() != 0 && ! physHeld[slot.getNoteNumber()])
            slot.setVelocity (0.0f);
}

//==============================================================================
// Undoable step conditions and lane edits

namespace
{
    // A per-step value that isn't a parameter (condition or ratchet).
    struct StepValueAction : juce::UndoableAction
    {
        enum Field { Cond, Ratchet, Pitch };
        StepValueAction (TugMidiSeqAudioProcessor& p, Field f, int l, int s, int from, int to)
            : proc (p), field (f), line (l), step (s), oldValue (from), newValue (to) {}
        // the host is told once per edit / undo, not per step (see notifyStateChanged)
        bool perform() override { set (newValue); return true; }
        bool undo() override    { set (oldValue); return true; }
        int getSizeInUnits() override { return 1; }

        void set (int v)
        {
            if      (field == Cond)    proc.setStepCond (line, step, v);
            else if (field == Ratchet) proc.setStepRatchet (line, step, v);
            else                       proc.setStepPitch (line, step, v);
        }

        TugMidiSeqAudioProcessor& proc;
        Field field;
        int line, step, oldValue, newValue;
    };

    // One lane's pattern and settings. File-static so a lane copied in one
    // instance of the plugin can be pasted into another in the same host.
    struct LaneClipboard
    {
        bool  valid = false;
        float cells[numOfStep] = {}, vels[numOfStep] = {};
        int   conds[numOfStep] = {}, ratchets[numOfStep] = {}, pitches[numOfStep] = {};
        std::map<juce::String, float> settings;   // base name -> value
    };
    LaneClipboard laneClipboard;

    // per-lane settings a copy carries (not the MIDI channel: that's routing)
    const int copiedSettings[] = { GRIDNUM, SPEEED, DUR, OCTAVE, VEL, EVENT, GRIDSHUFFLE, GRIDDELAY, DIRECTION, MUTATE, PLAYMODE, SPREAD,
                                   STRUMSHAPE, STRUMTENSION, STRUMVEL, STRUMHUMAN };

    juce::String cellID (int base, int line, int step)
    {
        juce::String s;
        s << valueTreeNames[base] << line << step;
        return s;
    }
}

void TugMidiSeqAudioProcessor::setStepCondUndoable (int line, int step, int cond)
{
    const int old = getStepCond (line, step);
    if (old != cond)
        undoManager.perform (new StepValueAction (*this, StepValueAction::Cond, line, step, old, cond));
}

void TugMidiSeqAudioProcessor::setStepRatchetUndoable (int line, int step, int hits)
{
    const int old = getStepRatchet (line, step);
    hits = jlimit (1, maxRatchet, hits);
    if (old != hits)
        undoManager.perform (new StepValueAction (*this, StepValueAction::Ratchet, line, step, old, hits));
}

void TugMidiSeqAudioProcessor::setStepPitchUndoable (int line, int step, int offset)
{
    const int old = getStepPitch (line, step);
    offset = jlimit (-maxStepPitch, maxStepPitch, offset);
    if (old != offset)
        undoManager.perform (new StepValueAction (*this, StepValueAction::Pitch, line, step, old, offset));
}

// Step conditions aren't parameters, so the host doesn't see them change on
// its own: without this it wouldn't mark the project as modified.
void TugMidiSeqAudioProcessor::notifyStateChanged()
{
    updateHostDisplay (ChangeDetails{}.withNonParameterStateChanged (true));
}

static float paramValue (juce::AudioProcessorValueTreeState& s, const juce::String& id)
{
    auto* p = s.getParameter (id);
    return p != nullptr ? p->convertFrom0to1 (p->getValue()) : 0.0f;
}

void TugMidiSeqAudioProcessor::copyLane (int line)
{
    for (int s = 0; s < numOfStep; s++)
    {
        laneClipboard.cells[s] = paramValue (valueTreeState, cellID (BLOCK, line, s));
        laneClipboard.vels[s]  = paramValue (valueTreeState, cellID (VELGRIDBUTTON, line, s));
        laneClipboard.conds[s] = getStepCond (line, s);
        laneClipboard.ratchets[s] = getStepRatchet (line, s);
        laneClipboard.pitches[s] = getStepPitch (line, s);
    }
    for (int base : copiedSettings)
        laneClipboard.settings[valueTreeNames[base]] = paramValue (valueTreeState, valueTreeNames[base] + juce::String (line));
    laneClipboard.valid = true;
}

bool TugMidiSeqAudioProcessor::hasLaneClipboard() const { return laneClipboard.valid; }

void TugMidiSeqAudioProcessor::pasteLane (int line)
{
    if (! laneClipboard.valid) return;
    undoableEdit ([&]
    {
        for (auto& [base, value] : laneClipboard.settings)
            setParamValue (base + juce::String (line), value);
        for (int s = 0; s < numOfStep; s++)
        {
            setParamValue (cellID (BLOCK, line, s), laneClipboard.cells[s]);
            setParamValue (cellID (VELGRIDBUTTON, line, s), laneClipboard.vels[s]);
            setStepCondUndoable (line, s, laneClipboard.conds[s]);
            setStepRatchetUndoable (line, s, laneClipboard.ratchets[s]);
            setStepPitchUndoable (line, s, laneClipboard.pitches[s]);
        }
    });
    notifyStateChanged();
    myGridChangeListener.sendChangeMessage();
}

// Rotates a lane's steps within its length; the caller owns the undo step.
void TugMidiSeqAudioProcessor::rotateLaneSteps (int line, int delta)
{
    const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
    float cells[numOfStep], vels[numOfStep];
    int conds[numOfStep], ratchets[numOfStep], pitches[numOfStep];
    for (int s = 0; s < n; s++)
    {
        cells[s] = paramValue (valueTreeState, cellID (BLOCK, line, s));
        vels[s]  = paramValue (valueTreeState, cellID (VELGRIDBUTTON, line, s));
        conds[s] = getStepCond (line, s);
        ratchets[s] = getStepRatchet (line, s);
        pitches[s] = getStepPitch (line, s);
    }
    for (int s = 0; s < n; s++)
    {
        const int from = ((s - delta) % n + n) % n;   // delta +1 moves everything one step right
        setParamValue (cellID (BLOCK, line, s), cells[from]);
        setParamValue (cellID (VELGRIDBUTTON, line, s), vels[from]);
        setStepCondUndoable (line, s, conds[from]);
        setStepRatchetUndoable (line, s, ratchets[from]);
        setStepPitchUndoable (line, s, pitches[from]);
    }
}

void TugMidiSeqAudioProcessor::shiftLane (int line, int delta)
{
    undoableEdit ([&] { rotateLaneSteps (line, delta); });
    notifyStateChanged();
}

void TugMidiSeqAudioProcessor::shiftAllLanes (int delta)
{
    undoableEdit ([&] { for (int i = 0; i < numOfLine; i++) rotateLaneSteps (i, delta); });
    notifyStateChanged();
}

// Even spread of `hits` over the lane's length (the Bresenham form of the
// Euclidean rhythm), turned by `rotation`. Steps beyond the length are left
// alone. The caller owns the undo step, so a knob drag is one undo.
void TugMidiSeqAudioProcessor::euclidLane (int line, int hits, int rotation)
{
    const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
    hits = jlimit (0, n, hits);
    for (int s = 0; s < n; s++)
    {
        const int pos = ((s - rotation) % n + n) % n;
        const bool on = (pos * hits) % n < hits;
        setParamValue (cellID (BLOCK, line, s), on ? 1.0f : 0.0f);
    }
}

void TugMidiSeqAudioProcessor::clearLane (int line)
{
    undoableEdit ([&]
    {
        for (int s = 0; s < numOfStep; s++)
        {
            setParamValue (cellID (BLOCK, line, s), 0.0f);
            setStepCondUndoable (line, s, CondNone);
            setStepRatchetUndoable (line, s, 1);
            setStepPitchUndoable (line, s, 0);
        }
    });
    notifyStateChanged();
}

void TugMidiSeqAudioProcessor::setLaneDirection (int line, int dir)
{
    undoableEdit ([&] { setParamValue (valueTreeNames[DIRECTION] + juce::String (line), (float) dir); });
}

// The lane's next (or previous) direction; with allLanes every lane takes it, as one undo step.
void TugMidiSeqAudioProcessor::cycleLaneDirection (int line, int delta, bool allLanes)
{
    const int n = directionNames.size();
    const int dir = ((getDirection (line) + delta) % n + n) % n;
    undoableEdit ([&]
    {
        for (int i = 0; i < numOfLine; i++)
            if (allLanes || i == line)
                setParamValue (valueTreeNames[DIRECTION] + juce::String (i), (float) dir);
    });
}

void TugMidiSeqAudioProcessor::setLaneMutate (int line, int percent)
{
    undoableEdit ([&] { setParamValue (valueTreeNames[MUTATE] + juce::String (line), (float) percent); });
}

void TugMidiSeqAudioProcessor::setLaneMute (int line, bool muted)
{
    undoableEdit ([&] { setParamValue (valueTreeNames[MUTE] + juce::String (line), muted ? 1.0f : 0.0f); });
}

void TugMidiSeqAudioProcessor::setLanePlayMode (int line, int mode)
{
    undoableEdit ([&] { setParamValue (valueTreeNames[PLAYMODE] + juce::String (line), (float) mode); });
}

// Audio thread, once per completed loop of the lane.
void TugMidiSeqAudioProcessor::mutateLane (int line)
{
    const int chance = getMutate (line);
    if (chance <= 0) return;
    const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
    auto& rng = juce::Random::getSystemRandom();
    for (int s = 0; s < n; s++)
        if (rng.nextInt (100) < chance)
            mutateMask[line] ^= 1u << s;
}

//==============================================================================
// MIDI export

namespace
{
    // A transport that is always at `pos` samples into a 120 BPM 4/4 song.
    // The engine's timing is all in note values, so the tempo only sets the
    // sample <-> beat conversion and doesn't change the exported pattern.
    struct OfflinePlayHead : juce::AudioPlayHead
    {
        static constexpr double sampleRate = 48000.0, bpm = 120.0;
        juce::int64 pos = 0;
        bool playing = false;

        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo info;
            info.setBpm (bpm);
            info.setIsPlaying (playing);
            info.setTimeInSamples (pos);
            info.setPpqPosition ((double) pos / samplesPerBeat());
            info.setTimeSignature (TimeSignature { 4, 4 });
            return info;
        }
        static double samplesPerBeat() { return sampleRate * 60.0 / bpm; }
    };
}

// The lanes' notes first, in lane order (so FirstIn mode hands each lane the
// same note it has now), then any other held notes.
juce::Array<int> TugMidiSeqAudioProcessor::notesForExport() const
{
    juce::Array<int> notes;
    for (int i = 0; i < numOfLine; i++)
        if (laneInNote[i].load() >= 0)
            notes.addIfNotAlreadyThere (laneInNote[i].load());
    for (int n = 0; n < 128; n++)
        if (isNoteHeld (n))
            notes.addIfNotAlreadyThere (n);
    if (notes.isEmpty())
        notes = { 60, 64, 67, 71, 74 };   // nothing held: a C major 9 voicing, one note per lane
    return notes;
}

juce::MidiMessageSequence TugMidiSeqAudioProcessor::renderPattern (int bars, const juce::Array<int>& notes)
{
    constexpr int block = 512;
    constexpr double ticksPerBeat = 960.0;

    TugMidiSeqAudioProcessor copy;
    copy.offlineRender = true;
    juce::MemoryBlock state;
    getStateInformation (state);
    copy.setStateInformation (state.getData(), (int) state.getSize());
    copy.setParamValue (valueTreeNames[INBUILTSYNTH], 0.0f);
    copy.setParamValue (valueTreeNames[LATCH], 0.0f);   // the notes below are simply held

    OfflinePlayHead playHead;
    copy.setPlayHead (&playHead);
    copy.prepareToPlay (OfflinePlayHead::sampleRate, block);
    juce::AudioBuffer<float> buffer (2, block);

    juce::MidiMessageSequence out;
    auto runBlock = [&] (juce::MidiBuffer& midi, bool capture)
    {
        buffer.clear();
        copy.processBlock (buffer, midi);
        if (capture)
            for (const auto meta : midi)
            {
                auto m = meta.getMessage();
                if (! m.isNoteOnOrOff()) continue;
                const double beats = (double) (playHead.pos + meta.samplePosition) / OfflinePlayHead::samplesPerBeat();
                m.setTimeStamp (std::round (beats * ticksPerBeat));
                out.addEvent (m);
            }
        playHead.pos += block;
    };

    // stopped: take the notes in and let the engine reset its lanes
    {
        juce::MidiBuffer in;
        for (int n : notes)
            in.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), 0);
        runBlock (in, false);
        juce::MidiBuffer none;
        runBlock (none, false);
    }

    // play from bar 1
    const juce::int64 length = (juce::int64) std::llround (bars * 4 * OfflinePlayHead::samplesPerBeat());
    playHead.pos = 0;
    playHead.playing = true;
    while (playHead.pos < length)
    {
        juce::MidiBuffer midi;
        runBlock (midi, true);
    }

    // anything still sounding ends at the last bar line; drop what started after it
    const double endTick = bars * 4 * ticksPerBeat;
    for (int i = out.getNumEvents(); --i >= 0;)
        if (out.getEventPointer (i)->message.getTimeStamp() >= endTick && out.getEventPointer (i)->message.isNoteOn())
            out.deleteEvent (i, false);
    out.updateMatchedPairs();
    for (int i = 0; i < out.getNumEvents(); i++)
    {
        auto* e = out.getEventPointer (i);
        if (! e->message.isNoteOn()) continue;
        if (e->noteOffObject == nullptr)
            out.addEvent (juce::MidiMessage::noteOff (e->message.getChannel(), e->message.getNoteNumber()), endTick);
        else if (e->noteOffObject->message.getTimeStamp() > endTick)
            e->noteOffObject->message.setTimeStamp (endTick);
    }
    for (int i = out.getNumEvents(); --i >= 0;)   // orphaned note-offs past the end
        if (out.getEventPointer (i)->message.getTimeStamp() > endTick)
            out.deleteEvent (i, false);
    out.sort();
    out.updateMatchedPairs();
    return out;
}

juce::File TugMidiSeqAudioProcessor::renderPatternToMidiFile (int bars)
{
    auto sequence = renderPattern (bars, notesForExport());

    juce::MidiMessageSequence meta;
    meta.addEvent (juce::MidiMessage::textMetaEvent (3, "TugMidiSeq"), 0);   // track name
    meta.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0);

    juce::MidiFile file;
    file.setTicksPerQuarterNote (960);
    file.addTrack (meta);
    file.addTrack (sequence);

    // a fresh name each time: some hosts keep reading the dropped file later
    auto f = juce::File::getSpecialLocation (juce::File::tempDirectory)
                 .getChildFile ("TugMidiSeq " + juce::String (bars) + (bars == 1 ? " bar " : " bars ")
                                + juce::Time::getCurrentTime().formatted ("%H-%M-%S") + ".mid")
                 .getNonexistentSibling();
    juce::FileOutputStream os (f);
    if (! os.openedOk() || ! file.writeTo (os))
        return {};
    return f;
}

//==============================================================================
// Step pitch and scale lock

bool TugMidiSeqAudioProcessor::isInScale (int note) const
{
    const auto& scale = scaleIntervals[(size_t) jlimit (0, (int) scaleIntervals.size() - 1, getScaleType())];
    if (scale.empty()) return true;
    const int pc = ((note - getScaleKey()) % 12 + 12) % 12;
    return std::find (scale.begin(), scale.end(), pc) != scale.end();
}

// Scale off: note + offset semitones. Scale on: the note is snapped to the
// nearest scale tone (ties go down), then moved by `offset` scale degrees —
// so in C major, +2 from C is E, and an incoming F# plays as F.
int TugMidiSeqAudioProcessor::pitchedNote (int note, int offset) const
{
    const auto& scale = scaleIntervals[(size_t) jlimit (0, (int) scaleIntervals.size() - 1, getScaleType())];
    if (scale.empty())
        return jlimit (0, 127, note + offset);

    const int key  = getScaleKey();
    const int size = (int) scale.size();
    const int rel  = note - key;                                   // relative to the key's C-octave
    const int oct  = (rel >= 0 ? rel : rel - 11) / 12;             // floor division
    const int pc   = rel - oct * 12;

    // nearest scale degree to pc (searching down first, so ties go down)
    int degree = 0, best = 99;
    for (int d = 0; d < size; d++)
        for (int shift : { 0, -12, 12 })                           // wrap across the octave
        {
            const int dist = std::abs (scale[(size_t) d] + shift - pc);
            if (dist < best || (dist == best && scale[(size_t) d] + shift < pc))
            {
                best = dist;
                degree = d + (shift < 0 ? -size : shift > 0 ? size : 0);
            }
        }

    const int target    = degree + offset;
    const int targetOct = (target >= 0 ? target : target - (size - 1)) / size;
    const int targetDeg = target - targetOct * size;
    return jlimit (0, 127, key + (oct + targetOct) * 12 + scale[(size_t) targetDeg]);
}

//==============================================================================
// Play direction

int TugMidiSeqAudioProcessor::directedStep (int line, int slot) const
{
    const int n = jlimit (1, numOfStep, (int) *numOfGrid[line]);
    slot = jlimit (0, n - 1, slot);   // #Grid may have just shrunk under the slot
    switch ((int) *gridsDirectionAtomic[line])
    {
        case DirReverse:  return n - 1 - slot;
        case DirPingPong: return jmax (0, loopCount[line]) % 2 == 0 ? slot : n - 1 - slot;
        case DirRandom:   return juce::Random::getSystemRandom().nextInt (n);
        default:          return slot;
    }
}

// Whether the step a slot will play is active — used to keep a note from
// running into the next one. Random is unknown ahead, so every slot counts.
bool TugMidiSeqAudioProcessor::cellActiveAtSlot (int line, int slot) const
{
    if ((int) *gridsDirectionAtomic[line] == DirRandom)
        return true;
    return cellAt (line, directedStep (line, slot)) != 0;
}

//==============================================================================
// On-screen keyboard

void TugMidiSeqAudioProcessor::pushScreenNote (int note, bool on)
{
    int start1, size1, start2, size2;
    screenFifo.prepareToWrite (1, start1, size1, start2, size2);
    if (size1 + size2 == 0) return;   // full: drop rather than block the GUI
    screenEvents[size1 > 0 ? start1 : start2] = { note, on };
    screenFifo.finishedWrite (1);
}

void TugMidiSeqAudioProcessor::drainScreenNotes (juce::MidiBuffer& midiMessages)
{
    int start1, size1, start2, size2;
    screenFifo.prepareToRead (screenFifo.getNumReady(), start1, size1, start2, size2);
    auto add = [&midiMessages, this] (int start, int size)
    {
        for (int i = start; i < start + size; i++)
        {
            const auto& e = screenEvents[i];
            midiMessages.addEvent (e.on ? MidiMessage::noteOn (1, e.note, (uint8) 100)
                                        : MidiMessage::noteOff (1, e.note), 0);
        }
    };
    add (start1, size1);
    add (start2, size2);
    screenFifo.finishedRead (size1 + size2);
}

//==============================================================================
// Trig conditions

void TugMidiSeqAudioProcessor::resetTrigCondState()
{
    for (int i = 0; i < numOfLine; i++)
    {
        loopCount[i]      = -1;          // the first step after play wraps it to loop 0
        lastStep[i]       = numOfStep;   // above any real step, so that first step counts as a wrap
        lastCondResult[i] = false;
    }
}

bool TugMidiSeqAudioProcessor::evaluateTrigCond (int line, int step) const
{
    // A:B — true on loop A of every B loops
    static constexpr int ratio[][2] = { {1,2},{2,2},{1,3},{2,3},{3,3},{1,4},{2,4},{3,4},{4,4} };

    const int loop      = jmax (0, loopCount[line]);
    const int neighbour = (line + numOfLine - 1) % numOfLine;   // the lane below
    const int cond      = getStepCond (line, step);

    if (cond >= Cond1of2 && cond <= Cond4of4)
    {
        const auto& r = ratio[cond - Cond1of2];
        return loop % r[1] == r[0] - 1;
    }
    switch (cond)
    {
        case CondFirst:    return loop == 0;
        case CondNotFirst: return loop != 0;
        case CondPre:      return   lastCondResult[line];
        case CondNotPre:   return ! lastCondResult[line];
        case CondNei:      return   lastCondResult[neighbour];
        case CondNotNei:   return ! lastCondResult[neighbour];
        case CondFill:     return   isFillOn();
        case CondNotFill:  return ! isFillOn();
        default:           return true;
    }
}

// One comma-separated property per lane under a "stepConds" / "stepRatchets"
// child, so the per-step data round-trips with the DAW project alongside the
// parameters.
void TugMidiSeqAudioProcessor::writeStepDataTo (juce::ValueTree& state) const
{
    auto write = [&state] (const char* nodeName, auto&& valueAt)
    {
        auto node = state.getOrCreateChildWithName (nodeName, nullptr);
        for (int i = 0; i < numOfLine; i++)
        {
            juce::StringArray values;
            for (int j = 0; j < numOfStep; j++)
                values.add (juce::String (valueAt (i, j)));
            node.setProperty (juce::Identifier ("lane" + juce::String (i)), values.joinIntoString (","), nullptr);
        }
    };
    write ("stepConds",    [this] (int i, int j) { return getStepCond (i, j); });
    write ("stepRatchets", [this] (int i, int j) { return getStepRatchet (i, j); });
    write ("stepPitches",  [this] (int i, int j) { return getStepPitch (i, j); });
}

void TugMidiSeqAudioProcessor::readStepDataFrom (const juce::ValueTree& state)
{
    clearStepData();   // projects saved before these existed have none
    auto read = [&state] (const char* nodeName, auto&& setAt)
    {
        auto node = state.getChildWithName (nodeName);
        if (! node.isValid()) return;
        for (int i = 0; i < numOfLine; i++)
        {
            auto values = juce::StringArray::fromTokens (node.getProperty (juce::Identifier ("lane" + juce::String (i))).toString(), ",", "");
            for (int j = 0; j < numOfStep && j < values.size(); j++)
                setAt (i, j, values[j].getIntValue());
        }
    };
    read ("stepConds",    [this] (int i, int j, int v) { setStepCond (i, j, v); });
    read ("stepRatchets", [this] (int i, int j, int v) { setStepRatchet (i, j, v); });
    read ("stepPitches",  [this] (int i, int j, int v) { setStepPitch (i, j, v); });
}

//==============================================================================
// Pattern slots

void TugMidiSeqAudioProcessor::resetSlots()
{
    for (int s = 0; s < numSlots; s++)
        for (int i = 0; i < numOfLine; i++)
            for (int j = 0; j < numOfStep; j++)
            {
                slotCell[s][i][j].store (0);
                slotVel[s][i][j].store (100);
                stepCond[s][i][j].store (CondNone);
                stepRatchet[s][i][j].store (1);
                stepPitch[s][i][j].store (0);
            }
}

void TugMidiSeqAudioProcessor::switchLaneSlot (int line, int slot)
{
    activeSlot[line].store (slot, std::memory_order_relaxed);
    mutateMask[line] = 0;   // mutations belong to the pattern that was playing
}

void TugMidiSeqAudioProcessor::cellParamChanged (int line, int step, bool velocity, float value)
{
    // mirrorActiveSlots copying a slot into the pads: the slot already holds it
    if (mirroring && juce::MessageManager::existsAndIsCurrentThread())
        return;
    const int slot = getActiveSlot (line);
    (velocity ? slotVel : slotCell)[slot][line][step].store ((int) std::lround (value), std::memory_order_relaxed);
}

void TugMidiSeqAudioProcessor::requestSlot (int slot)
{
    setParamValue (valueTreeNames[PATTERNSLOT], (float) jlimit (0, numSlots - 1, slot));
}

bool TugMidiSeqAudioProcessor::isSlotEmpty (int slot) const
{
    for (int i = 0; i < numOfLine; i++)
        for (int j = 0; j < numOfStep; j++)
            if (slotCell[slot][i][j].load (std::memory_order_relaxed) != 0)
                return false;
    return true;
}

// Copies each lane's active slot into the pad parameters once the lane
// switched (or a slot it plays was rewritten). The values go into the APVTS
// tree without the undo manager, so the copy isn't an undoable edit; after a
// switch the history is cleared instead, because its entries name pads, and
// the pads now stand for another slot.
void TugMidiSeqAudioProcessor::mirrorActiveSlots()
{
    bool pending = false;
    for (int i = 0; i < numOfLine; i++)
        pending |= getActiveSlot (i) != mirroredSlot[i] || mirrorDirty[i].load();
    if (! pending) return;

    (void) valueTreeState.copyState();   // flush first, so the tree matches the pads
    bool switched = false;
    for (int i = 0; i < numOfLine; i++)
    {
        const int slot = getActiveSlot (i);
        const bool dirty = mirrorDirty[i].exchange (false);
        if (slot == mirroredSlot[i] && ! dirty) continue;
        switched |= slot != mirroredSlot[i];
        mirroredSlot[i] = slot;

        const juce::ScopedValueSetter<bool> svs (mirroring, true);
        for (int j = 0; j < numOfStep; j++)
            for (bool vel : { false, true })
            {
                const auto id = valueTreeNames[vel ? VELGRIDBUTTON : BLOCK] + juce::String (i) + juce::String (j);
                const double value = (vel ? slotVel : slotCell)[slot][i][j].load();
                auto child = valueTreeState.state.getChildWithProperty ("id", id);
                if (child.isValid())
                    child.setProperty ("value", value, nullptr);
                else
                    setParamValue (id, (float) value);
            }
    }
    if (switched)
        undoManager.clearUndoHistory();
    myGridChangeListener.sendChangeMessage();
}

// one slot's step data, five values per step: cell, velocity, condition, ratchet, pitch
void TugMidiSeqAudioProcessor::snapshotSlot (int slot, std::vector<int>& out) const
{
    out.clear();
    for (int i = 0; i < numOfLine; i++)
        for (int j = 0; j < numOfStep; j++)
            for (auto* a : { &slotCell, &slotVel, &stepCond, &stepRatchet, &stepPitch })
                out.push_back ((*a)[slot][i][j].load());
}

void TugMidiSeqAudioProcessor::restoreSlot (int slot, const std::vector<int>& data)
{
    size_t k = 0;
    for (int i = 0; i < numOfLine; i++)
    {
        for (int j = 0; j < numOfStep; j++)
            for (auto* a : { &slotCell, &slotVel, &stepCond, &stepRatchet, &stepPitch })
                (*a)[slot][i][j].store (data[k++]);
        if (getActiveSlot (i) == slot)
            mirrorDirty[i].store (true);
    }
    notifyStateChanged();
}

namespace
{
    // copy / clear of a whole slot, as one undo step
    struct SlotDataAction : juce::UndoableAction
    {
        SlotDataAction (TugMidiSeqAudioProcessor& p, int s, std::vector<int> from, std::vector<int> to)
            : proc (p), slot (s), before (std::move (from)), after (std::move (to)) {}
        bool perform() override { proc.restoreSlotData (slot, after);  return true; }
        bool undo() override    { proc.restoreSlotData (slot, before); return true; }
        int getSizeInUnits() override { return 10; }
        TugMidiSeqAudioProcessor& proc;
        int slot;
        std::vector<int> before, after;
    };
}

void TugMidiSeqAudioProcessor::copySlot (int from, int to)
{
    if (from == to) return;
    std::vector<int> before, after;
    snapshotSlot (to, before);
    snapshotSlot (from, after);
    flushToUndo();
    undoManager.beginNewTransaction();
    undoManager.perform (new SlotDataAction (*this, to, std::move (before), std::move (after)));
    undoManager.beginNewTransaction();
}

void TugMidiSeqAudioProcessor::clearSlot (int slot)
{
    std::vector<int> before, after;
    snapshotSlot (slot, before);
    after = before;
    for (size_t k = 0; k < after.size(); k += 5)
    {
        after[k] = 0; after[k + 1] = 100; after[k + 2] = CondNone; after[k + 3] = 1; after[k + 4] = 0;
    }
    flushToUndo();
    undoManager.beginNewTransaction();
    undoManager.perform (new SlotDataAction (*this, slot, std::move (before), std::move (after)));
    undoManager.beginNewTransaction();
}

// "patternSlots": the lanes' active slots, and per slot and lane one
// comma-separated list for each kind of step data.
void TugMidiSeqAudioProcessor::writeSlotsTo (juce::ValueTree& state) const
{
    auto node = state.getOrCreateChildWithName ("patternSlots", nullptr);
    juce::StringArray actives;
    for (int i = 0; i < numOfLine; i++)
        actives.add (juce::String (getActiveSlot (i)));
    node.setProperty ("active", actives.joinIntoString (","), nullptr);

    const char* kinds[] = { "cells", "vels", "conds", "ratchets", "pitches" };
    const std::atomic<int> (*arrays[])[numOfLine][numOfStep] = { slotCell, slotVel, stepCond, stepRatchet, stepPitch };
    for (int s = 0; s < numSlots; s++)
        for (int i = 0; i < numOfLine; i++)
            for (int k = 0; k < 5; k++)
            {
                juce::StringArray values;
                for (int j = 0; j < numOfStep; j++)
                    values.add (juce::String (arrays[k][s][i][j].load()));
                node.setProperty (juce::Identifier (juce::String (kinds[k]) + "_" + juce::String (s) + "_" + juce::String (i)),
                                  values.joinIntoString (","), nullptr);
            }
}

bool TugMidiSeqAudioProcessor::readSlotsFrom (const juce::ValueTree& state)
{
    const auto node = state.getChildWithName ("patternSlots");
    if (! node.isValid()) return false;

    const char* kinds[] = { "cells", "vels", "conds", "ratchets", "pitches" };
    std::atomic<int> (*arrays[])[numOfLine][numOfStep] = { slotCell, slotVel, stepCond, stepRatchet, stepPitch };
    const int lo[] = { 0, 0, 0, 1, -maxStepPitch }, hi[] = { 2, 127, NumTrigConds - 1, maxRatchet, maxStepPitch };
    for (int s = 0; s < numSlots; s++)
        for (int i = 0; i < numOfLine; i++)
            for (int k = 0; k < 5; k++)
            {
                const auto key = juce::Identifier (juce::String (kinds[k]) + "_" + juce::String (s) + "_" + juce::String (i));
                if (! node.hasProperty (key)) continue;
                const auto values = juce::StringArray::fromTokens (node.getProperty (key).toString(), ",", "");
                for (int j = 0; j < numOfStep && j < values.size(); j++)
                    arrays[k][s][i][j].store (jlimit (lo[k], hi[k], values[j].getIntValue()));
            }
    return true;
}

//==============================================================================
// Note map snapshot for the GUI

void TugMidiSeqAudioProcessor::publishNoteMap()
{
    uint64_t held[2] = {}, phys[2] = {};
    for (const auto& m : inMidiNoteList)
    {
        const int n = m.getNoteNumber();
        held[n >> 6] |= (uint64_t) 1 << (n & 63);
    }
    for (int n = 0; n < 128; n++)
        if (physHeld[n])
            phys[n >> 6] |= (uint64_t) 1 << (n & 63);
    for (int i = 0; i < 2; i++)
    {
        heldMask[i].store (held[i], std::memory_order_relaxed);
        physMask[i].store (phys[i], std::memory_order_relaxed);
    }

    const bool firstIn = *sortedOrFirstEmptySelectAtomic != 0;
    for (int i = 0; i < numOfLine; i++)
    {
        int in = -1;
        if (! firstIn)
        {
            if (inMidiNoteList.size() > (size_t) i)
                in = inMidiNoteList[(size_t) i].getNoteNumber();
        }
        else if (inMidiNoteListVector.at (i).getVelocity() != 0)
        {
            in = inMidiNoteListVector.at (i).getNoteNumber();
        }
        laneInNote[i].store (in, std::memory_order_relaxed);
        laneOutNote[i].store (myIsPlaying && midiState[i] ? lastOutNote[i] : -1, std::memory_order_relaxed);

        const int slotLen = stepResetIntervalForShuffle[i][jlimit (0, numOfStep - 1, steps[i])];
        const int dir     = getDirection (i);
        pubPlayStep[i].store (myIsPlaying ? playStep[i] : -1, std::memory_order_relaxed);
        pubPlayFrac[i].store (slotLen > 0 ? jlimit (0.0f, 1.0f, (float) stpSample[i] / (float) slotLen) : 0.0f,
                              std::memory_order_relaxed);
        pubBackward[i].store (dir == DirReverse || (dir == DirPingPong && jmax (0, loopCount[i]) % 2 == 1),
                              std::memory_order_relaxed);
        pubMutateMask[i].store (myIsPlaying ? mutateMask[i] : 0u, std::memory_order_relaxed);
    }
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

