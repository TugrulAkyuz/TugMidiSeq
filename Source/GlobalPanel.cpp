/*
 ==============================================================================
 
 GlobalPanel.cpp
 Created: 23 May 2022 9:19:45pm
 Author:  Tuğrul Akyüz
 
 ==============================================================================
 */

#include "GlobalPanel.h"
#include "Satellite.h"

extern ChangeBroadcaster updateMidiPort;;

GlobalPanel::GlobalPanel(TugMidiSeqAudioProcessor& p ): audioProcessor (p) , velUsageButton("VelButton")
    , presetPrevButton("presetPrev", 0.5f, Theme::accentBright)
    , presetNextButton("presetNext", 0.0f, Theme::accentBright)
{
    startTimer(100);

    updateMidiPort.addChangeListener(this);

    addAndMakeVisible(midiPort);
    addAndMakeVisible(writeButton);
    addAndMakeVisible(deleteButton);
    addAndMakeVisible(openFolderButton);
    addAndMakeVisible(presetPrevButton);
    addAndMakeVisible(presetNextButton);
    addAndMakeVisible(inBuiltSynthButton);
    addAndMakeVisible(sortedOrFirstEmptySelectButton);
    addAndMakeVisible(channelOnButton);
    
    midiPort.setButtonText(audioProcessor.getMidiPortName());

    midiPort.onClick = [this]()
        {


            auto dialog = std::make_unique<ComboBoxDialog>(audioProcessor);

            // Launch the panel in a new window
            juce::DialogWindow::LaunchOptions options;
            options.dialogTitle = "Select Midi Port";
            options.content.setOwned(dialog.release());
            options.componentToCentreAround = getParentComponent();
            options.useNativeTitleBar = true;
            options.resizable = false;

            options.launchAsync();

        };

    inBuiltSynthButton.setButtonText("InBSynth");
    inBuiltSynthButton.setClickingTogglesState (true);
    inBuiltSynthButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    inBuiltSynthButton.setColour(TextButton::ColourIds::buttonOnColourId, Theme::accent);
    inBuiltSynthButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    inBuiltSynthButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    
    channelOnButton.setButtonText("Ch");
    channelOnButton.setClickingTogglesState (true);
    channelOnButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    channelOnButton.setColour(TextButton::ColourIds::buttonOnColourId, Theme::accent);
    channelOnButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    channelOnButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    
    
    
    velUsageButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    sortedOrFirstEmptySelectButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    
    velUsageButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    velUsageButton.setColour(TextButton::ColourIds::textColourOnId, Theme::screen);
    velUsageButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    velUsageButton.setColour(TextButton::ColourIds::buttonOnColourId, Theme::accent);
    
    
    sortedOrFirstEmptySelectButton.setButtonText("Sorted");
    sortedOrFirstEmptySelectButton.setClickingTogglesState (true);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::textColourOnId, Theme::screen);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    sortedOrFirstEmptySelectButton.setColour(TextButton::ColourIds::buttonOnColourId, Theme::accent);
    
    
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.add(new TextButton());
        addAndMakeVisible(randomButton.getLast());
        //randomButton.getLast()->setLookAndFeel(&myLookAndFeel);
        randomButton.getLast()->setButtonText("Rnd "+std::to_string(5-i));
        randomButton.getLast()->setColour(TextButton::ColourIds::textColourOffId, colourarray[4-i]);
        randomButton.getLast()->setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
        randomButton.getLast()->setColour(ComboBox::outlineColourId, Theme::hairline);
        
    }
    midiPort.setColour(TextButton::ColourIds::textColourOffId,Theme::accentBright);
    midiPort.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    midiPort.setColour(ComboBox::outlineColourId, Theme::hairline);

    
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
    gridAllNumberSlider.setRange(2, numOfStep,1);
    gridAllVelSlider.setRange(1, 127,1);
    gridAllEventSlider.setRange(1, 100,1);
    gridAllNumberSlider.setValue(16,juce::dontSendNotification);
    gridAllVelSlider.setValue(100,juce::dontSendNotification);
    gridAllEventSlider.setValue(50,juce::dontSendNotification);

    gridAllShuffleSlider.setColour(juce::Slider::rotarySliderFillColourId,Theme::accent);
    gridAllShuffleSlider.setLookAndFeel(&myLookAndFeel2);
    loopBarlenghtSlider.setColour(juce::Slider::rotarySliderFillColourId,Theme::accent);
    
    //loopBarCounterLabel.setColour(Label::ColourIds::backgroundColourId, Colours::yellow);
    loopBarCounterLabel.setColour(Label::ColourIds::textColourId, Theme::accentBright);
    
  //  loopBarlenghtSlider.setSliderStyle (Slider::SliderStyle::LinearBarVertical);
