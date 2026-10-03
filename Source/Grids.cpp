/*
  ==============================================================================

    Grids.cpp
    Created: 13 May 2022 3:35:51pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/


#include "Grids.h"
 



Grids::Grids(TugMidiSeqAudioProcessor& p,int line)  : audioProcessor (p) , stepArrow("",
                                                                                   0.0f,
                                                                                   juce::Colours::orange)
{
    myLine = line;
    step = 0;
    myLineLabel.setText(std::to_string(myLine + 1), juce::NotificationType::dontSendNotification);
    myLineLabel.setColour(juce::Label::ColourIds::textColourId, colourarray[myLine]);
    myLineLabel.setJustificationType(Justification::right);
    myLineLabel.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    myLineLabel.addMouseListener (this, false);   // -> Grids::mouseDown: lane menu

    addAndMakeVisible(myLineLabel);
    addAndMakeVisible(octaveSlider);
    addAndMakeVisible(stepArrow);
    addAndMakeVisible(gridNumberSlider);
    addAndMakeVisible(gridSpeedCombo);
    addAndMakeVisible(gridMidiRouteCombo);
    
    
    octaveSlider.setSliderStyle(juce::Slider::LinearVertical);
    octaveSlider.setColour(Slider::textBoxOutlineColourId , juce::Colours::black.withAlpha(0.0f));
    //octaveSlider.gette
    octaveSlider.setLookAndFeel(&myLookAndFeel);
    // no number box: the value reads from the ticks drawn beside the slider,
    // and a bubble shows it while dragging (the room went to the spread knob)
    octaveSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    octaveSlider.setPopupDisplayEnabled (true, true, nullptr);
    octaveSlider.setColour(Slider::textBoxTextColourId,  Theme::textValue);
    addAndMakeVisible(midiInNote);
    midiInNote.setLookAndFeel (&myLookAndFeel);   // the panel's button style
    midiInNote.setColour(juce::TextButton::ColourIds::buttonColourId, Theme::surfaceAlt);
    midiInNote.setColour(juce::TextButton::textColourOffId, Theme::textSecondary);

    gridSpeedCombo.setLookAndFeel(&myLookAndFeel);
    gridMidiRouteCombo.setLookAndFeel(&myLookAndFeel);
    //gridSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
    addAndMakeVisible(gridDurationCombo);
    gridDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::lightgrey);
    gridDurationCombo.getLookAndFeel().setColour (PopupMenu::backgroundColourId, Colours::black);
    gridDurationCombo.getLookAndFeel().setColour (ComboBox::backgroundColourId, Colours::black);
    // popup styling shared by the combos and the pads' condition menu
    myLookAndFeel.setColour (PopupMenu::textColourId, Theme::textPrimary);
    myLookAndFeel.setColour (PopupMenu::headerTextColourId, Theme::accentBright);
    myLookAndFeel.setColour (PopupMenu::highlightedBackgroundColourId, Theme::accent);
    myLookAndFeel.setColour (PopupMenu::highlightedTextColourId, Theme::screen);

    subGrids =  std::make_unique<SubGrids>(*this,audioProcessor,myLine,SELECTEDGRID);
    subGrids2 =  std::make_unique<SubGrids>(*this,audioProcessor,myLine,FOLLOWGRID);
    addAndMakeVisible(subGrids.get());
    addAndMakeVisible(subGrids2.get());
    

    
    //gridNumberSlider.setSliderStyle(juce::Slider::Rotary);
    gridNumberSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    //gridNumberSlider.setValue(16);
    addAndMakeVisible(gridVelSlider);
    //gridVelSlider.setSliderStyle(juce::Slider::Rotary);
    gridVelSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    addAndMakeVisible(gridEventSlider);
    gridEventSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    addAndMakeVisible(gridShuffleSlider);
    gridShuffleSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
    addAndMakeVisible(gridDelaySlider);
    gridDelaySlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    
   // gridVelSlider.setValue(90);
    juce::String  tmp_s;
    for (int i = 0; i < numOfStep; ++i)
    {
        addAndMakeVisible (buttons.add (new MultiStateButton (audioProcessor , myLine, i)));
        buttons.getLast()->setOwnerGrid(this);
        buttons.getLast()->setClickingTogglesState(true);
        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
        buttons.getLast()->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::orange);
        buttons.getLast()->setConnectedEdges(30);
        buttons.getLast()->onClick = [this]()
        {
            int i = 0;
            for(MultiStateButton * b : buttons)
            {
    
            }
            subGrids->rP();
        };
        tmp_s.clear();
        tmp_s << "block" << line << i;
        MultiStateButton *  st = buttons.getLast();
       // buttonAttachmentArray.add (new AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.valueTreeState, tmp_s,*st));
        buttonAttachmentArray.add( std::make_unique  <MultiStateButtonAttachment>(audioProcessor.valueTreeState, tmp_s,*st));
        tmp_s.clear();
        tmp_s << valueTreeNames[EVENT] << line;
        
        audioProcessor.valueTreeState.addParameterListener(tmp_s,(juce::AudioProcessorValueTreeState::Listener *)(st));
       
        auto v = audioProcessor.valueTreeState.getParameter(tmp_s)->getValue();
      
        st->parameterChanged(tmp_s, 100*v);
        
       
        tmp_s.clear();
        tmp_s <<valueTreeNames[VELGRIDBUTTON]<< line << i;
        /*
        addAndMakeVisible (velButtons.add (new CustomRoratySlider ()));
        CustomRoratySlider *  st_vel = velButtons.getLast();
        st_vel->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        st_vel->setLookAndFeel(&myLookAndFeel);
        velButtonAttachmentArray.add( std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s,*st_vel));
         */
    }
  
   // dynamic_cast<<#type#>>(expression)
    myLookAndFeel2.setdrawRotaryCenterd(true);
    gridShuffleSlider.setLookAndFeel(&myLookAndFeel2);
    gridDelaySlider.setLookAndFeel(&myLookAndFeel2);
    
    gridNumberSlider.setColour(juce::Slider::rotarySliderFillColourId,colourarray[myLine]);
    gridVelSlider.setColour(juce::Slider::rotarySliderFillColourId,colourarray[myLine]);
    gridEventSlider.setColour(juce::Slider::rotarySliderFillColourId,colourarray[myLine]);
    gridShuffleSlider.setColour(juce::Slider::rotarySliderFillColourId,colourarray[myLine]);
    gridDelaySlider.setColour(juce::Slider::rotarySliderFillColourId,colourarray[myLine]);
    //gridNumberSlider.setRange(1, numOfStep,1);
    //gridVelSlider.setRange(1, 127,1);
    tmp_s.clear();
    tmp_s << valueTreeNames[GRIDSHUFFLE] << line;
    
    audioProcessor.valueTreeState.addParameterListener(tmp_s, this);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GRIDNUM] <<line;
    gridNumberSliderAttachment = std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridNumberSlider);
    tmp_s.clear();
    tmp_s << valueTreeNames[VEL]<<line;
    gridVelSliderAttachment = std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridVelSlider);
    tmp_s.clear();
    tmp_s << valueTreeNames[OCTAVE]<<line;
    octaveAttachment = std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, octaveSlider);

    addAndMakeVisible (spreadKnob);
    spreadKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    spreadKnob.setColour (juce::Slider::rotarySliderFillColourId, colourarray[myLine]);
    spreadAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.valueTreeState, valueTreeNames[SPREAD] + juce::String (line), spreadKnob);
    spreadKnob.setLookAndFeel (&myLookAndFeel2);   // centred arc: - strums down, + up
    spreadKnob.setShowRangeLabels (false);         // "-100" / "100" don't fit under a 34 px knob
    spreadKnob.onRightClick = [this] { showStrumShape(); };
    spreadKnob.setEnabled (audioProcessor.getPlayMode (myLine) >= PlayStrum);   // the timer keeps it in step
    
    tmp_s.clear();
    tmp_s << valueTreeNames[EVENT] << line;
    gridEventSliderAttachment =  std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridEventSlider);
    gridEventSlider.setEnabled (audioProcessor.laneHasEventStep (myLine));   // the timer keeps it in step
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GRIDSHUFFLE] << line;
    gridShuffleSliderAttachment =  std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridShuffleSlider);
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GRIDDELAY] << line;
    gridDelaySliderAttachment =  std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridDelaySlider);
    
    gridDelaySlider.onValueChange = [this]()
    {
        resized();
    };
    
    gridNumberSlider.onValueChange = [this]()
    {
        resized();
        subGrids->rP();
    };
    gridVelSlider.onValueChange = [this]()
    {
//         step = gridVelSlider.getValue() -1;
//        resized();
    };
    startTimer(20);

    
    
    int i= 1;
    for(auto s: myNotetUnit)
    {
        gridSpeedCombo.addItem(s,i);
        i++;
    }
    tmp_s.clear();
    tmp_s << valueTreeNames[SPEEED]<<line;
    comBoxSpeedAtaachment = std::make_unique <AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.valueTreeState, tmp_s, gridSpeedCombo);

     i= 1;
    for(auto s: myNotetUnit)
    {
        gridDurationCombo.addItem(s,i);
        i++;
    }
    tmp_s.clear();
    tmp_s << valueTreeNames[DUR] <<line;
    comBoxDurationAtaachment =  std::make_unique <AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.valueTreeState, tmp_s, gridDurationCombo);

    octaveSlider.setColour (Slider::ColourIds::backgroundColourId, Theme::well);
    octaveSlider.setColour (Slider::ColourIds::trackColourId, colourarray[myLine].withAlpha (0.55f));
    octaveSlider.setColour (Slider::ColourIds::thumbColourId, colourarray[myLine]);
    //Slider::ColourIds::thumbColourId
    //octaveSlider.setRange(-2, 2,1);
   // octaveSlider.setValue(0);
    
    for(i = 1 ; i <= 16; i++)
    {
        gridMidiRouteCombo.addItem(std::to_string(i),i);
    }
    
    tmp_s.clear();
    tmp_s << valueTreeNames[GRIDMIDIROUTE]<<line;
    gridMidiRouteAttachment = std::make_unique <AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.valueTreeState, tmp_s, gridMidiRouteCombo);


    
   
    audioProcessor.valueTreeState.addParameterListener(valueTreeNames[SHUFFLE], this);
 
    myGridChangeListener.addChangeListener(this);
    
    // click: solo; shift+click: mute
    midiInNote.onClick = [this]()
    {
        if (juce::ModifierKeys::getCurrentModifiers().isShiftDown())
            audioProcessor.setLaneMute (myLine, ! audioProcessor.isLaneMuted (myLine));
        else
            audioProcessor.setGridSolo(myLine);
    };
    
}
void Grids::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // alternating raised lane surface
    g.setColour (myLine % 2 == 0 ? Theme::surface : Theme::surfaceAlt);
    g.fillRect (bounds);

    // lane identity tab on the far left
    g.setColour (colourarray[myLine].withAlpha (0.85f));
    g.fillRect (0.0f, 2.0f, 3.0f, bounds.getHeight() - 4.0f);

    // play direction, under the lane number
    drawDirectionGlyph (g, directionArea.toFloat().withSizeKeepingCentre (11.0f, 9.0f)
                                                 .withX ((float) directionArea.getRight() - 12.0f));

    // waiting for its loop to end before moving to another pattern slot: that slot's letter, blinking
    if (audioProcessor.isLaneWaitingForSlot (myLine) && (juce::Time::getMillisecondCounter() / 250) % 2 == 0)
        Theme::drawCaption (g, slotNames[audioProcessor.getRequestedSlot()],
                            directionArea.withTrimmedLeft (4).withWidth (10), juce::Justification::centred,
                            Theme::accentBright, 10.0f);

    // the lane's note was picked with the mouse (all of them, for an ALL lane):
    // the lane number sits in a brass ring, as on the on-screen keyboard
    if (laneSource == NoteFromClick)
    {
        const auto font = myLineLabel.getFont();
        const auto text = myLineLabel.getText();
        const auto border = myLineLabel.getBorderSize();
        const auto lb = myLineLabel.getBounds().toFloat();
        const float textW = juce::GlyphArrangement::getStringWidth (font, text);
        const float cx = lb.getRight() - (float) border.getRight() - textW * 0.5f;
        const float d = 17.0f;
        g.setColour (Theme::accentBright);
        g.drawEllipse (juce::Rectangle<float> (d, d).withCentre ({ cx, lb.getCentreY() }), 1.6f);
    }

    // chord / strum lanes: stacked note bars left of the lane number, staggered for strums
    const int play = audioProcessor.getPlayMode (myLine);
    if (play != PlayVoice)
    {
        g.setColour (colourarray[myLine].withAlpha (0.95f));
        const float x = 4.5f, y = (float) myLineLabel.getBounds().getCentreY() - 5.0f;
        for (int k = 0; k < 3; k++)
        {
            float shift = 0.0f;
            const int spread = audioProcessor.getSpread (myLine);
            if (play == PlayStrum && spread > 0) shift = (float) (2 - k) * 1.5f;   // low note (bottom) first
            if (play == PlayStrum && spread < 0) shift = (float) k * 1.5f;
            if (play == PlayStrumUpDown && spread != 0) shift = k == 1 ? 1.5f : 0.0f;
            g.fillRect (x + shift, y + (float) k * 4.0f, 4.0f, 2.0f);
        }
    }

    // hairline separators (row bottom, top for the first lane, control columns)
    g.setColour (Theme::hairline.withAlpha (0.7f));
    g.drawLine (0, bounds.getBottom() - 0.5f, bounds.getRight(), bounds.getBottom() - 0.5f, 1.0f);
    if (myLine == 4)
        g.drawLine (0, 0.5f, bounds.getRight(), 0.5f, 1.0f);

    g.setColour (Theme::hairline.withAlpha (0.45f));
    g.drawLine (octaveSlider.getRight() + 2.0f, 5.0f, octaveSlider.getRight() + 2.0f, getHeight() - 5.0f, 1.0f);

    // octave ticks (-2..+2) beside the slider, the 0 one longer
    g.setColour (Theme::textDim);
    for (int v = -2; v <= 2; v++)
    {
        const float y = (float) octaveSlider.getY() + (float) octaveSlider.getPositionOfValue ((double) v);
        const float len = v == 0 ? 4.0f : 2.5f;
        g.drawLine ((float) octaveSlider.getX(), y, (float) octaveSlider.getX() + len, y, 1.0f);
    }
    g.drawLine (gridNumberSlider.getX() - 5.0f, 5.0f, gridNumberSlider.getX() - 5.0f, getHeight() - 5.0f, 1.0f);

    // MIDI-activity LED
    float lx = gridMidiRouteCombo.getX() + 7.0f;
    if (audioProcessor.midiState[myLine] == true)
    {
        g.setColour (Colour (0xff86e0a0).withAlpha (0.3f));
        g.fillEllipse (lx - 2.0f, 0.0f, 9.0f, 9.0f);        // glow
        g.setColour (Colour (0xff86e0a0));
        g.fillEllipse (lx, 2.0f, 5.0f, 5.0f);
    }
    else
    {
        g.setColour (Theme::hairline);
        g.fillEllipse (lx, 2.0f, 5.0f, 5.0f);
    }

}

