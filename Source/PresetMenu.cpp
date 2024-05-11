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
        
    }
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
    
    // mySynth.clearSounds();
    
    //initPrepareValue();
    
    myGridChangeListener.sendChangeMessage();
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
                
                
                tmp_s.clear();
                tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
                v = myProgram.at(p).gridVelArr[i][j];
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
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[EVENT]<< i;
            v = myProgram.at(p).gridsEvent[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[GRIDSHUFFLE]<< i;
            v = myProgram.at(p).gridsShuffle[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[GRIDDELAY]<< i;
            v = myProgram.at(p).gridsDelay[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[GRIDMIDIROUTE]<< i;
            v = myProgram.at(p).gridsMidiRoute[i];
            newObj.getDynamicObject()->setProperty(tmp_s,v);
        }
        tmp_s.clear();
        tmp_s << valueTreeNames[GLOBALRESTBAR];
        auto v  = myProgram.at(p).globalResyncBar;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[GLOABLINORFIXVEL];
        v = myProgram.at(p).GlobalInOrFixedVel;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[INBUILTSYNTH];
        v = myProgram.at(p).inBuiltSynth;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[SORTEDORFIRST];
        v = myProgram.at(p).sortedOrFirst;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
        tmp_s.clear();
        tmp_s << valueTreeNames[SHUFFLE];
        v  = myProgram.at(p).channelOn;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
    
        tmp_s.clear();
        tmp_s << valueTreeNames[CHANNON];
        v  = myProgram.at(p).shuffle;
        newObj.getDynamicObject()->setProperty(tmp_s,v);
        
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
                
                tmp_s.clear();
                tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
                if (preset.hasProperty(tmp_s))
                   v = preset.getProperty(tmp_s, var());
                else
                    v =  90;
                p.gridVelArr[i][j] = v;
                
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
                // Assign a specific value if the property doesn't exist
                v =   valueTreeState.getParameter(tmp_s)->convertFrom0to1(  valueTreeState.getParameter(tmp_s)->getDefaultValue()); // Change defaultValueForMissingProperty to your desired value
            }
            p.gridsMidiRoute[i] = v;
            
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
        v = preset.getProperty(tmp_s, var());
        p.channelOn = v;
        
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
            
            tmp_s.clear();
            tmp_s <<valueTreeNames[VELGRIDBUTTON]<< i << j;
            
            paramProg.gridVelArr[i][j] = *valueTreeState.getRawParameterValue(tmp_s);
            
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
    
    myProgram.push_back(paramProg);
    writePresetToFileJSON();
}


void TugMidiSeqAudioProcessor::deletePreset(int index)
{
    if(index == 0) return;
    myProgram.erase(myProgram.begin() + index -1);
    
    
}