/*
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxOutlineColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    loopBarlenghtSlider.setColour(Slider::textBoxBackgroundColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Label::textWhenEditingColourId, Colours::orange);
    loopBarlenghtSlider.setColour(Slider::ColourIds::textBoxBackgroundColourId, Colours::orange);
  */
    deleteButton.setButtonText("Delete");
    deleteButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    deleteButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    deleteButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    writeButton.setButtonText("Save");
    writeButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    writeButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    writeButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    openFolderButton.setButtonText("Folder");
    openFolderButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    openFolderButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    openFolderButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    
    
    /*
    loopBarlenghtSlider.setVelocityBasedMode (true);
    loopBarlenghtSlider.setVelocityModeParameters (0.4, 1, 0.09, false);
    loopBarlenghtSlider.setRange(1, 32,1);
     */
    loopBarlenghtSlider.setLookAndFeel(&myLookAndFeel);
    loopBarlenghtSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    channelOnButton.onClick = [this](){
        myGridChangeListener.sendChangeMessage();
    };
    
    velUsageButton.onClick = [this](){
        
        for(auto i = 0; i < 5 ;i++)
        if(otherg[i] == 0) return;
        if(velUsageButton.getToggleState() == false)
        {
            for(auto i = 0; i < 5 ;i++)
            otherg[i]->setEnable(true);
            velUsageButton.setButtonText("Own Vel");
        }
        else
        {
            for(auto i = 0; i < 5 ;i++)
            otherg[i]->setEnable(false);
            velUsageButton.setButtonText("Midi Vel");
        }
    };
   // resetButton.setLookAndFeel(&myLookAndFeel);
    resetButton.setColour(TextButton::ColourIds::buttonOnColourId, Theme::accent);
    resetButton.setColour(TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    resetButton.setColour(TextButton::ColourIds::textColourOffId, Theme::textSecondary);
    resetButton.setColour(ComboBox::outlineColourId, Theme::hairline);
    //velUsageButton.setLookAndFeel(&myLookAndFeel);
    resetButton.setButtonText("Reset");
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
    gridAllSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Theme::textValue);
    gridAllDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridAllDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Theme::textValue);
    
    myLookAndFeel.setColour (ComboBox::textColourId, Theme::textValue);
    myLookAndFeel.setColour (PopupMenu::backgroundColourId, Theme::well);
    myLookAndFeel.setColour (PopupMenu::textColourId, Theme::textPrimary);
    myLookAndFeel.setColour (PopupMenu::highlightedBackgroundColourId, Theme::accent);
    myLookAndFeel.setColour (PopupMenu::highlightedTextColourId, Theme::screen);
    myLookAndFeel.setColour (ComboBox::backgroundColourId, Theme::well);
    
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
    tmp_s << valueTreeNames[CHANNON];
    channelOnButtonAttachment =  std::make_unique <AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.valueTreeState, tmp_s, channelOnButton);
    
    
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
        pwdDialog->setColour(AlertWindow::ColourIds::textColourId, Theme::accent);
        pwdDialog->setColour(AlertWindow::ColourIds::backgroundColourId,  Theme::panel);
        pwdDialog->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
        pwdDialog->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
        pwdDialog->enterModalState(true,ModalCallbackFunction::create([this](int r)
                                                                      {
            if (r)
            {
                auto text = pwdDialog->getTextEditorContents("Preset");
                setPresetMenu(text);   // createPrograms already writes the single file
            }
        }), true);
    };

    openFolderButton.onClick = [this]
    {
        folderChooser = std::make_shared<juce::FileChooser>(
            "Choose Preset Folder  (select folder or any preset inside it)",
            audioProcessor.presetFolder, "*.json", true);
        folderChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                   juce::FileBrowserComponent::canSelectDirectories |
                                   juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (!result.exists()) return;
                const juce::File folder = result.isDirectory() ? result
                                                               : result.getParentDirectory();
                audioProcessor.setPresetFolder(folder);
                refreshPresetList();
            });
    };

    presetPrevButton.onClick = [this] { stepPreset(-1); };
    presetNextButton.onClick = [this] { stepPreset(+1); };

    // NB: onChange / delete handlers are assigned unconditionally — even when
    // the plugin opens with no presets, ones added later (folder pick, Save)
    // must still load on click.
    presetCombo.onChange = [this]
    {
        if(presetCombo.getNumItems() == 0) return;
        auto x = presetCombo.getSelectedId();
        if(x == 0) return;
        audioProcessor.setCurrentProgram(x);

    };
    deleteButton.onClick = [&]
    {
        deletePresetMenu();
    };

    refreshPresetList();
    // restore the previously selected preset in the combo (GUI reopen /
    // project reload) without re-triggering a program load
    const int curr = audioProcessor.getCurrentProgram();
    if (curr > 0 && curr <= presetCombo.getNumItems())
        presetCombo.setSelectedId(curr, juce::dontSendNotification);
    associatedComponent.getLookAndFeel().setColour(AlertWindow::ColourIds::backgroundColourId, Theme::panel);
    associatedComponent.getLookAndFeel().setColour(AlertWindow::ColourIds::textColourId, Theme::accent);
    
}
void GlobalPanel::paint (juce::Graphics& g)
{
    g.fillAll (Theme::section);

    // subtle column separators, hairline weight (no more hard grey rules)
    g.setColour (Theme::hairline.withAlpha (0.55f));
    g.drawLine (gridAllNumberSlider.getX() - 5.0f, 6.0f, gridAllNumberSlider.getX() - 5.0f, getHeight() - 6.0f, 1.0f);
    g.drawLine (channelOnButton.getRight() + 4.0f, 6.0f, channelOnButton.getRight() + 4.0f, getHeight() - 6.0f, 1.0f);
}
void GlobalPanel::resized()
{
    
    auto area = getLocalBounds();
    auto xarea =getLocalBounds();
    auto top_area = xarea.removeFromTop(13);
    auto rightarea = area.removeFromRight(200);


    midiPort.setButtonText(audioProcessor.getMidiPortName());
    channelOnButton.setBounds( area.removeFromRight(40).reduced(0, 13));
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
    auto d = area.removeFromRight(260);
    auto dd = d.removeFromBottom(getHeight()/2);
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.getUnchecked(i)->setBounds(d.removeFromRight(53).reduced(3, 2));
    }
    globalNameLabel.setBounds(top_area.removeFromLeft(100).reduced(3, 0));
    auto r = randomButton.getUnchecked(2);

    midiPort.setBounds(dd.reduced(2, 2));
    resetButton.setBounds( rightarea.removeFromRight(48).reduced(2, 15));

    // preset strip, two rows:
    //   row 1:  ◀  [preset combo]  ▶
    //   row 2:  [Folder] [Save] [Delete]
    auto presetArea = rightarea.reduced(2, 2);
    auto row1 = presetArea.removeFromTop(presetArea.getHeight() / 2);
    presetPrevButton.setBounds(row1.removeFromLeft(14).withSizeKeepingCentre(10, 10));
    presetNextButton.setBounds(row1.removeFromRight(14).withSizeKeepingCentre(10, 10));
    presetCombo.setBounds(row1.reduced(0, 2));

    auto row2 = presetArea;
    const int bw = row2.getWidth() / 3;
    openFolderButton.setBounds(row2.removeFromLeft(bw).reduced(2, 2));
    writeButton.setBounds(row2.removeFromLeft(bw).reduced(2, 2));
    deleteButton.setBounds(row2.reduced(2, 2));
    
    xarea.removeFromBottom(5);
    inBuiltSynthButton.setBounds(xarea.removeFromLeft(70).reduced(4, 5));
    sortedOrFirstEmptySelectButton.setBounds(xarea.removeFromLeft(70).reduced(4, 5));
    
    auto sh = area.removeFromRight(50);
    ShuffleNameLabel.setBounds(sh.removeFromTop(10).translated(0, 1));
    gridAllShuffleSlider.setBounds( sh);
   // ShuffleNameLabel
}