// Forward is the default, so it's drawn faint; any other direction stands out.
void Grids::drawDirectionGlyph (juce::Graphics& g, juce::Rectangle<float> r) const
{
    const int dir = audioProcessor.getDirection (myLine);
    g.setColour (colourarray[myLine].withAlpha (dir == DirForward ? 0.35f : 0.95f));

    const float cy = r.getCentreY(), head = 3.0f;
    auto arrowHead = [&] (float tipX, float dirSign)
    {
        juce::Path p;
        p.addTriangle (tipX, cy, tipX - dirSign * head, cy - head, tipX - dirSign * head, cy + head);
        g.fillPath (p);
    };

    switch (dir)
    {
        case DirForward:
            g.drawLine (r.getX(), cy, r.getRight() - head, cy, 1.4f);
            arrowHead (r.getRight(), 1.0f);
            break;
        case DirReverse:
            g.drawLine (r.getX() + head, cy, r.getRight(), cy, 1.4f);
            arrowHead (r.getX(), -1.0f);
            break;
        case DirPingPong:
            g.drawLine (r.getX() + head, cy, r.getRight() - head, cy, 1.4f);
            arrowHead (r.getRight(), 1.0f);
            arrowHead (r.getX(), -1.0f);
            break;
        default:   // random: a die
        {
            auto die = r.withSizeKeepingCentre (8.0f, 8.0f);
            g.drawRoundedRectangle (die, 1.5f, 1.1f);
            g.fillEllipse (die.getX() + 1.6f, die.getY() + 1.6f, 1.8f, 1.8f);
            g.fillEllipse (die.getRight() - 3.4f, die.getBottom() - 3.4f, 1.8f, 1.8f);
            break;
        }
    }
}

