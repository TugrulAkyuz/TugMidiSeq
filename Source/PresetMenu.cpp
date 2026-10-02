/*
 ==============================================================================
 
 PresetMenu.cpp
 Created: 27 May 2022 5:02:06pm
 Author:  Tuğrul Akyüz
 
 ==============================================================================
 */

#include "PluginProcessor.h"

extern ChangeBroadcaster myGridChangeListener;

void TugMidiSeqAudioProcessor::resetAllParam()
{
    String tmp_s;
    for(int j = 0 ; j <  numOfLine; j++)
    {
        for(int i = 0 ; i < numOfStep ; i++)
        {
            tmp_s.clear();
            tmp_s << valueTreeNames[BLOCK] << j << i;
            valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
            
            tmp_s.clear();
            tmp_s << valueTreeNames[VELGRIDBUTTON] << j << i;
            valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        }
        tmp_s.clear();
        tmp_s << valueTreeNames[SPEEED] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        tmp_s.clear();
        tmp_s << valueTreeNames[DUR] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDNUM] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        tmp_s.clear();
        tmp_s << valueTreeNames[OCTAVE] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        tmp_s.clear();
        tmp_s << valueTreeNames[VEL] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        tmp_s.clear();
        tmp_s << valueTreeNames[EVENT] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDDELAY] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDSHUFFLE] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
        
        tmp_s.clear();
        tmp_s << valueTreeNames[GRIDMIDIROUTE] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());

        tmp_s.clear();
        tmp_s << valueTreeNames[DIRECTION] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
    }
    // through the undo history: Reset is undoable (GlobalPanel wraps it)
    for (int j = 0; j < numOfLine; j++)
        for (int i = 0; i < numOfStep; i++)
        {
            setStepCondUndoable (j, i, CondNone);
            setStepRatchetUndoable (j, i, 1);
            setStepPitchUndoable (j, i, 0);
        }
    notifyStateChanged();
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
    
    //    tmp_s.clear();
    //    tmp_s << valueTreeNames[INBUILTSYNTH];
    //    valueTreeState.getParameterAsValue(tmp_s).setValue(0);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());
    
    tmp_s.clear();
    tmp_s << valueTreeNames[CHANNON];
    valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());

    for (int id : { SCALEKEY, SCALETYPE })
        valueTreeState.getParameter(valueTreeNames[id])->setValueNotifyingHost(valueTreeState.getParameter(valueTreeNames[id])->getDefaultValue());
    
    // mySynth.clearSounds();
    
    //initPrepareValue();
    
    myGridChangeListener.sendChangeMessage();
}

var TugMidiSeqAudioProcessor::presetToVar(const TugMidiSeqProgram& prg)
{
    var newObj ( new DynamicObject() );
    juce::String tmp_s;
    newObj.getDynamicObject()->setProperty("PresetName", prg.myProgramname);

    for(int i = 0 ; i < numOfLine; i++)
    {
        for(int j = 0 ; j < numOfStep ; j++)
        {
            tmp_s.clear();
            tmp_s <<valueTreeNames[BLOCK]<< i << j;
            newObj.getDynamicObject()->setProperty(tmp_s, prg.grids[i][j]);

            tmp_s.clear();
            tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
            newObj.getDynamicObject()->setProperty(tmp_s, prg.gridVelArr[i][j]);

            // sparse: only steps that actually carry a condition / ratchet
            if (prg.stepCond[i][j] != CondNone)
            {
                tmp_s.clear();
                tmp_s << stepCondKey << i << j;
                newObj.getDynamicObject()->setProperty(tmp_s, prg.stepCond[i][j]);
            }
            if (prg.stepRatchet[i][j] > 1)
            {
                tmp_s.clear();
                tmp_s << stepRatchetKey << i << j;
                newObj.getDynamicObject()->setProperty(tmp_s, prg.stepRatchet[i][j]);
            }
            if (prg.stepPitch[i][j] != 0)
            {
                tmp_s.clear();
                tmp_s << stepPitchKey << i << j;
                newObj.getDynamicObject()->setProperty(tmp_s, prg.stepPitch[i][j]);
            }
        }

        tmp_s.clear();
        tmp_s <<valueTreeNames[SPEEED]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsSpeed[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[DUR]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsDuration[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDNUM]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.numOfGrid[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[OCTAVE]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.octave[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[VEL]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsVel[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[EVENT]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsEvent[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDSHUFFLE]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsShuffle[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDDELAY]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsDelay[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDMIDIROUTE]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.gridsMidiRoute[i]);

        tmp_s.clear();
        tmp_s <<valueTreeNames[DIRECTION]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.direction[i]);
    }
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.globalResyncBar);

    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.GlobalInOrFixedVel);

    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.inBuiltSynth);

    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.sortedOrFirst);

    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.shuffle);

    tmp_s.clear();
    tmp_s << valueTreeNames[CHANNON];
    newObj.getDynamicObject()->setProperty(tmp_s, prg.channelOn);

    newObj.getDynamicObject()->setProperty(valueTreeNames[SCALEKEY],  prg.scaleKey);
    newObj.getDynamicObject()->setProperty(valueTreeNames[SCALETYPE], prg.scaleType);

    return newObj;
}

