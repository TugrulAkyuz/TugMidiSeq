/*
 ==============================================================================
 
 GlobalPanel.cpp
 Created: 23 May 2022 9:19:45pm
 Author:  Tuğrul Akyüz
 
 ==============================================================================
 */

#include "GlobalPanel.h"
#include "Satellite.h"


GlobalPanel::GlobalPanel(TugMidiSeqAudioProcessor& p ): audioProcessor (p)
{
    startTimer(100);

    
    addAndMakeVisible(writeButton);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(inBuiltSynthButton);
    addAndMakeVisible(sortedOrFirstEmptySelectButton);
    inBuiltSynthButton.setButtonText("InBSynth");
    inBuiltSynthButton.setClickingTogglesState (true);
    inBuiltSynthButton.setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    inBuiltSynthButton.setColour(TextButton::ColourIds::buttonOnColourId, Colours::orange);
    inBuiltSynthButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    velUsageButton.setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    velUsageButton.setColour(TextButton::ColourIds::textColourOnId, Colours::orange);
    velUsageButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    velUsageButton.setColour(TextButton::ColourIds::buttonOnColourId, Colours::black);
    
    
    sortedOrFirstEmptySelectButton.setButtonText("Sorted");
    sortedOrFirstEmptySelectButton.setClickingTogglesState (true);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::textColourOnId, Colours::orange);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::buttonOnColourId, Colours::black);
    
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.add(new TextButton());
        addAndMakeVisible(randomButton.getLast());
        //randomButton.getLast()->setLookAndFeel(&myLookAndFeel);
        randomButton.getLast()->setButtonText("Rnd "+std::to_string(5-i));
        randomButton.getLast()->setColour(TextButton::ColourIds::textColourOffId, colourarray[4-i]);
        randomButton.getLast()->setColour(TextButton::ColourIds::buttonColourId, Colours::black);
        
        
    }
    
    globalNameLabel.setText("GLOBAL CONTROLS", juce::dontSendNotification);
    globalNameLabel.setColour(juce::Label::ColourIds::textColourId, myTextLabelColour);
    
    addAndMakeVisible(globalNameLabel);
    loopBarlenghtSliderLabel.setFont (juce::Font (12, juce::Font::italic));
    loopBarlenghtSliderLabel.setText("RESYNC BAR", juce::dontSendNotification);
    loopBarlenghtSliderLabel.setColour(juce::Label::ColourIds::textColourId, myTextLabelColour);
    
    ShuffleNameLabel.setFont (juce::Font (12, juce::Font::italic));
    ShuffleNameLabel.setText("SHUFFLE", juce::dontSendNotification);
    ShuffleNameLabel.setColour(juce::Label::ColourIds::textColourId, myTextLabelColour);
    addAndMakeVisible(ShuffleNameLabel);
    
    addAndMakeVisible(loopBarlenghtSliderLabel);
    
    presetCombo.setLookAndFeel(&myLookAndFeel);
    addAndMakeVisible(presetCombo);
    addAndMakeVisible(loopBarlenghtSlider);
    addAndMakeVisible( loopBarCounterLabel);
    addAndMakeVisible( resetButton);
    addAndMakeVisible( velUsageButton);
    addAndMakeVisible(gridAllShuffleSlider);
    
    velUsageButton.setClickingTogglesState (true);
    velUsageButton.setButtonText("Fixed Vel");
    
    addAndMakeVisible(gridAllNumberSlider);
    addAndMakeVisible(gridAllVelSlider);
    addAndMakeVisible(gridAllSpeedCombo);
    addAndMakeVisible(gridAllDurationCombo);
    addAndMakeVisible(gridAllEventSlider);
    
    addAndMakeVisible( gridGridAllShuffleSlider);
    addAndMakeVisible(gridAllDelaySlider);
    gridGridAllShuffleSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllDelaySlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridGridAllShuffleSlider.setRange(-75, 75,1);
    myLookAndFeel2.setdrawRotaryCenterd(true);
    gridGridAllShuffleSlider.setLookAndFeel(&myLookAndFeel2);
    gridAllDelaySlider.setRange(-99,99,1);
    gridAllDelaySlider.setLookAndFeel(&myLookAndFeel2);
    
    gridAllNumberSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllVelSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllEventSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllShuffleSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    gridAllNumberSlider.setRange(1, numOfStep,1);
    gridAllVelSlider.setRange(1, 127,1);
    gridAllEventSlider.setRange(1, 100,1);
    gridAllNumberSlider.setValue(16,juce::dontSendNotification);
    gridAllVelSlider.setValue(100,juce::dontSendNotification);
    gridAllEventSlider.setValue(50,juce::dontSendNotification);

    gridAllShuffleSlider.setColour(juce::Slider::rotarySliderFillColourId,Colours::orange);
    gridAllShuffleSlider.setLookAndFeel(&myLookAndFeel2);
    loopBarlenghtSlider.setColour(juce::Slider::rotarySliderFillColourId,Colours::orange);
    
    //loopBarCounterLabel.setColour(Label::ColourIds::backgroundColourId, Colours::yellow);
    loopBarCounterLabel.setColour(Label::ColourIds::textColourId, Colours::yellowgreen);
    
  //  loopBarlenghtSlider.setSliderStyle (Slider::SliderStyle::LinearBarVertical);