void Grids::mouseDown (const juce::MouseEvent& e)
{
    if (e.eventComponent == &myLineLabel
        || (e.eventComponent == this && directionArea.contains (e.getPosition())))
        showLaneMenu();
}

void Grids::showLaneMenu()
{
    enum { dirBase = 10, euclidId = 20, copyId = 30, pasteId, shiftLeftId = 40, shiftRightId, clearId, shiftAllLeftId, shiftAllRightId,
           mutateBase = 100, mutateResetId = 300, playBase = 400, muteId = 500, soloId, strumShapeId = 600 };
    static const int mutateAmounts[] = { 0, 5, 10, 25, 50, 100 };

    const int dir = audioProcessor.getDirection (myLine);
    juce::PopupMenu m;
    const juce::String shiftSymbol =
       #if JUCE_MAC
        juce::CharPointer_UTF8 ("\xe2\x87\xa7");   // ⇧
       #else
        "Shift+";
       #endif
    m.addSectionHeader ("Lane " + juce::String (myLine + 1));
    {
        juce::PopupMenu::Item mute ("Mute");
        mute.itemID = muteId;
        mute.isTicked = audioProcessor.isLaneMuted (myLine);
        mute.shortcutKeyDescription = shiftSymbol + "click the note box";
        m.addItem (mute);
        juce::PopupMenu::Item solo ("Solo");
        solo.itemID = soloId;
        solo.isTicked = audioProcessor.getSoloState() == myLine;
        solo.shortcutKeyDescription = "click the note box";
        m.addItem (solo);
    }

    const int play = audioProcessor.getPlayMode (myLine);
    m.addSectionHeader ("Plays");
    for (int p = 0; p < playModeNames.size(); p++)
        m.addItem (playBase + p, p == PlayVoice ? juce::String ("Voice  (its own note of the chord)")
                                 : p == PlayStrum ? juce::String ("Chord / Strum  (STRM knob: 0 chord, + up, - down)")
                                                  : juce::String ("Strum Up/Down  (alternates, starts by the knob's sign)"),
                   true, p == play);
    m.addItem (strumShapeId, "Strum shape...  (or right-click STRM)", play == PlayStrum || play == PlayStrumUpDown);
    m.addSectionHeader ("Direction");
    for (int d = 0; d < directionNames.size(); d++)
        m.addItem (dirBase + d, directionNames[d], true, d == dir);
    const int mutate = audioProcessor.getMutate (myLine);
    m.addSectionHeader ("Mutate  -  each loop, steps flip with this chance");
    bool listed = false;
    for (int amount : mutateAmounts)
    {
        m.addItem (mutateBase + amount, amount == 0 ? juce::String ("Off") : juce::String (amount) + " %", true, amount == mutate);
        listed |= amount == mutate;
    }
    if (! listed)   // set from automation to a value the menu doesn't list
        m.addItem (mutateBase + mutate, juce::String (mutate) + " %  (automated)", false, true);
    m.addItem (mutateResetId, "Reset mutations");
    m.addSeparator();
    m.addItem (euclidId, "Euclidean fill...");
    m.addSeparator();
    m.addItem (copyId,  "Copy lane");
    m.addItem (pasteId, "Paste lane", audioProcessor.hasLaneClipboard());
    m.addSeparator();
    // the shortcuts work while the mouse is over a lane
    auto shortcutItem = [&m] (int id, const juce::String& text, const juce::String& keys)
    {
        juce::PopupMenu::Item item (text);
        item.itemID = id;
        item.shortcutKeyDescription = keys;
        m.addItem (item);
    };
    shortcutItem (shiftLeftId,     "Shift steps left",            ShortcutText::left);
    shortcutItem (shiftRightId,    "Shift steps right",           ShortcutText::right);
    shortcutItem (shiftAllLeftId,  "Shift all lanes left",        ShortcutText::shiftLeft);
    shortcutItem (shiftAllRightId, "Shift all lanes right",       ShortcutText::shiftRight);
    m.addItem (clearId,      "Clear lane");

    m.setLookAndFeel (&myLookAndFeel);
    juce::Component::SafePointer<Grids> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&myLineLabel),
                     [safe] (int r)
                     {
                         if (safe == nullptr || r == 0) return;
                         auto& p = safe->audioProcessor;
                         const int line = safe->myLine;
                         if (r == strumShapeId) safe->showStrumShape();
                         else if (r == muteId) p.setLaneMute (line, ! p.isLaneMuted (line));
                         else if (r == soloId) p.setGridSolo (line);
                         else if (r >= dirBase && r < dirBase + directionNames.size()) p.setLaneDirection (line, r - dirBase);
                         else if (r >= mutateBase && r <= mutateBase + 100)      p.setLaneMutate (line, r - mutateBase);
                         else if (r == mutateResetId) p.requestMutationReset (line);
                         else if (r >= playBase && r < playBase + playModeNames.size()) p.setLanePlayMode (line, r - playBase);
                         else if (r == copyId)       p.copyLane (line);
                         else if (r == pasteId)      p.pasteLane (line);
                         else if (r == shiftLeftId)  p.shiftLane (line, -1);
                         else if (r == shiftRightId) p.shiftLane (line, +1);
                         else if (r == shiftAllLeftId)  p.shiftAllLanes (-1);
                         else if (r == shiftAllRightId) p.shiftAllLanes (+1);
                         else if (r == clearId)      p.clearLane (line);
                         else if (r == euclidId)
                             juce::CallOutBox::launchAsynchronously (std::make_unique<EuclidPanel> (p, line),
                                                                     safe->myLineLabel.getScreenBounds(), nullptr);
                         safe->repaint();
                     });
}

