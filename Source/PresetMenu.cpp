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

        tmp_s.clear();
        tmp_s << valueTreeNames[MUTATE] << j;
        valueTreeState.getParameter(tmp_s)->setValueNotifyingHost(valueTreeState.getParameter(tmp_s)->getDefaultValue());

        for (int id : { PLAYMODE, SPREAD, STRUMSHAPE, STRUMTENSION, STRUMVEL, STRUMHUMAN, STRUMSYNC, STRUMDIV })
        {
            auto* prm = valueTreeState.getParameter (valueTreeNames[id] + juce::String (j));
            prm->setValueNotifyingHost (prm->getDefaultValue());
        }
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

        tmp_s.clear();
        tmp_s <<valueTreeNames[MUTATE]<< i;
        newObj.getDynamicObject()->setProperty(tmp_s, prg.mutate[i]);

        newObj.getDynamicObject()->setProperty(valueTreeNames[PLAYMODE] + juce::String (i), prg.playMode[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[SPREAD] + juce::String (i), prg.spread[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMSHAPE] + juce::String (i), prg.strumShape[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMTENSION] + juce::String (i), prg.strumTension[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMVEL] + juce::String (i), prg.strumVel[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMHUMAN] + juce::String (i), prg.strumHuman[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMSYNC] + juce::String (i), prg.strumSync[i]);
        newObj.getDynamicObject()->setProperty(valueTreeNames[STRUMDIV] + juce::String (i), prg.strumDiv[i]);
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

        tmp_s.clear();
        tmp_s <<valueTreeNames[MUTATE]<< i;
        if (preset.hasProperty(tmp_s))
            p.mutate[i] = jlimit(0, 100, (int) preset.getProperty(tmp_s, var()));

        if (preset.hasProperty(valueTreeNames[PLAYMODE] + juce::String (i)))
            p.playMode[i] = jlimit(0, playModeNames.size() - 1, (int) preset.getProperty(valueTreeNames[PLAYMODE] + juce::String (i), var()));
        if (preset.hasProperty(valueTreeNames[SPREAD] + juce::String (i)))
            p.spread[i] = jlimit(-maxStrumMs, maxStrumMs, (int) preset.getProperty(valueTreeNames[SPREAD] + juce::String (i), var()));
        auto readInt = [&] (int base, int lo, int hi, int& dest)
        {
            const auto key = valueTreeNames[base] + juce::String (i);
            if (preset.hasProperty (key))
                dest = jlimit (lo, hi, (int) preset.getProperty (key, var()));
        };
        readInt (STRUMSHAPE, 0, strumShapeNames.size() - 1, p.strumShape[i]);
        readInt (STRUMTENSION, -100, 100, p.strumTension[i]);
        readInt (STRUMVEL, -100, 100, p.strumVel[i]);
        readInt (STRUMHUMAN, 0, 100, p.strumHuman[i]);
        readInt (STRUMSYNC, 0, 1, p.strumSync[i]);
        readInt (STRUMDIV, 0, strumDivNames.size() - 1, p.strumDiv[i]);

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



// The current pattern and settings as a preset (not written anywhere).
TugMidiSeqProgram TugMidiSeqAudioProcessor::captureCurrentProgram (const juce::String& preset_name)
{
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

        tmp_s.clear();
        tmp_s <<valueTreeNames[MUTATE]<< i;
        paramProg.mutate[i] = (int) *valueTreeState.getRawParameterValue(tmp_s);
        paramProg.playMode[i] = (int) *valueTreeState.getRawParameterValue(valueTreeNames[PLAYMODE] + juce::String (i));
        paramProg.spread[i]   = (int) *valueTreeState.getRawParameterValue(valueTreeNames[SPREAD] + juce::String (i));
        paramProg.strumShape[i]   = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMSHAPE] + juce::String (i));
        paramProg.strumTension[i] = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMTENSION] + juce::String (i));
        paramProg.strumVel[i]     = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMVEL] + juce::String (i));
        paramProg.strumHuman[i]   = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMHUMAN] + juce::String (i));
        paramProg.strumSync[i]    = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMSYNC] + juce::String (i));
        paramProg.strumDiv[i]     = (int) *valueTreeState.getRawParameterValue(valueTreeNames[STRUMDIV] + juce::String (i));
        
        
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
    
    return paramProg;
}

//==============================================================================
// Preset library