/*
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxOutlineColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    loopBarlenghtSlider.setColour(Slider::textBoxBackgroundColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Label::textWhenEditingColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxBackgroundColourId, Colours::orange);
  */
    deleteButton.setButtonText("Delete");
    deleteButton.setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    deleteButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    writeButton.setButtonText("Save");
    writeButton.setColour(TextButton::ColourIds::textColourOffId, Colours::lightgrey);
    writeButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    
    
    /*
    loopBarlenghtSlider.setVelocityBasedMode (true);
    loopBarlenghtSlider.setVelocityModeParameters (0.4, 1, 0.09, false);
    loopBarlenghtSlider.setRange(1, 32,1);
     */
    loopBarlenghtSlider.setLookAndFeel(&myLookAndFeel);
    loopBarlenghtSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    velUsageButton.onClick = [this](){
        
        for(auto i = 0; i < 5 ;i++)
        if(otherg[i] == 0) return;
        if(velUsageButton.getToggleState() == false)
        {
            for(auto i = 0; i < 5 ;i++)
            otherg[i]->setEnable(true);
            velUsageButton.setButtonText("Fixed Vel");
        }
        else
        {
            for(auto i = 0; i < 5 ;i++)
            otherg[i]->setEnable(false);
            velUsageButton.setButtonText("In Vel");
        }
    };
   // resetButton.setLookAndFeel(&myLookAndFeel);
    resetButton.setColour(TextButton::ColourIds::buttonOnColourId, Colours::orange);
    resetButton.setColour(TextButton::ColourIds::buttonColourId, Colours::black);
    //velUsageButton.setLookAndFeel(&myLookAndFeel);
    resetButton.setButtonText("RESET");
    int i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllSpeedCombo.addItem(s,i);
        i++;
    }
    
    i= 1;
    for(auto s: myNotetUnit)
    {
        gridAllDurationCombo.addItem(s,i);
        i++;
    }
    
    
    gridAllSpeedCombo.setLookAndFeel(&myLookAndFeel);
    gridAllSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::white);
    gridAllDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridAllDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::white);
    
    myLookAndFeel.setColour (ComboBox::textColourId, Colours::white);
    myLookAndFeel.setColour (PopupMenu::backgroundColourId, Colours::black);
    myLookAndFeel.setColour (ComboBox::backgroundColourId, Colours::black);
    
    juce::String tmp_s;
    tmp_s << "GlobalRestncBar";
    loopBarlenghtSliderAttachment = std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, loopBarlenghtSlider);
    
    tmp_s.clear();
    tmp_s << "GlobalInOrFixedVel";
    velUsageButtonAttachment = std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.valueTreeState, tmp_s, velUsageButton);
    
    
    tmp_s.clear();
    tmp_s << valueTreeNames[INBUILTSYNTH];
    inBultSynthButtonAttachment =  std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.valueTreeState, tmp_s, inBuiltSynthButton);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SORTEDORFIRST];
    sortedOrFixedButtonAttachment =  std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.valueTreeState, tmp_s, sortedOrFirstEmptySelectButton);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[SHUFFLE];
    gridAllShuffleSliderAttachment =  std::make_unique <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridAllShuffleSlider);
    
    
    
    if(sortedOrFirstEmptySelectButton.getToggleState() == false)
        sortedOrFirstEmptySelectButton.setButtonText("Sorted");
    else sortedOrFirstEmptySelectButton.setButtonText("FirstIn");
    
    if(velUsageButton.getToggleState() == false)
        velUsageButton.setButtonText("Fixed Vel");
    else  velUsageButton.setButtonText("In Vel");
    
    gridAllSpeedCombo.onChange = [this]
    {
        audioProcessor.setAllValue(valueTreeNames[SPEEED],gridAllSpeedCombo.getSelectedItemIndex());
    };
    gridAllNumberSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue(valueTreeNames[GRIDNUM],gridAllNumberSlider.getValue());
    };
    gridAllDurationCombo.onChange = [this]
    {
        audioProcessor.setAllValue(valueTreeNames[DUR],gridAllDurationCombo.getSelectedItemIndex());
    };
    gridAllVelSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue(valueTreeNames[VEL],gridAllVelSlider.getValue());
    };
    
    gridAllEventSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue( valueTreeNames[EVENT],gridAllEventSlider.getValue());
    };
    
    gridAllShuffleSlider.onValueChange = [this]()
    {
        audioProcessor.setShuffle();
    };
    
    gridGridAllShuffleSlider.onValueChange = [this]
    {
        audioProcessor.setAllValue( valueTreeNames[GRIDSHUFFLE],gridGridAllShuffleSlider.getValue());
    };
    gridAllDelaySlider.onValueChange = [this]
    {
        audioProcessor.setAllValue( valueTreeNames[GRIDDELAY],gridAllDelaySlider.getValue());
    };
    
    for(auto r : randomButton)
    {
        
        r->onClick = [this,r]
        {
            randomButton.indexOf(r);
            audioProcessor.randomizeGrids(4 -randomButton.indexOf(r));
        };
    };
    resetButton.onClick = [this]
    {
        audioProcessor.resetAllParam();
    };
    sortedOrFirstEmptySelectButton.onClick = [this]
    {
        if(sortedOrFirstEmptySelectButton.getToggleState() == false)
            sortedOrFirstEmptySelectButton.setButtonText("Sorted");
        else sortedOrFirstEmptySelectButton.setButtonText("FirstIn");
    };
    
    writeButton.onClick = [this]
    {
        //        String s = "Preset";
        //        s << std::to_string(preset_index_sil);
        //        preset_index_sil++;
        //        audioProcessor.createPrograms(s);
        //        const auto callback = juce::ModalCallbackFunction::create([this](int result) {
        //            if (result == 0) { return; }// result == 0 means you click Cancel
        //            if (result == 1) { /*factoryConfirmed();*/ }// result == 1 means you click OK
        //            });
        
        pwdDialog =  new AlertWindow  ( "Add Preset", "Please enter your preset name", AlertWindow::AlertIconType::NoIcon );
        pwdDialog->addTextEditor( "Preset", "Preset ?" );
        pwdDialog->setColour(AlertWindow::ColourIds::textColourId, Colours::orange);
        pwdDialog->setColour(AlertWindow::ColourIds::backgroundColourId,  Colours::black);
        pwdDialog->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
        pwdDialog->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        pwdDialog->enterModalState(true,ModalCallbackFunction::create([this](int r)
                                                                      {
            if (r)
            {
                auto text = pwdDialog->getTextEditorContents("Preset");
                setPresetMenu(text);
                audioProcessor.writePresetToFileJSON();
                
            }
        }), true);
    };
    int k =  audioProcessor.getNumPrograms();
    if(k == 1 ) return;
    for(auto i = 0  ; i < k ; i++ )
    {
        String s  = audioProcessor.getProgramName(i + 1);
        
        presetCombo.addItem(s, i + 1);
    }
    presetCombo.onChange = [this]
    {
        auto x = presetCombo.getNumItems();
        if(presetCombo.getNumItems() == 0) return;
        x = presetCombo.getSelectedId();
        if(x == 0) return;
        audioProcessor.setCurrentProgram(x);
        
    };
    deleteButton.onClick = [&]
    {
        //        BasicWindow *basicWindow = new BasicWindow("Information", Colours::grey, DocumentWindow::allButtons);
        //
        //        basicWindow->setUsingNativeTitleBar(true);
        //        basicWindow->setContentOwned(new InformationComponent(), true);// InformationComponent is my GUI editor component (the visual editor of JUCE)
        //
        //        basicWindow->centreWithSize(basicWindow->getWidth(), basicWindow->getHeight());
        //        basicWindow->setVisible(true);
        deletePresetMenu();
    };
    associatedComponent.getLookAndFeel().setColour(AlertWindow::ColourIds::backgroundColourId, Colours::black);
    associatedComponent.getLookAndFeel().setColour(AlertWindow::ColourIds::textColourId, Colours::orange);
    
}
void GlobalPanel::paint (juce::Graphics& g)
{
    g.fillAll (  Colour(0xff303030));
    //g.fillAll (  Colours::darkslategrey);
    g.setColour(juce::Colours::orange.withAlpha(0.7f));
    
    g.drawLine(gridAllNumberSlider.getX() -5, 0, gridAllNumberSlider.getX() - 5, getHeight());
    g.drawLine(gridAllDelaySlider.getRight() + 4, 0, gridAllDelaySlider.getRight() +  4, getHeight());
    
}
void GlobalPanel::resized()
{
    
    auto area = getLocalBounds();
    auto xarea =getLocalBounds();
    auto top_area = xarea.removeFromTop(13);
    auto rightarea = area.removeFromRight(200);
 
 
    gridAllDelaySlider.setBounds(area.removeFromRight(50).reduced(3, 5));
    gridGridAllShuffleSlider.setBounds(area.removeFromRight(50).reduced(3, 5));
    gridAllEventSlider.setBounds(area.removeFromRight(50).reduced(3, 5));
    gridAllVelSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    gridAllDurationCombo.setBounds(area.removeFromRight(56).reduced(2,13));
    
    
    gridAllSpeedCombo.setBounds(area.removeFromRight(56).reduced(2,13));
    gridAllNumberSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    
    
    auto ra = area.removeFromRight(80);
    auto ar = ra.removeFromTop(10);
  
    //gridAllShuffleSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    loopBarCounterLabel.setBounds( ra.removeFromRight(30).reduced(3, 5));
    loopBarlenghtSlider.setBounds( ra.removeFromRight(60).reduced(0, 0).translated(10, 0));
    
    ar.removeFromTop(2);
    loopBarlenghtSliderLabel.setBounds(ar.translated(0, 1));
    
    
    velUsageButton.setBounds( area.removeFromRight(70).reduced(3, 10));
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.getUnchecked(i)->setBounds( area.removeFromRight(53).reduced(3, 14));
    }
    globalNameLabel.setBounds(top_area.removeFromLeft(100).reduced(3, 0));
    
    resetButton.setBounds( rightarea.removeFromRight(60).reduced(3, 10));
    presetCombo.setBounds( rightarea.removeFromRight(70).reduced(3, 10));
    rightarea.removeFromTop(5);
    rightarea.removeFromBottom(5);
    writeButton.setBounds( rightarea.removeFromTop(20).reduced(6, 1));
    deleteButton.setBounds( rightarea.reduced(6, 1));
    
    xarea.removeFromBottom(5);
    inBuiltSynthButton.setBounds(xarea.removeFromLeft(70).reduced(4, 5));
    sortedOrFirstEmptySelectButton.setBounds(xarea.removeFromLeft(70).reduced(4, 5));
    
    auto sh = area.removeFromRight(50);
    ShuffleNameLabel.setBounds(sh.removeFromTop(10).translated(0, 1));
    gridAllShuffleSlider.setBounds( sh);
   // ShuffleNameLabel
}