void GlobalPanel::refreshPresetList()
{
    presetCombo.clear(NotificationType::dontSendNotification);
    const int k = audioProcessor.getNumPrograms();
    for (auto i = 0; i < k; i++)
    {
        String s = audioProcessor.getProgramName(i + 1);
        if (s.isEmpty()) continue;
        presetCombo.addItem(s, i + 1);
    }
}

void GlobalPanel::stepPreset(int delta)
{
    const int n = presetCombo.getNumItems();
    if (n == 0) return;
    int idx = presetCombo.getSelectedItemIndex();
    if (idx < 0) idx = (delta > 0 ? 0 : n - 1);   // no selection: start from an end
    else         idx = (idx + delta + n) % n;      // wraps around at both ends
    presetCombo.setSelectedItemIndex(idx);         // onChange -> loads the program
}

void GlobalPanel::deleteConfirmed()
{
    int k = audioProcessor.getNumPrograms();
    if (k == 0) return;
    int curr_prg = audioProcessor.getCurrentProgram();
    if (curr_prg == 0) return;
    audioProcessor.deletePreset(curr_prg);
    refreshPresetList();
    k = audioProcessor.getNumPrograms();
    if ((k + 1) == curr_prg)  curr_prg--;
    presetCombo.setSelectedId(curr_prg);
    audioProcessor.setCurrentProgram(curr_prg);
    // harmless & needed: only rewrites bundle residents, so a deleted bundle
    // preset actually drops out of the legacy file
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
    refreshPresetList();
    audioProcessor.setCurrentProgram(k);
    presetCombo.setSelectedId(k);

}
