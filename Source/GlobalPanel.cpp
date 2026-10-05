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
   #if JucePlugin_IsMidiEffect
    inBuiltSynthButton.setVisible (false);   // the MIDI FX build has no audio out to play it on
   #endif
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
    globalNameLabel.setFont (Theme::labelFont (10.5f));
    globalNameLabel.setMinimumHorizontalScale (1.0f);
    
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
    myLookAndFeel.setColour (PopupMenu::headerTextColourId, Theme::accentBright);
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
            const int lane = 4 - randomButton.indexOf(r);
            audioProcessor.undoableEdit ([&] { audioProcessor.randomizeGrids (lane); });
        };
    };
    resetButton.onClick = [this]
    {
        audioProcessor.undoableEdit ([this] { audioProcessor.resetAllParam(); });
    };
    sortedOrFirstEmptySelectButton.onClick = [this]
    {
        if(sortedOrFirstEmptySelectButton.getToggleState() == false)
            sortedOrFirstEmptySelectButton.setButtonText("Sorted");
        else sortedOrFirstEmptySelectButton.setButtonText("FirstIn");
    };
    
    writeButton.onClick      = [this] { showSaveDialog(); };
    openFolderButton.onClick = [this] { showFolderMenu(); };

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
        audioProcessor.undoableEdit ([this, x] { audioProcessor.setCurrentProgram (x); });
        // the menu marks the folders the chosen preset is in: rebuild it once the combo is done
        juce::Component::SafePointer<GlobalPanel> safe (this);
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->refreshPresetList(); });
    };
    deleteButton.onClick = [&]
    {
        deletePresetMenu();
    };

    refreshPresetList();   // also shows the current preset, without loading it again
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
    channelOnButton.setBounds( area.removeFromRight(LaneLayout::chan).reduced(0, 13));   // columns as in the lanes
    gridAllDelaySlider.setBounds(area.removeFromRight(LaneLayout::delay).reduced(3, 5));
    gridGridAllShuffleSlider.setBounds(area.removeFromRight(LaneLayout::shuffle).reduced(3, 5));
    gridAllEventSlider.setBounds(area.removeFromRight(LaneLayout::event).reduced(3, 5));
    gridAllVelSlider.setBounds( area.removeFromRight(LaneLayout::vel).reduced(3, 5));
    gridAllDurationCombo.setBounds(area.removeFromRight(LaneLayout::duration).reduced(2,13));   // matches the lane combos


    gridAllSpeedCombo.setBounds(area.removeFromRight(LaneLayout::speed).reduced(2,13));
    gridAllNumberSlider.setBounds( area.removeFromRight(LaneLayout::steps).reduced(3, 5));
    
    
    auto ra = area.removeFromRight(80);
    auto ar = ra.removeFromTop(10);
  
    //gridAllShuffleSlider.setBounds( area.removeFromRight(50).reduced(3, 5));
    loopBarCounterLabel.setBounds( ra.removeFromRight(30).reduced(3, 5));
    loopBarlenghtSlider.setBounds( ra.removeFromRight(60).reduced(0, 0).translated(10, 0));
    
    ar.removeFromTop(2);
    loopBarlenghtSliderLabel.setBounds(ar.translated(0, 1));
    
    
    velUsageButton.setBounds( area.removeFromRight(70).reduced(3, 10));
    // 20px narrower since the speed/duration combos grew, so the shuffle knob
    // left of it stays clear of the InBSynth / Sorted buttons
    auto d = area.removeFromRight(240);
    auto dd = d.removeFromBottom(getHeight()/2);
    for(int i = 0 ; i < 5 ; i++)
    {
        randomButton.getUnchecked(i)->setBounds(d.removeFromRight(48).reduced(3, 2));
    }
    globalNameLabel.setBounds(top_area.removeFromLeft(140).reduced(3, 0));   // room for the whole caption
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


// The preset menu mirrors the library: each sub-folder a submenu (as deep as
// they go), then the folder's own presets. Item ids are the program numbers,
// which run in that same order, so the arrows step through it as listed. The
// folders holding the current preset are ticked.
void GlobalPanel::refreshPresetList()
{
    presetCombo.clear (juce::dontSendNotification);
    struct Node { std::map<juce::String, Node> children; juce::StringArray order; std::vector<int> items; };
    Node root;
    const int n = audioProcessor.getNumPrograms();
    for (int i = 1; i <= n; i++)
    {
        auto* node = &root;
        juce::StringArray parts;
        parts.addTokens (audioProcessor.presetCategory (i), "/", "");
        parts.removeEmptyStrings();
        for (auto& part : parts)
        {
            if (node->children.count (part) == 0) node->order.add (part);
            node = &node->children[part];
        }
        node->items.push_back (i);
    }

    const int selected = audioProcessor.getCurrentProgram();
    std::function<bool (const Node&, juce::PopupMenu&)> build = [&] (const Node& node, juce::PopupMenu& menu)
    {
        bool holdsSelected = false;
        for (auto& name : node.order)
        {
            juce::PopupMenu sub;
            const bool here = build (node.children.at (name), sub);
            menu.addSubMenu (name, sub, true, nullptr, here);
            holdsSelected |= here;
        }
        for (int i : node.items)
        {
            menu.addItem (i, audioProcessor.getProgramName (i));
            holdsSelected |= i == selected;
        }
        return holdsSelected;
    };
    build (root, *presetCombo.getRootMenu());
    if (n == 0)
        presetCombo.setTextWhenNothingSelected ("No presets");
    else
        presetCombo.setTextWhenNothingSelected ("Presets");
    if (selected > 0 && selected <= n)
        presetCombo.setSelectedId (selected, juce::dontSendNotification);
}