void GlobalPanel::deleteConfirmed()
{
    int k = audioProcessor.getNumPrograms();
    if (k == 1) return;
    int curr_prg = audioProcessor.getCurrentProgram();
    if (curr_prg == 0) return;
    audioProcessor.deletePreset(curr_prg);
    presetCombo.clear();
    k = audioProcessor.getNumPrograms();
    
    for (auto i = 0; i < k; i++)
    {
        String s = audioProcessor.getProgramName(i + 1);
        presetCombo.addItem(s, i + 1);
        
    }
    if ((k + 1) == curr_prg)  curr_prg--;
    presetCombo.setSelectedId(curr_prg);
    audioProcessor.setCurrentProgram(curr_prg);
    audioProcessor.writePresetToFileJSON();
    
}

void GlobalPanel::deletePresetMenu()
{
    
    const auto callback = juce::ModalCallbackFunction::create([this](int result) {
        if (result == 0) { return; }// result == 0 means you click Cancel
        if (result == 1) { deleteConfirmed(); }// result == 1 means you click OK
    });
    //juce::NativeMessageBox::showYesNoBox(juce::AlertWindow::WarningIcon,"Are you sure to delete?", "Are you sure to delete?", this, callback);
    /*AlertWindow *alertWindow = new AlertWindow("Save changes to the current project?",
     "The current project has unsaved changed that will be lost if you don't save them.",
     AlertWindow::InfoIcon);
     */
    AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon,
                                 "The preset will be deleted.",
                                 "Are you sure to delete it?",
                                 "Yes",
                                 "Cancel",
                                 &associatedComponent,
                                 callback);
    
    return;
    // int result = alertWindow->runModalLoop();
    
    
    
}


void GlobalPanel::setPresetMenu(String preset_name)
{
    audioProcessor.createPrograms(preset_name);
    int k =  audioProcessor.getNumPrograms();
    String s  = audioProcessor.getProgramName(k);
    //   myControlPanel->
    presetCombo.clear();
    for (auto i = 0; i < k; i++)
    {
        String s = audioProcessor.getProgramName(i + 1);
        presetCombo.addItem(s, i + 1);
    }
    audioProcessor.setCurrentProgram(k);
    presetCombo.setSelectedId(k);
    
}