void Grids::showStrumShape()
{
    juce::CallOutBox::launchAsynchronously (std::make_unique<StrumShapePanel> (audioProcessor, myLine),
                                            spreadKnob.getScreenBounds(), nullptr);
}

//== Strum shape call-out ======================================================

StrumShapePanel::StrumShapePanel (TugMidiSeqAudioProcessor& p, int l) : proc (p), line (l)
{
    centredLook.setdrawRotaryCenterd (true);
    const juce::String n (line);
    auto setup = [&] (CustomRoratySlider& k, bool centred, int base, std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>& att)
    {
        k.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        k.setColour (juce::Slider::rotarySliderFillColourId, colourarray[line]);
        if (centred) k.setLookAndFeel (&centredLook);
        addAndMakeVisible (k);
        att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.valueTreeState, valueTreeNames[base] + n, k);
    };
    setup (tension,  true,  STRUMTENSION, tensionAtt);
    setup (velocity, true,  STRUMVEL,     velocityAtt);
    setup (humanize, false, STRUMHUMAN,   humanizeAtt);

    for (auto* b : { &linearButton, &curveButton })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId, Theme::surfaceAlt);
        b->setColour (juce::TextButton::buttonOnColourId, Theme::accent);
        b->setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
        b->setColour (juce::TextButton::textColourOnId, Theme::screen);
        addAndMakeVisible (*b);
    }
    linearButton.onClick = [this] { setShape (StrumLinear); };
    curveButton.onClick  = [this] { setShape (StrumCurve); };

    setSize (270, 196);
    timerCallback();
    startTimerHz (30);
}

StrumShapePanel::~StrumShapePanel()
{
    stopTimer();
    tensionAtt.reset(); velocityAtt.reset(); humanizeAtt.reset();
    for (auto* k : { &tension, &velocity })
        k->setLookAndFeel (nullptr);
}

void StrumShapePanel::setShape (int shape)
{
    proc.undoableEdit ([&] { proc.setParamValue (valueTreeNames[STRUMSHAPE] + juce::String (line), (float) shape); });
}

// keep the buttons, the tension knob and the preview in step with the parameters
void StrumShapePanel::timerCallback()
{
    const int shape = proc.getStrumShape (line);
    linearButton.setToggleState (shape == StrumLinear, juce::dontSendNotification);
    curveButton.setToggleState (shape == StrumCurve, juce::dontSendNotification);
    tension.setEnabled (shape == StrumCurve);
    repaint (preview.toNearestInt().expanded (2));
}

void StrumShapePanel::resized()
{
    auto b = getLocalBounds().reduced (10, 8);
    b.removeFromTop (14);                                   // caption
    preview = b.removeFromTop (70).toFloat();
    b.removeFromTop (6);
    auto buttons = b.removeFromTop (20);
    linearButton.setBounds (buttons.removeFromLeft (buttons.getWidth() / 2).reduced (2, 0));
    curveButton.setBounds (buttons.reduced (2, 0));
    b.removeFromTop (4);
    b.removeFromBottom (12);                                // knob captions
    const int w = b.getWidth() / 3;
    tension.setBounds (b.removeFromLeft (w));
    velocity.setBounds (b.removeFromLeft (w));
    humanize.setBounds (b);
}