// bundle (legacy) writer — only writes presets that still live in the bundle
// file; presets that have their own single file are skipped
void  TugMidiSeqAudioProcessor::writePresetToFileJSON()
{
    if (resourceJsonFile == nullptr) return;

    DynamicObject* tree = new DynamicObject();
    Array<var> arr;
    for(auto p = 0 ; p < myProgram.size(); p++)
    {
        if (myProgram.at(p).sourceFile != File()) continue;  // lives in its own file
        arr.add(presetToVar(myProgram.at(p)));
    }
    tree->setProperty("Presets",arr);

    // replaceWithText truncates safely (stream + setPosition(0) left stale
    // bytes behind when the JSON got shorter, corrupting the file)
    resourceJsonFile->replaceWithText(JSON::toString(var(tree)));
}

void TugMidiSeqAudioProcessor::writeSinglePresetToFileJSON(TugMidiSeqProgram& prg)
{
    if (resourceJsonFile == nullptr) return;
    if (prg.sourceFile == File())
    {
        auto dir = presetFolder.isDirectory() ? presetFolder
                                              : resourceJsonFile->getParentDirectory();
        String legal = File::createLegalFileName(prg.myProgramname);
        if (legal.isEmpty()) legal = "Preset";
        auto f = dir.getChildFile(legal + ".json");
        if (f == *resourceJsonFile) f = dir.getChildFile(legal + "_preset.json");
        prg.sourceFile = f;
    }
    DynamicObject* tree = new DynamicObject();
    Array<var> arr;  arr.add(presetToVar(prg));
    tree->setProperty("Presets", arr);
    prg.sourceFile.replaceWithText(JSON::toString(var(tree)));
}

TugMidiSeqProgram TugMidiSeqAudioProcessor::varToPreset(const var& preset)
{
    juce::String  tmp_s;
    String strName = preset.getProperty("PresetName", var()).toString();

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

            tmp_s.clear();
            tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
            if (preset.hasProperty(tmp_s))
               v = preset.getProperty(tmp_s, var());
            else
                v =  90;
            p.gridVelArr[i][j] = v;

            tmp_s.clear();
            tmp_s << stepCondKey << i << j;
            if (preset.hasProperty(tmp_s))
                p.stepCond[i][j] = jlimit(0, NumTrigConds - 1, (int) preset.getProperty(tmp_s, var()));

            tmp_s.clear();
            tmp_s << stepRatchetKey << i << j;
            if (preset.hasProperty(tmp_s))
                p.stepRatchet[i][j] = jlimit(1, maxRatchet, (int) preset.getProperty(tmp_s, var()));

            tmp_s.clear();
            tmp_s << stepPitchKey << i << j;
            if (preset.hasProperty(tmp_s))
                p.stepPitch[i][j] = jlimit(-maxStepPitch, maxStepPitch, (int) preset.getProperty(tmp_s, var()));

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
        tmp_s <<valueTreeNames[EVENT]<< i;
        v = preset.getProperty(tmp_s, var());
        p.gridsEvent[i] = v;


        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDSHUFFLE]<< i;
        v = preset.getProperty(tmp_s, var());
        p.gridsShuffle[i] = v;

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDDELAY]<< i;
        v = preset.getProperty(tmp_s, var());
        p.gridsDelay[i] = v;

        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDMIDIROUTE]<< i;
        if (preset.hasProperty(tmp_s)) {
            v = preset.getProperty(tmp_s, var());
        } else {
            v =   valueTreeState.getParameter(tmp_s)->convertFrom0to1(  valueTreeState.getParameter(tmp_s)->getDefaultValue()); //
        }
        p.gridsMidiRoute[i] = v;

        tmp_s.clear();
        tmp_s <<valueTreeNames[DIRECTION]<< i;
        if (preset.hasProperty(tmp_s))
            p.direction[i] = jlimit(0, directionNames.size() - 1, (int) preset.getProperty(tmp_s, var()));

    }
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    v = preset.getProperty(tmp_s, var());
    p.globalResyncBar = v;

    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    v = preset.getProperty(tmp_s, var());
    p.GlobalInOrFixedVel = v;


    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    v = preset.getProperty(tmp_s, var());
    p.inBuiltSynth = v;

    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    v = preset.getProperty(tmp_s, var());
    p.sortedOrFirst = v;

    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    v = preset.getProperty(tmp_s, var());
    p.shuffle = v;

    tmp_s.clear();
    tmp_s << valueTreeNames[CHANNON];
    if (preset.hasProperty(tmp_s)) {
        v = preset.getProperty(tmp_s, var());
        p.channelOn = v != 0;
    }

    if (preset.hasProperty(valueTreeNames[SCALEKEY]))
        p.scaleKey = jlimit(0, scaleKeyNames.size() - 1, (int) preset.getProperty(valueTreeNames[SCALEKEY], var()));
    if (preset.hasProperty(valueTreeNames[SCALETYPE]))
        p.scaleType = jlimit(0, scaleTypeNames.size() - 1, (int) preset.getProperty(valueTreeNames[SCALETYPE], var()));

    return p;
}