void GlobalPanel::stepPreset (int delta)
{
    const int n = audioProcessor.getNumPrograms();
    if (n == 0) return;
    const int id = presetCombo.getSelectedId();
    const int next = id < 1 ? (delta > 0 ? 1 : n) : (id - 1 + delta + n) % n + 1;   // wraps at both ends
    presetCombo.setSelectedId (next);   // onChange -> loads the program
}

void GlobalPanel::deleteConfirmed()
{
    if (! audioProcessor.deleteCurrentPreset()) return;
    refreshPresetList();
}

void GlobalPanel::deletePresetMenu()
{
    const int curr = audioProcessor.getCurrentProgram();
    if (curr < 1)
    {
        AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon, "Delete preset", "Choose a preset first.", "OK", &associatedComponent);
        return;
    }
    AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon,
                                  "Delete \"" + audioProcessor.getProgramName (curr) + "\"?",
                                  "Its file is deleted:\n" + audioProcessor.getCurrentPresetFile().getFullPathName(),
                                  "Delete", "Cancel", &associatedComponent,
                                  juce::ModalCallbackFunction::create ([this] (int result) { if (result == 1) deleteConfirmed(); }));
}

// Name and folder (relative to the preset folder, "Bass/Dark", empty for the
// top); the folder starts as the current preset's.
void GlobalPanel::showSaveDialog()
{
    const int curr = audioProcessor.getCurrentProgram();
    auto* aw = new AlertWindow ("Save preset", "Folder: inside the preset folder, e.g. Bass/Dark. Leave it empty for the top.",
                                AlertWindow::NoIcon);
    aw->setColour (AlertWindow::textColourId, Theme::textPrimary);
    aw->setColour (AlertWindow::backgroundColourId, Theme::panel);
    aw->addTextEditor ("name", curr > 0 ? audioProcessor.getProgramName (curr) : juce::String ("My Preset"), "Name");
    aw->addTextEditor ("folder", audioProcessor.presetCategory (curr), "Folder");
    aw->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    aw->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    aw->enterModalState (true, ModalCallbackFunction::create ([this, aw] (int r)
    {
        if (r != 1) return;
        const auto name = aw->getTextEditorContents ("name"), folder = aw->getTextEditorContents ("folder");
        juce::Component::SafePointer<GlobalPanel> safe (this);
        auto save = [safe, name, folder]
        {
            if (safe == nullptr) return;
            safe->audioProcessor.savePresetAs (name, folder);
            safe->refreshPresetList();
        };
        const auto file = audioProcessor.presetFileFor (name, folder);
        if (file.existsAsFile())
            AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Replace \"" + file.getFileNameWithoutExtension() + "\"?",
                                          "A preset with that name is already in this folder.", "Replace", "Cancel",
                                          &associatedComponent, ModalCallbackFunction::create ([save] (int ok) { if (ok == 1) save(); }));
        else
            save();
    }), true);
}

void GlobalPanel::showFolderMenu()
{
    enum { chooseId = 1, defaultId, openId };
    const auto folder = audioProcessor.presetFolder;
    const auto def = TugMidiSeqAudioProcessor::defaultPresetFolder();
    juce::PopupMenu m;
    m.setLookAndFeel (&myLookAndFeel);
    m.addSectionHeader ("Preset folder");
    m.addItem (-1, folder.getFullPathName(), false);
    m.addSeparator();
    m.addItem (chooseId, "Choose folder...");
    m.addItem (defaultId, "Use the default folder", folder != def);
   #if JUCE_MAC
    m.addItem (openId, "Show in Finder");
   #elif JUCE_WINDOWS
    m.addItem (openId, "Show in Explorer");
   #else
    m.addItem (openId, "Open the folder");
   #endif
    juce::Component::SafePointer<GlobalPanel> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&openFolderButton),
                     [safe, def] (int id)
    {
        if (safe == nullptr) return;
        auto& proc = safe->audioProcessor;
        if (id == defaultId) { def.createDirectory(); proc.setPresetFolder (def); safe->refreshPresetList(); }
        if (id == openId)    proc.presetFolder.startAsProcess();
        if (id == chooseId)
        {
            // JUCE's own browser, not the system one: the Windows and Linux system
            // dialogs can't show the presets while picking a folder (and zenity
            // can't pick a folder at all when files are asked for too)
            safe->folderChooser = std::make_shared<juce::FileChooser> ("Choose the preset folder", proc.presetFolder, "*.json", false);
            safe->folderChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                [safe] (const juce::FileChooser& fc)
                {
                    if (safe == nullptr) return;
                    const auto r = fc.getResult();
                    if (! r.exists()) return;
                    safe->audioProcessor.setPresetFolder (r.isDirectory() ? r : r.getParentDirectory());
                    safe->refreshPresetList();
                });
        }
    });
}