void StrumShapePanel::paint (juce::Graphics& g)
{
    g.fillAll (Theme::section);
    auto b = getLocalBounds().reduced (10, 8);

    const int spread = proc.getSpread (line);
    const juce::String dir = spread > 0 ? "up" : spread < 0 ? "down" : "at once";
    Theme::drawCaption (g, "Strum shape  -  lane " + juce::String (line + 1) + "  (" + dir + ")",
                        b.removeFromTop (14), juce::Justification::centredLeft, Theme::textSecondary, 10.0f);

    // six strings, low at the bottom; a dot where each note of the strum lands
    Theme::drawRecessedWell (g, preview, Theme::radSm);
    auto area = preview.reduced (10.0f, 8.0f);
    constexpr int strings = 6;
    const int shape = proc.getStrumShape (line);
    const float tens = proc.getStrumTension (line), tilt = proc.getStrumVelTilt (line), human = proc.getStrumHumanize (line);
    const auto lane = colourarray[line];
    for (int s = 0; s < strings; s++)
    {
        const float y = area.getBottom() - area.getHeight() * (float) s / (float) (strings - 1);
        g.setColour (Theme::hairline);
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
    }
    for (int k = 0; k < strings; k++)
    {
        float pos, velF;
        strumNotePlacement (k, strings, shape, tens, tilt, pos, velF);
        const int stringIndex = spread < 0 ? strings - 1 - k : k;   // a down strum starts on the high string
        const float y = area.getBottom() - area.getHeight() * (float) stringIndex / (float) (strings - 1);
        const float x = spread == 0 ? area.getX() : area.getX() + pos * area.getWidth();
        if (human > 0.0f && k > 0 && spread != 0)
        {
            const float jitter = 0.5f * human * area.getWidth() / (float) (strings - 1);
            g.setColour (lane.withAlpha (0.25f));
            g.fillRoundedRectangle (x - jitter, y - 2.0f, jitter * 2.0f, 4.0f, 2.0f);
        }
        const float r = 2.5f + 2.5f * jlimit (0.0f, 1.6f, velF);
        g.setColour (lane);
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
    }

    auto captions = getLocalBounds().reduced (10, 8).removeFromBottom (12);
    const int w = captions.getWidth() / 3;
    Theme::drawCaption (g, "Tension",  captions.removeFromLeft (w), juce::Justification::centred, Theme::textDim, 9.0f);
    Theme::drawCaption (g, "Velocity", captions.removeFromLeft (w), juce::Justification::centred, Theme::textDim, 9.0f);
    Theme::drawCaption (g, "Humanize", captions, juce::Justification::centred, Theme::textDim, 9.0f);
}

//== Euclidean fill call-out ===================================================

EuclidPanel::EuclidPanel (TugMidiSeqAudioProcessor& p, int l)
    : proc (p), line (l), length (juce::jlimit (1, numOfStep, (int) *p.numOfGrid[l]))
{
    // start from the lane's current number of active steps, unrotated; nothing
    // is written until a knob moves
    int active = 0;
    for (int s = 0; s < length; s++)
        if (*proc.gridsArr[line][s] != 0) active++;

    for (auto* k : { &hits, &rotate })
    {
        k->setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        k->setColour (juce::Slider::rotarySliderFillColourId, colourarray[line]);
        addAndMakeVisible (*k);
    }
    hits.setRange (0, length, 1);
    rotate.setRange (0, juce::jmax (1, length - 1), 1);
    hits.setValue (active, juce::dontSendNotification);
    rotate.setValue (0, juce::dontSendNotification);

    proc.beginUndoStep();
    auto apply = [this] { proc.euclidLane (line, (int) hits.getValue(), (int) rotate.getValue()); };
    hits.onValueChange   = apply;
    rotate.onValueChange = apply;

    setSize (150, 96);
}

EuclidPanel::~EuclidPanel()
{
    proc.endUndoStep();
}

void EuclidPanel::paint (juce::Graphics& g)
{
    g.fillAll (Theme::section);
    auto b = getLocalBounds().reduced (8, 4);
    Theme::drawCaption (g, "Euclidean  -  " + juce::String (length) + " steps", b.removeFromTop (14),
                        juce::Justification::centred, Theme::textSecondary, 10.0f);
    auto labels = b.removeFromBottom (12);
    Theme::drawCaption (g, "Hits",   labels.removeFromLeft (labels.getWidth() / 2), juce::Justification::centred, Theme::textDim, 9.0f);
    Theme::drawCaption (g, "Rotate", labels, juce::Justification::centred, Theme::textDim, 9.0f);
}

void EuclidPanel::resized()
{
    auto b = getLocalBounds().reduced (8, 4);
    b.removeFromTop (14);
    b.removeFromBottom (12);
    hits.setBounds (b.removeFromLeft (b.getWidth() / 2));
    rotate.setBounds (b);
}