namespace
{
    const juce::String legacyBundleName = "TugMidiSeqPresets.json";

    // the presets a file holds: its "Presets" array (every file the plugin has
    // written), or the file itself when it's a single preset object
    juce::Array<juce::var> presetsInFile (const juce::File& f)
    {
        const auto json = juce::JSON::parse (f.loadFileAsString());
        if (auto* arr = json.getProperty ("Presets", juce::var()).getArray())
            return *arr;
        if (json.isObject())
            return { json };
        return {};
    }

    bool isPresetFile (const juce::File& f)
    {
        return ! f.getFileName().startsWithChar ('.') && f.hasFileExtension ("json")
               && ! (f.getFileName() == legacyBundleName && f.getParentDirectory() == TugMidiSeqAudioProcessor::defaultPresetFolder());
    }

    // a folder's presets in menu order: its sub-folders (sorted, each the same
    // way down) and then its own files (sorted)
    void collectPresetFiles (const juce::File& folder, juce::Array<juce::File>& out, int depth = 0)
    {
        if (depth > 16) return;   // a link loop must not run away
        auto dirs = folder.findChildFiles (juce::File::findDirectories, false);
        dirs.sort();
        for (auto& d : dirs)
            if (! d.getFileName().startsWithChar ('.'))
                collectPresetFiles (d, out, depth + 1);
        auto files = folder.findChildFiles (juce::File::findFiles, false, "*.json");
        files.sort();
        for (auto& f : files)
            if (isPresetFile (f))
                out.add (f);
    }

    juce::String keyOf (const TugMidiSeqProgram& p)
    {
        return p.sourceFile.getFullPathName() + "#" + juce::String (p.sourceEntry);
    }

    // a folder path typed by the user, made safe: "Bass / Dark" -> "Bass/Dark"
    juce::String cleanCategory (const juce::String& category)
    {
        juce::StringArray parts;
        parts.addTokens (category.replaceCharacter ('\\', '/'), "/", "");
        juce::StringArray kept;
        for (auto p : parts)
        {
            p = juce::File::createLegalFileName (p.trim());
            if (p.isNotEmpty() && p != "." && p != "..")
                kept.add (p);
        }
        return kept.joinIntoString ("/");
    }
}

// TUGMIDISEQ_TEST_ROOT (set by Tests/EngineTest) moves the library into a
// scratch folder and keeps the settings file out of it altogether.
juce::String TugMidiSeqAudioProcessor::testRoot()
{
    return juce::SystemStats::getEnvironmentVariable ("TUGMIDISEQ_TEST_ROOT", {});
}

juce::PropertiesFile* TugMidiSeqAudioProcessor::settings()
{
    return testRoot().isEmpty() ? appProperties.getUserSettings() : nullptr;
}

juce::File TugMidiSeqAudioProcessor::defaultPresetFolder()
{
    if (testRoot().isNotEmpty())
        return juce::File (testRoot()).getChildFile ("Presets");
   #if JUCE_MAC
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Library/Audio/Presets/2Rule/TugMidiSeq");
   #else   // %APPDATA%\2Rule\TugMidiSeq on Windows, ~/.config/2Rule/TugMidiSeq on Linux
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("2Rule").getChildFile ("TugMidiSeq");
   #endif
}