void  TugMidiSeqAudioProcessor::readPresetToFileJSON()
{
    juce::FileInputStream inputStream (*resourceJsonFile);
    if (inputStream.failedToOpen())
        return;
    String sil_string = inputStream.readString();

    var jsonReply = JSON::parse(sil_string);
    Array<var>* presetArray= jsonReply.getProperty("Presets", var()).getArray();
    if (presetArray == nullptr)
        return;  // corrupt / foreign file

    for (auto& preset : *presetArray)
    {
        preset_idex++;
        TugMidiSeqProgram p = varToPreset(preset);
        p.sourceFile = File();  // lives in the legacy bundle file
        myProgram.push_back(p);
    }
}

void TugMidiSeqAudioProcessor::readSinglePresetFilesJSON()
{
    if (resourceJsonFile == nullptr) return;
    auto dir = presetFolder.isDirectory() ? presetFolder
                                          : resourceJsonFile->getParentDirectory();
    auto files = dir.findChildFiles(File::findFiles, false, "*.json");
    files.sort();
    for (auto& f : files)
    {
        if (f == *resourceJsonFile) continue;          // the bundle file itself
        var jsonReply = JSON::parse(f.loadFileAsString());
        Array<var>* presetArray = jsonReply.getProperty("Presets", var()).getArray();
        if (presetArray == nullptr) continue;          // not our format
        for (auto& preset : *presetArray)
        {
            preset_idex++;
            TugMidiSeqProgram p = varToPreset(preset);
            p.sourceFile = f;
            myProgram.push_back(p);
        }
    }
}

void TugMidiSeqAudioProcessor::setPresetFolder(const File& dir)
{
    if (!dir.isDirectory()) return;
    presetFolder = dir;
    if (auto* props = appProperties.getUserSettings())
    {
        props->setValue("presetFolder", presetFolder.getFullPathName());
        props->saveIfNeeded();
    }
    myProgram.clear();
    preset_idex = 0;
    readPresetToFileJSON();        // bundle always from its default location
    readSinglePresetFilesJSON();   // singles from the new folder
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
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
            
            paramProg.gridVelArr[i][j] = *valueTreeState.getRawParameterValue(tmp_s);

            paramProg.stepCond[i][j] = getStepCond(i, j);
            paramProg.stepRatchet[i][j] = getStepRatchet(i, j);
            paramProg.stepPitch[i][j] = getStepPitch(i, j);
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
        tmp_s <<valueTreeNames[EVENT]<< i;
        
        paramProg.gridsEvent[i] = *valueTreeState.getRawParameterValue(tmp_s);;
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDSHUFFLE]<< i;
        
        paramProg.gridsShuffle[i] = *valueTreeState.getRawParameterValue(tmp_s);;
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDDELAY]<< i;
        
        paramProg.gridsDelay[i] = *valueTreeState.getRawParameterValue(tmp_s);
        
        tmp_s.clear();
        tmp_s <<valueTreeNames[GRIDMIDIROUTE]<< i;
        
        paramProg.gridsMidiRoute[i] = *valueTreeState.getRawParameterValue(tmp_s);;

        tmp_s.clear();
        tmp_s <<valueTreeNames[DIRECTION]<< i;
        paramProg.direction[i] = (int) *valueTreeState.getRawParameterValue(tmp_s);
        
        
    }
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOBALRESTBAR];
    paramProg.globalResyncBar = *valueTreeState.getRawParameterValue(tmp_s);;
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GLOABLINORFIXVEL];
    paramProg.GlobalInOrFixedVel = *valueTreeState.getRawParameterValue(tmp_s);;
    
    
    
    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    paramProg.inBuiltSynth = *valueTreeState.getRawParameterValue(tmp_s);;
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    paramProg.sortedOrFirst = *valueTreeState.getRawParameterValue(tmp_s);;
    
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    paramProg.shuffle = *valueTreeState.getRawParameterValue(tmp_s);;
    
    tmp_s.clear();
    tmp_s << valueTreeNames[CHANNON];
    paramProg.channelOn = *valueTreeState.getRawParameterValue(tmp_s);;
    paramProg.scaleKey  = (int) *valueTreeState.getRawParameterValue(valueTreeNames[SCALEKEY]);
    paramProg.scaleType = (int) *valueTreeState.getRawParameterValue(valueTreeNames[SCALETYPE]);
    
    myProgram.push_back(paramProg);
    writeSinglePresetToFileJSON(myProgram.back());  // new presets get their own file
}


void TugMidiSeqAudioProcessor::deletePreset(int index)   // index is 1-based
{
    if(index == 0) return;
    auto& prg = myProgram.at(index - 1);
    if (prg.sourceFile != File())
        prg.sourceFile.deleteFile();       // single-file preset: delete its own file
    myProgram.erase(myProgram.begin() + index -1);
    // bundle residents: caller rewrites the bundle via writePresetToFileJSON()
}