void Grids::paintOverChildren (juce::Graphics& g)
{
    if (audioProcessor.isLaneMuted (myLine))
    {
        // muted: shadowed like a lane outside a solo, and labelled over the grid
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRect (getLocalBounds());
        auto grid = subGrids->getBounds();
        Theme::drawCaption (g, "Muted", grid, juce::Justification::centred, Theme::accentBright, 12.0f);
        return;
    }

    int solo = audioProcessor.getSoloState();
    if (solo == -1) return;               // no solo active: nothing to shade

    if (solo == myLine)
    {
        // the soloed lane stays lit, with a crisp brass frame above the widgets
        g.setColour (Theme::accent.withAlpha (0.85f));
        g.drawRect (getLocalBounds().toFloat().reduced (1.0f), 1.2f);
    }
    else
    {
        // every other lane is shadowed (covers its widgets too)
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRect (getLocalBounds());
    }
}
void Grids::resized()
{
    float marjin =  1;
 
    auto area = getLocalBounds();
    if(audioProcessor.getChannelStatus()) gridMidiRouteCombo.setEnabled(true);
    else  gridMidiRouteCombo.setEnabled(false);
    gridMidiRouteCombo.setBounds( area.removeFromRight(LaneLayout::chan).reduced(0,8));
    gridDelaySlider.setBounds( area.removeFromRight(LaneLayout::delay));
    gridShuffleSlider.setBounds( area.removeFromRight(LaneLayout::shuffle));
    gridEventSlider.setBounds( area.removeFromRight(LaneLayout::event));
    gridVelSlider.setBounds( area.removeFromRight(LaneLayout::vel));
    // wide enough for a note glyph + "1/128t"
    gridDurationCombo.setBounds(area.removeFromRight(LaneLayout::duration).reduced(2,8));
    gridSpeedCombo.setBounds(area.removeFromRight(LaneLayout::speed).reduced(2,8));
    gridNumberSlider.setBounds( area.removeFromRight(LaneLayout::steps));

    // lane number on top, its play-direction glyph underneath (both open the lane menu)
    auto laneTab = area.removeFromLeft(LaneLayout::number);
    directionArea = laneTab.removeFromBottom (13).withTrimmedRight (2);
    myLineLabel.setBounds (laneTab.withTrimmedTop (4));
    midiInNote.setBounds(area.removeFromLeft(LaneLayout::midiIn).reduced(0,10));
    spreadKnob.setBounds(area.removeFromLeft(LaneLayout::spread));
    octaveSlider.setBounds(area.removeFromLeft(LaneLayout::octave));
    
    subGrids->setBounds(area);
    area.removeFromTop(7);
    area.removeFromBottom(7);
    
    auto griidbounds =  area.reduced(10, 2);
    juce::FlexBox fb;
    fb.flexWrap = juce::FlexBox::Wrap::wrap;

    fb.alignContent = juce::FlexBox::AlignContent::center;

    fb.flexWrap= juce::FlexBox::Wrap::noWrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
 
    int n = *audioProcessor.numOfGrid[myLine] ;
    auto w = (griidbounds.toFloat().getWidth()   /(n)) ;
    float  w_tmp;
    for ( auto *b : buttons) b->setVisible(false);
    stepArrow.setVisible(false);
    float sumButton =  0 ;
    int totalGridWidth = griidbounds.getWidth();
    totalGridWidth = totalGridWidth - 1;
    
    for ( int i = 0; i < n;i++)
    {
        auto r = audioProcessor.getStepDisplayRatio (myLine, i);
         w_tmp = w*r;
        sumButton =  sumButton + w_tmp ;
        
        if(sumButton  > totalGridWidth)
            w_tmp = w_tmp  - (sumButton -totalGridWidth);
            
        if(step != i || step == -1)
        {
        buttons[i]->setVisible(true);
        buttons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey);
        buttons[i]->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::orange.withAlpha(0.95f));
        buttons[i]->setButtonText("");
            
       
       // fb.items.add (juce::FlexItem (*buttons[i]).withMinWidth (w_tmp-2*marjin).withMinHeight ((float) griidbounds.getHeight() -2 ).withMargin(marjin));
        fb.items.add(juce::FlexItem(*buttons[i]).withMinWidth(w_tmp - 2 * marjin).withMinHeight((float)griidbounds.getHeight() - 2).withMargin(marjin));
        
        }
        
        
        else{
            buttons[i]->setVisible(true);
            buttons[step]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black.withAlpha(0.90f));
            buttons[step]->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::cyan.withAlpha(0.90f));
            buttons[step]->setButtonText(">");
            buttons[step]->setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::darkgrey);
            buttons[step]->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::darkgrey);
            fb.items.add (juce::FlexItem (*buttons[i]).withMinWidth(w_tmp-2*marjin).withMinHeight ((float) griidbounds.getHeight() -2 ).withMargin(marjin));
        }
    }
    fb.performLayout (griidbounds.toFloat());
 /*
    for ( int i = 0; i < n;i++)
    {
        auto r = audioProcessor.getSfuffleRatios(myLine,i);
         w_tmp = w*r;
        sumButton =  sumButton + w_tmp ;
        
        if(sumButton  > totalGridWidth)
            w_tmp = w_tmp  - (sumButton -totalGridWidth);
        
        if(step != i || step == -1)
        {
        velButtons[i]->setVisible(true);

            
       
       // fb.items.add (juce::FlexItem (*buttons[i]).withMinWidth (w_tmp-2*marjin).withMinHeight ((float) griidbounds.getHeight() -2 ).withMargin(marjin));
        fb.items.add(juce::FlexItem(*velButtons[i]).withMinWidth(w_tmp - 2 * marjin).withMinHeight((float)griidbounds.getHeight() - 2).withMargin(marjin));
        
        }
        
    }
  */
     
    //fb.performLayout (griidbounds.toFloat().translated(getWidth()* audioProcessor.getDelayRatio(myLine), 0));
    fb.performLayout (griidbounds.toFloat());

}

void  SubGrids::paint (juce::Graphics& g)
{
    auto soloLane = audioProcessor.getSoloState();
    if( soloLane != -1 &&  soloLane != myLine) return;

    const int numSteps = ownerGrid.getParam (GETNUMOF);
    const int playing  = audioProcessor.getPlayheadStep (myLine);   // -1 when stopped

    // note-length strips above the active pads; the one being played lights up
    float len = audioProcessor.getGridSampleLen( myLine);
    auto eventProb = audioProcessor.getEventRandom(myLine);
    for (int i = numSteps - 1; i >= 0; i--)
    {
        const int s_x = ownerGrid.getParam (GETCOORDOFBUTTON, i);
        if (s_x == -1) continue;   // off pad
        auto sr = audioProcessor.getStepDisplayRatio (myLine, i);
        const bool lit = i == playing && audioProcessor.midiState[myLine];
        const float alpha = lit ? 1.0f : (audioProcessor.getGridButtonState (myLine, i) == 2 ? eventProb : 1.0f) * 0.8f;
        g.setColour (colourarray[myLine].withAlpha (alpha));
        const float noteW = (float) (0.95 * len * sr * getWidth());
        const int hits = audioProcessor.getStepRatchet (myLine, i);
        if (hits <= 1)
        {
            g.fillRect (Rectangle<float> ((float) s_x, 4.0f, noteW, 5.0f));
        }
        else
        {
            // a ratchet plays `hits` shorter notes across the step: one segment
            // each, as long as the engine gates them (3/4 of a share at most)
            const float share = ownerGrid.padBounds (i).getWidth() / (float) hits;
            const float segW  = jmax (1.0f, jmin (noteW, share * 0.75f));
            for (int k = 0; k < hits; k++)
                g.fillRect (Rectangle<float> ((float) s_x + share * (float) k, 4.0f, segW, 5.0f));
        }
    }

    if (playing >= 0 && playing < numSteps)
        drawPlayhead (g, playing, numSteps);
}