void TugMidiSeqAudioProcessor::rescanPresets()
{
    juce::Array<juce::File> files;
    collectPresetFiles (presetFolder, files);
    std::vector<TugMidiSeqProgram> found;
    for (auto& f : files)
    {
        const auto entries = presetsInFile (f);
        for (int e = 0; e < entries.size(); e++)
        {
            auto p = varToPreset (entries[e]);
            p.sourceFile  = f;
            p.sourceEntry = e;
            // a file of its own is named by its file; an old multi-preset file by its entries
            if (entries.size() == 1 || p.myProgramname.isEmpty())
                p.myProgramname = f.getFileNameWithoutExtension() + (entries.size() > 1 ? " " + juce::String (e + 1) : juce::String());
            found.push_back (p);
        }
    }
    myProgram = std::move (found);
    preset_idex = (int) myProgram.size();

    program = 0;   // the selection follows its file, not its old number
    for (size_t i = 0; i < myProgram.size(); i++)
        if (keyOf (myProgram[i]) == currentPresetKey)
            program = (int) i + 1;

    // the host's program list changed; only with a preset chosen, since AU would
    // otherwise show program 0 as the current one
    if (program > 0)
        updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

void TugMidiSeqAudioProcessor::setPresetFolder (const juce::File& dir)
{
    if (! dir.isDirectory()) return;
    presetFolder = dir;
    if (auto* props = settings())
    {
        if (dir == defaultPresetFolder()) props->removeValue ("presetFolder");
        else                              props->setValue ("presetFolder", dir.getFullPathName());
        props->saveIfNeeded();
    }
    rescanPresets();
}

juce::String TugMidiSeqAudioProcessor::presetCategory (int index) const
{
    if (index < 1 || index > (int) myProgram.size()) return {};
    const auto folder = myProgram[(size_t) index - 1].sourceFile.getParentDirectory();
    if (folder == presetFolder || ! folder.isAChildOf (presetFolder)) return {};
    return folder.getRelativePathFrom (presetFolder).replaceCharacter ('\\', '/');
}

juce::File TugMidiSeqAudioProcessor::presetFileFor (const juce::String& name, const juce::String& category) const
{
    auto legal = juce::File::createLegalFileName (name.trim());
    if (legal.isEmpty()) legal = "Preset";
    const auto cat = cleanCategory (category);
    const auto folder = cat.isEmpty() ? presetFolder : presetFolder.getChildFile (cat);
    return folder.getChildFile (legal + ".json");
}

juce::File TugMidiSeqAudioProcessor::savePresetAs (const juce::String& name, const juce::String& category)
{
    const auto file = presetFileFor (name, category);
    file.getParentDirectory().createDirectory();
    auto prog = captureCurrentProgram (file.getFileNameWithoutExtension());
    juce::Array<juce::var> one;
    one.add (presetToVar (prog));
    auto* root = new juce::DynamicObject();
    root->setProperty ("Presets", one);   // the layout every TugMidiSeq version reads
    if (! file.replaceWithText (juce::JSON::toString (juce::var (root))))
        return {};
    currentPresetKey = file.getFullPathName() + "#0";
    rescanPresets();
    return file;
}

bool TugMidiSeqAudioProcessor::deleteCurrentPreset()
{
    if (program < 1 || program > (int) myProgram.size()) return false;
    const auto prog = myProgram[(size_t) program - 1];
    auto entries = presetsInFile (prog.sourceFile);
    bool done;
    if (entries.size() <= 1)
        done = prog.sourceFile.deleteFile();
    else
    {   // an old multi-preset file: take this one out, keep the rest
        entries.remove (prog.sourceEntry);
        auto* root = new juce::DynamicObject();
        root->setProperty ("Presets", entries);
        done = prog.sourceFile.replaceWithText (juce::JSON::toString (juce::var (root)));
    }
    currentPresetKey.clear();
    rescanPresets();
    return done;
}

juce::File TugMidiSeqAudioProcessor::getCurrentPresetFile() const
{
    return program >= 1 && program <= (int) myProgram.size() ? myProgram[(size_t) program - 1].sourceFile : juce::File();
}

// The bundle every preset used to share, TugMidiSeqPresets.json in the default
// folder, is split once into <default folder>/Legacy, a file per preset. The
// bundle itself is left as it was, and a marker keeps this from running again.
void TugMidiSeqAudioProcessor::migrateLegacyPresetsIfNeeded()
{
    const auto folder = defaultPresetFolder();
    const auto bundle = folder.getChildFile (legacyBundleName);
    const auto marker = folder.getChildFile (".legacy-split");
    if (! bundle.existsAsFile() || marker.exists()) return;

    const auto legacy = folder.getChildFile ("Legacy");
    for (auto& entry : presetsInFile (bundle))
    {
        auto name = juce::File::createLegalFileName (entry.getProperty ("PresetName", "Preset").toString().trim());
        if (name.isEmpty()) name = "Preset";
        legacy.createDirectory();
        auto dest = legacy.getChildFile (name + ".json").getNonexistentSibling();
        juce::Array<juce::var> one;
        one.add (entry);
        auto* root = new juce::DynamicObject();
        root->setProperty ("Presets", one);
        dest.replaceWithText (juce::JSON::toString (juce::var (root)));
    }
    marker.replaceWithText ("TugMidiSeqPresets.json was split into Legacy/ on " + juce::Time::getCurrentTime().toString (true, true) + "\n");
}