void SubGrids::drawPlayhead (juce::Graphics& g, int step, int numSteps)
{
    const auto pad      = ownerGrid.padBounds (step);
    const bool backward = audioProcessor.isPlayheadBackward (myLine);
    const float frac    = audioProcessor.getPlayheadFraction (myLine);
    const float y       = (float) getHeight() - 6.0f;
    const float sign    = backward ? -1.0f : 1.0f;
    const float headX   = backward ? pad.getRight() - frac * pad.getWidth()
                                   : pad.getX()     + frac * pad.getWidth();

    // travelling bar: from where this pass started up to the head
    float startX = backward ? ownerGrid.padBounds (numSteps - 1).getRight() : ownerGrid.padBounds (0).getX();
    if (audioProcessor.getDirection (myLine) == DirRandom)
    {
        // no pass to show: the steps jump. The last two played pads keep a
        // fading underline so the jumps can be followed.
        for (int k = 0; k < 2; k++)
            if (trail[k] >= 0 && trail[k] < numSteps && trail[k] != step)
            {
                auto t = ownerGrid.padBounds (trail[k]);
                g.setColour (Theme::accent.withAlpha (k == 0 ? 0.22f : 0.45f));
                g.fillRect (t.getX(), y - 1.0f, t.getWidth(), 2.0f);
            }
        startX = pad.getX();
    }

    auto bar = juce::Rectangle<float> (juce::jmin (startX, headX), y - 1.0f, std::abs (headX - startX), 2.0f);
    g.setColour (Theme::accent);
    g.fillRect (bar);
    DropShadow (Theme::accentBright.withAlpha (0.9f), 3, {}).drawForRectangle (g, bar.toNearestInt());

    g.setColour (Theme::accentBright);
    g.drawArrow (juce::Line<float> (headX, y, headX + sign * 10.0f, y), 4.0f, 4.0f, 5.0f);
}

void  SubGrids::resized ()
{


}

//== Drag-to-paint =============================================================

void MultiStateButton::mouseDown (const MouseEvent& e)
{
    shiftPressed = false;
    altPressed = false;
    audioProcesor.beginUndoStep();   // one undo step per gesture (paint, velocity / pitch drag, menu)

    // Real right button only: on macOS ctrl+left-click also counts as a popup
    // click (isPopupMenu), but ctrl+click is already "paint an Event cell".
    if (e.mods.isRightButtonDown())
    {
        showStepMenu();
        return;
    }

    if (juce::ModifierKeys::currentModifiers.isShiftDown())
    {
        shiftPressed = true;                // shift+drag = step velocity
        y = e.getPosition().getY();
        if (auto* p = stepVelParam())
            p->beginChangeGesture();
        showValuePopup();
        return;
    }

    if (e.mods.isAltDown())
    {
        altPressed = true;                  // alt+drag = step pitch
        y = e.getPosition().getY();
        pitchDrag = 0.0f;
        showValuePopup();
        return;
    }

    if (! e.mods.isLeftButtonDown())
    {
        Button::mouseDown (e);
        return;
    }

    // Open a paint gesture: decide the brush from this pad, apply it here, then
    // let Grids stamp the same value onto every pad the cursor crosses.
    State brush = cycledState (e.mods.isCtrlDown());
    paintTo (brush);
    if (ownerGrid != nullptr)
        ownerGrid->beginPaint (brush, myStep);
}

void MultiStateButton::mouseDrag (const MouseEvent& e)
{
    if (shiftPressed)
    {
        auto p = e.getPosition().getY();
        float z = y - p;
        y = p;
        // read the step's own value: getVelButton() reports 1.0 in "In Vel" mode
        if (auto* param = stepVelParam())
            param->setValueNotifyingHost (jlimit (0.0f, 1.0f, param->getValue() + z / 127.0f));
        updateValuePopup();
        return;
    }

    if (altPressed)
    {
        // one semitone / scale degree per 6px of vertical travel
        auto p = (float) e.getPosition().getY();
        pitchDrag += (y - p) / 6.0f;
        y = p;
        const int steps = (int) pitchDrag;   // towards zero, keeps the remainder
        if (steps != 0)
        {
            pitchDrag -= (float) steps;
            audioProcesor.setStepPitchUndoable (myLine, myStep, audioProcesor.getStepPitch (myLine, myStep) + steps);
            updateValuePopup();
            repaint();
        }
        return;
    }

    // Screen coords so the gesture can reach pads in any lane, not just this one.
    if (ownerGrid != nullptr && e.mods.isLeftButtonDown())
        ownerGrid->paintDrag (e.getScreenPosition());
}

void MultiStateButton::mouseUp (const MouseEvent& e)
{
    if (shiftPressed)
    {
        if (auto* p = stepVelParam())
            p->endChangeGesture();
        hideValuePopup();
        shiftPressed = false;
    }
    if (altPressed)
    {
        hideValuePopup();
        altPressed = false;
        audioProcesor.notifyStateChanged();   // step pitch isn't a parameter
    }

    // The origin pad was already painted on mouseDown; suppress the Button click
    // so the attachment doesn't double-toggle it.
    if (ownerGrid != nullptr)
        ownerGrid->endPaint();
}

juce::RangedAudioParameter* MultiStateButton::stepVelParam() const
{
    juce::String id;
    id << valueTreeNames[VELGRIDBUTTON] << myLine << myStep;
    return audioProcesor.valueTreeState.getParameter (id);
}

void MultiStateButton::showValuePopup()
{
    // the lane's parent is the editor content: room to float above the row
    auto* host = ownerGrid != nullptr ? ownerGrid->getParentComponent() : nullptr;
    if (host == nullptr) return;

    valuePopup = std::make_unique<StepValuePopup> (colourarray[myLine]);
    host->addAndMakeVisible (*valuePopup);
    valuePopup->placeFor (*this, *host);
    updateValuePopup();
    repaint();
}

void MultiStateButton::updateValuePopup()
{
    if (valuePopup == nullptr) return;

    if (altPressed)
    {
        // pitch: centre-zero meter; the unit follows the scale lock
        const int off = audioProcesor.getStepPitch (myLine, myStep);
        const auto text = (off > 0 ? "+" : "") + juce::String (off);
        const float pos = 0.5f + 0.5f * (float) off / (float) maxStepPitch;
        valuePopup->setContent (audioProcesor.isScaleOn() ? "pitch deg" : "pitch st", text, 0.5f, pos, false);
        return;
    }

    auto* p = stepVelParam();
    const int value = p != nullptr ? juce::roundToInt (p->convertFrom0to1 (p->getValue())) : 0;
    // "In Vel" on: the engine plays the incoming MIDI velocity, not this one
    const bool ignored = *audioProcesor.valueTreeState.getRawParameterValue (valueTreeNames[GLOABLINORFIXVEL]) != 0;
    valuePopup->setContent (ignored ? "in vel" : "step vel", juce::String (value), 0.0f, (float) value / 127.0f, ignored);
}

void MultiStateButton::hideValuePopup()
{
    valuePopup.reset();   // ~Component removes it from the editor content
    repaint();
}

void MultiStateButton::showStepMenu()
{
    enum { ratchetBase = 100, resetPitchId = 200, clearLaneId = 1000 };   // condition items use id = condition + 1

    const int current = audioProcesor.getStepCond (myLine, myStep);
    const int ratchet = audioProcesor.getStepRatchet (myLine, myStep);
    auto item = [current] (juce::PopupMenu& m, int cond, const juce::String& text)
    {
        m.addItem (cond + 1, text, true, cond == current);
    };

    juce::PopupMenu m;
    m.addSectionHeader ("Lane " + juce::String (myLine + 1) + "  -  step " + juce::String (myStep + 1) + " ratchet");
    for (int r = 1; r <= maxRatchet; r++)
        m.addItem (ratchetBase + r, r == 1 ? juce::String ("x1   single hit") : "x" + juce::String (r) + "   " + juce::String (r) + " hits per step",
                   true, r == ratchet);
    const int pitch = audioProcesor.getStepPitch (myLine, myStep);
    m.addItem (resetPitchId, pitch == 0 ? juce::String ("Pitch  0   (alt+drag to change)")
                                        : "Reset pitch  (" + juce::String (pitch > 0 ? "+" : "") + juce::String (pitch) + ")",
               pitch != 0);
    m.addSectionHeader ("Condition");
    item (m, CondNone, "Always");
    m.addSeparator();
    item (m, Cond1of2, "1:2   1st of every 2 loops");
    item (m, Cond2of2, "2:2   2nd of every 2 loops");
    item (m, Cond1of3, "1:3   1st of every 3 loops");
    item (m, Cond2of3, "2:3   2nd of every 3 loops");
    item (m, Cond3of3, "3:3   3rd of every 3 loops");
    item (m, Cond1of4, "1:4   1st of every 4 loops");
    item (m, Cond2of4, "2:4   2nd of every 4 loops");
    item (m, Cond3of4, "3:4   3rd of every 4 loops");
    item (m, Cond4of4, "4:4   4th of every 4 loops");
    m.addSeparator();
    item (m, CondFirst,    "1ST   first loop after play only");
    item (m, CondNotFirst, "!1ST  every loop but the first");
    m.addSeparator();
    item (m, CondPre,    "PRE   if this lane's previous condition passed");
    item (m, CondNotPre, "!PRE  if it failed");
    item (m, CondNei,    "NEI   if lane " + juce::String ((myLine + numOfLine - 1) % numOfLine + 1) + "'s last condition passed");
    item (m, CondNotNei, "!NEI  if it failed");
    m.addSeparator();
    item (m, CondFill,    "FILL  only while Fill is held");
    item (m, CondNotFill, "!FILL except while Fill is held");
    m.addSeparator();
    m.addItem (clearLaneId, "Clear all conditions in this lane");

    if (ownerGrid != nullptr)
        m.setLookAndFeel (&ownerGrid->getMenuLookAndFeel());

    juce::Component::SafePointer<MultiStateButton> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                     [safe] (int result)
                     {
                         if (safe == nullptr || result == 0) return;
                         auto& p = safe->audioProcesor;
                         if (result == resetPitchId)
                         {
                             p.undoableEdit ([&] { p.setStepPitchUndoable (safe->myLine, safe->myStep, 0); });
                         }
                         else if (result > ratchetBase && result <= ratchetBase + maxRatchet)
                         {
                             p.undoableEdit ([&] { p.setStepRatchetUndoable (safe->myLine, safe->myStep, result - ratchetBase); });
                         }
                         else if (result == clearLaneId)
                         {
                             p.undoableEdit ([&] {
                                 for (int s = 0; s < numOfStep; s++)
                                     p.setStepCondUndoable (safe->myLine, s, CondNone);
                             });
                         }
                         else
                         {
                             p.undoableEdit ([&] { p.setStepCondUndoable (safe->myLine, safe->myStep, result - 1); });
                         }
                         p.notifyStateChanged();
                         if (auto* lane = safe->getParentComponent())
                             lane->repaint();
                     });
}

// Walk the sibling lanes (all the Grids that share our parent editor).
template <typename Fn>
static void forEachLane (juce::Component* self, Fn&& fn)
{
    if (auto* parent = self->getParentComponent())
        for (int i = 0; i < parent->getNumChildComponents(); ++i)
            if (auto* g = dynamic_cast<Grids*> (parent->getChildComponent (i)))
                fn (g);
}

void Grids::beginPaint (MultiStateButton::State s, int originStep)
{
    painting   = true;
    brushState = s;

    // fresh gesture: clear the "already painted" marks on every lane
    forEachLane (this, [] (Grids* g) { g->resetPainted(); });

    if (originStep >= 0 && originStep < numOfStep)
        paintedStep[originStep] = true;   // origin was painted on mouseDown
    subGrids->rP();
}

void Grids::paintLocal (juce::Point<int> screenPos, MultiStateButton::State s)
{
    auto local = getLocalPoint (nullptr, screenPos);
    int n = (int) *audioProcessor.numOfGrid[myLine];
    for (int i = 0; i < n && i < buttons.size(); ++i)
    {
        auto* b = buttons[i];
        if (b->isVisible() && ! paintedStep[i] && b->getBounds().contains (local))
        {
            b->paintTo (s);
            paintedStep[i] = true;
        }
    }
    subGrids->rP();
}

void Grids::paintDrag (juce::Point<int> screenPos)
{
    if (! painting) return;
    // origin drives every lane; each hit-tests the cursor in its own space
    forEachLane (this, [this, screenPos] (Grids* g) { g->paintLocal (screenPos, brushState); });
}

void Grids::endPaint()
{
    painting = false;
}
