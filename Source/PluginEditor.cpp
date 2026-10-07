/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// EditorContent — the fixed-size UI. Identical layout to before; it just no
// longer lives directly in the editor so it can be scaled as one unit.
//==============================================================================
EditorContent::EditorContent (TugMidiSeqAudioProcessor& p)
    : audioProcessor (p), satellite (p), noteMap (p), globalPanel (p)
{
    addAndMakeVisible (satellite);
    addAndMakeVisible (noteMap);
    for (auto i = 4; i >= 0; i--)
    {
        auto g = new Grids (audioProcessor, i);
        grids.add (g);
        addAndMakeVisible (g);
    }
    addAndMakeVisible (globalPanel);

    for (auto i = 0; i < topLabel.size(); i++)
    {
        auto* lbl = new juce::Label (topLabel.at (i), juce::String (topLabel.at (i)).toUpperCase());
        lbl->setColour (Label::textColourId, myTextLabelColour);
        lbl->setJustificationType (juce::Justification::centred);
        lbl->setFont (Theme::labelFont (10.5f));
        topInLabel.add (lbl);
        addAndMakeVisible (lbl);
    }
    for (auto i = 0; i < 5; i++)
        globalPanel.setGridComp (grids[i], i);

    // takes the keyboard focus when anything inside is clicked, so keyPressed
    // sees the shortcuts; unhandled keys go on to the host. Combo boxes want the
    // focus by default and would eat the arrow keys (to step their selection),
    // so they're told not to: they still work with the mouse and wheel, and an
    // open popup menu still takes the arrows.
    setWantsKeyboardFocus (true);
    std::function<void (juce::Component&)> keepArrowsForShortcuts = [&] (juce::Component& c)
    {
        if (auto* box = dynamic_cast<juce::ComboBox*> (&c))
            box->setWantsKeyboardFocus (false);
        for (auto* child : c.getChildren())
            keepArrowsForShortcuts (*child);
    };
    keepArrowsForShortcuts (*this);

    versionBadge.onClick = [this] { showHelp(); };
    addAndMakeVisible (versionBadge);

    setSize (kEditorDesignW, kEditorDesignH);
}

EditorContent::~EditorContent()
{
    if (helpWindow != nullptr)   // it doesn't outlive the plugin's window
        delete helpWindow.getComponent();
}

void EditorContent::paint (juce::Graphics& g)
{
    // hardware module plate
    g.fillAll (Theme::panel);
    auto allarea = getLocalBounds();

    // outer frame + corner screws
    g.setColour (Theme::hairline);
    g.drawRect (allarea, 1);
    const float sc = 8.0f;
    Theme::drawScrew (g, sc,               sc,                sc * 0.55f);
    Theme::drawScrew (g, getWidth() - sc,  sc,                sc * 0.55f);
    Theme::drawScrew (g, sc,               getHeight() - sc,  sc * 0.55f);
    Theme::drawScrew (g, getWidth() - sc,  getHeight() - sc,  sc * 0.55f);

    // menu glyph over the lane numbers, which open each lane's menu
    {
        auto r = laneMenuHint.toFloat().withSizeKeepingCentre (11.0f, 8.0f).translated (2.0f, 1.0f);
        g.setColour (Theme::textSecondary);
        for (int k = 0; k < 3; k++)
            g.fillRoundedRectangle (r.getX(), r.getY() + (float) k * 3.5f, r.getWidth(), 1.4f, 0.7f);
    }
}

Grids* EditorContent::laneUnderMouse() const
{
    const auto mouse = juce::Desktop::getMousePosition();
    for (auto* g : grids)
        if (g->getScreenBounds().contains (mouse))
            return g;
    return nullptr;
}

// Cmd+Z / Cmd+Shift+Z (Ctrl on Windows): undo / redo.
// With the mouse over a lane: Left / Right shift that lane a step,
// Shift+Left / Shift+Right shift every lane. Away from the lanes the arrows
// are left to the host, as is every other key.
bool EditorContent::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();

    if (mods.isCommandDown() && juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) code) == 'z')
    {
        if (mods.isShiftDown()) audioProcessor.redo();
        else                    audioProcessor.undo();
        return true;
    }

    if ((code == juce::KeyPress::leftKey || code == juce::KeyPress::rightKey)
        && ! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        auto* lane = laneUnderMouse();
        if (lane == nullptr) return false;
        const int delta = code == juce::KeyPress::leftKey ? -1 : 1;
        if (mods.isShiftDown()) audioProcessor.shiftAllLanes (delta);
        else                    audioProcessor.shiftLane (lane->getLine(), delta);
        return true;
    }

    // up / down: the lane's play direction (shift: every lane)
    if ((code == juce::KeyPress::upKey || code == juce::KeyPress::downKey)
        && ! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        auto* lane = laneUnderMouse();
        if (lane == nullptr) return false;
        audioProcessor.cycleLaneDirection (lane->getLine(), code == juce::KeyPress::downKey ? 1 : -1, mods.isShiftDown());
        return true;
    }

    // F / R / P / X: Forward, Reverse, Ping-Pong, Random (shift: every lane)
    if (! mods.isCommandDown() && ! mods.isAltDown() && ! mods.isCtrlDown())
    {
        const auto letter = juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) code);
        int dir = -1;
        for (int d = 0; d < directionNames.size(); d++)
            if (letter == juce::CharacterFunctions::toLowerCase ((juce::juce_wchar) directionKeys[d]))
                dir = d;
        if (dir >= 0)
        {
            auto* lane = laneUnderMouse();
            if (lane == nullptr) return false;
            audioProcessor.setDirectionOfLanes (lane->getLine(), dir, mods.isShiftDown());
            return true;
        }
    }
    return false;
}

void EditorContent::resized()
{
    auto allarea = getLocalBounds();
    allarea.reduce (3, 3);
    // Captions sit over the lane columns, taken from the same LaneLayout
    // widths Grids::resized uses (the lanes span the width left of the
    // satellite). The narrow left columns get no label insets, or their
    // captions are cut short.
    auto label_area = allarea.removeFromTop (20);
    topInLabel[11]->setBounds (label_area.removeFromRight (200));   // satellite
    versionBadge.setBounds (getWidth() - 74, topInLabel[11]->getY(), 66, topInLabel[11]->getHeight());
    for (int i : { 0, 1, 12 })
        topInLabel[i]->setBorderSize ({});
    laneMenuHint = label_area.removeFromLeft (LaneLayout::number).withTrimmedLeft (4);
    topInLabel[0]->setBounds (label_area.removeFromLeft (LaneLayout::midiIn));
    topInLabel[12]->setBounds (label_area.removeFromLeft (LaneLayout::spread));
    // "OCT" is wider than its 16 px slider: let it run into the grid's caption area
    auto octave = label_area.removeFromLeft (LaneLayout::octave);
    topInLabel[1]->setJustificationType (juce::Justification::centredLeft);
    topInLabel[1]->setBounds (octave.withWidth (30));
    label_area.removeFromLeft (14);
    topInLabel[10]->setBounds (label_area.removeFromRight (LaneLayout::chan));
    topInLabel[9]->setBounds (label_area.removeFromRight (LaneLayout::delay));
    topInLabel[8]->setBounds (label_area.removeFromRight (LaneLayout::shuffle));
    topInLabel[7]->setBounds (label_area.removeFromRight (LaneLayout::event));
    topInLabel[6]->setBounds (label_area.removeFromRight (LaneLayout::vel));
    topInLabel[5]->setBounds (label_area.removeFromRight (LaneLayout::duration));
    topInLabel[4]->setBounds (label_area.removeFromRight (LaneLayout::speed));
    topInLabel[3]->setBounds (label_area.removeFromRight (LaneLayout::steps));
    topInLabel[2]->setBounds (label_area);

    auto area = allarea.removeFromTop (200);
    auto satelite_area = area.removeFromRight (200);
    auto h = area.getHeight() / 5;
    for (auto g : grids)
        g->setBounds (area.removeFromTop (h));

    satellite.setBounds (satelite_area);
    noteMap.setBounds (allarea.removeFromTop (kNoteMapH));
    globalPanel.setBounds (allarea);
}

//==============================================================================
// The editor is a thin uniform scaler around EditorContent.
//==============================================================================
TugMidiSeqAudioProcessorEditor::TugMidiSeqAudioProcessorEditor (TugMidiSeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), content (p)
{
    addAndMakeVisible (content);

    // minimum = the authored size; larger allowed, aspect ratio locked so the
    // whole panel scales together (works the same in AU and VST3).
    constrainer.setFixedAspectRatio ((double) kEditorDesignW / (double) kEditorDesignH);
    constrainer.setSizeLimits (kEditorDesignW, kEditorDesignH,
                               kEditorDesignW * 3, kEditorDesignH * 3);
    setConstrainer (&constrainer);
    setResizable (true, true);

    // restore the last-used width (project- and session-persistent); height
    // follows from the locked aspect ratio.
    int w = juce::jlimit (kEditorDesignW, kEditorDesignW * 3,
                          audioProcessor.getEditorWidth (kEditorDesignW));
    int h = juce::roundToInt (w * (double) kEditorDesignH / (double) kEditorDesignW);
    setSize (w, h);
}

TugMidiSeqAudioProcessorEditor::~TugMidiSeqAudioProcessorEditor()
{
}

void TugMidiSeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::panel);
}

void TugMidiSeqAudioProcessorEditor::resized()
{
    auto scale = (float) getWidth() / (float) kEditorDesignW;
    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, kEditorDesignW, kEditorDesignH);

    // remember the new width so it survives GUI reopen and project reload
    audioProcessor.setEditorWidth (getWidth());
}

//==============================================================================
// Help window: what's new in this version, and every control and shortcut.

void VersionBadge::paint (juce::Graphics& g)
{
    const bool over = isMouseOver();
    auto b = getLocalBounds().toFloat();
    auto icon = b.removeFromRight (12.0f).withSizeKeepingCentre (10.0f, 10.0f);
    const auto c = over ? Theme::accentBright : Theme::textDim;
    g.setColour (c);
    g.drawEllipse (icon.reduced (0.5f), 1.0f);
    g.setFont (Theme::valueFont (8.0f));
    g.drawText ("i", icon, juce::Justification::centred, false);
    g.setFont (Theme::valueFont (9.0f));
    g.drawText ("v" + juce::String (ProjectInfo::versionString), b.withTrimmedRight (3.0f), juce::Justification::centredRight, false);
}

namespace
{
   #if JUCE_MAC
    const juce::String kCmd = juce::CharPointer_UTF8 ("\xe2\x8c\x98"), kShift = juce::CharPointer_UTF8 ("\xe2\x87\xa7"),
                       kAlt = juce::CharPointer_UTF8 ("\xe2\x8c\xa5"), kSep = "";
   #else
    const juce::String kCmd = "Ctrl", kShift = "Shift", kAlt = "Alt", kSep = "+";
   #endif
    const juce::String kLeft = juce::CharPointer_UTF8 ("\xe2\x86\x90"), kRight = juce::CharPointer_UTF8 ("\xe2\x86\x92"),
                       kUp = juce::CharPointer_UTF8 ("\xe2\x86\x91"), kDown = juce::CharPointer_UTF8 ("\xe2\x86\x93"),
                       kDash = juce::CharPointer_UTF8 ("\xe2\x80\x93");

    // Writes styled text into a read-only TextEditor.
    struct HelpWriter
    {
        juce::TextEditor& ed;
        juce::Font body  { juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain) };
        juce::Font mono  { juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::plain) };
        juce::Font title { juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 15.0f, juce::Font::bold) };

        void put (const juce::String& t, const juce::Font& f, juce::Colour c)
        {
            ed.setFont (f);
            ed.setColour (juce::TextEditor::textColourId, c);
            ed.insertTextAtCaret (t);
        }
        void heading (const juce::String& t) { put ("\n" + t.toUpperCase() + "\n", title, Theme::accentBright); }
        void text (const juce::String& t)    { put (t + "\n", body, Theme::textPrimary); }
        void item (const juce::String& name, const juce::String& t)
        {
            put (name, body.boldened(), Theme::textPrimary);
            put ("  " + t + "\n", body, Theme::textSecondary);
        }
        void key (const juce::String& keys, const juce::String& t)
        {
            put (keys.paddedRight (' ', 23), mono, Theme::accent);
            put (t + "\n", body, Theme::textPrimary);
        }
    };

    void writeNews (HelpWriter& w)
    {
        w.put ("TugMidiSeq " + juce::String (ProjectInfo::versionString) + "\n", w.title.withHeight (18.0f), Theme::textPrimary);
        w.text ("Projects and presets from earlier versions open as before.");

        w.heading ("Fixed in 2.6.1");
        w.text ("Steady playback in Logic: when a project had just been opened, the lanes restarted every few "
                "milliseconds (the playhead shook at the start) until the tempo was changed.");

        w.put ("\nNew in 2.6\n", w.title.withHeight (16.0f), Theme::textPrimary);

        w.heading ("Pattern slots A " + kDash + " D");
        w.text ("Four patterns in one instance. Each lane switches when its own loop comes round, so lanes of "
                "different lengths change over without cutting a pattern in half. Right-click a slot to copy or "
                "clear it; automate Pattern Slot from the DAW.");

        w.heading ("Chords and strums");
        w.text ("A lane can play the whole held chord. STRM sets the strum: 0 is a block chord, right strums up, "
                "left strums down; Strum Up/Down alternates. Right-click STRM for Strum Shape: Time (up to 250 ms) "
                "or Sync (1/128 to 1/8, follows the tempo), a curve, a velocity tilt and humanize. When a strum "
                "can't fit before the next step it's squeezed: the STRM arc fades and the panel says so.");

        w.heading ("Steps");
        w.text ("Trig conditions (1:2 " + kDash + " 4:4, 1ST, PRE, NEI, FILL and their opposites), ratchets up to "
                "4 hits, step pitch and step velocity, all per step.");

        w.heading ("48 scales");
        w.text ("Scale lock keeps every lane in key; step pitch then moves in scale degrees. Modes, minors, "
                "pentatonics, jazz, symmetric, Messiaen's modes, world scales, Japanese and makam (12-TET).");

        w.heading ("Lanes");
        w.text ("Forward, reverse, ping-pong or random per lane. Mutate flips some steps every loop without "
                "touching the pattern. Mute and solo. Euclidean fill, copy / paste, shift and clear in the lane menu.");

        w.heading ("Play and see");
        w.text ("Latch holds the chord. Fill brings in the FILL steps while held. The keyboard strip shows which "
                "note feeds which lane; click its keys to build a chord. Each lane's playhead shows how far it is "
                "through its loop, and which way it's going.");

        w.heading ("Presets");
        w.text ("One file per preset, sub-folders as categories (any depth) in the preset menu. Save asks for a "
                "name and a folder. The old preset bundle was split into the Legacy folder.");

        w.heading ("Work faster");
        w.text ("Undo / redo for every edit, keyboard shortcuts (see the next tab), drag the MIDI button into the "
                "DAW to get the pattern as a clip, note values shown as notation.");

        w.heading ("Also");
        w.text ("A MIDI FX version of the AU for Logic's MIDI FX slot. Clearer text, a confirm before Reset and "
                "Delete, and fixes for strum timing, saved state and crashes.");
    }

    void writeControls (HelpWriter& w)
    {
        const auto plus = [] (const juce::String& a, const juce::String& b) { return a + kSep + b; };

        w.heading ("Shortcuts  (mouse over a lane)");
        w.key (plus (kCmd, "Z") + " / " + plus (plus (kShift, kCmd), "Z"), "Undo / redo");
        w.key (kLeft + " / " + kRight,                                  "Shift the lane's steps");
        w.key (plus (kShift, kLeft) + " / " + plus (kShift, kRight),    "Shift every lane");
        w.key (kUp + " / " + kDown,                                     "Cycle the lane's direction");
        w.key ("F  R  P  X",                                            "Forward, Reverse, Ping-Pong, Random");
        w.key (plus (kShift, "F R P X"),                                "The same for every lane");

        w.heading ("Mouse");
        w.key ("click pad",            "Step on / off (drag to paint)");
        w.key (plus (kCmd == "Ctrl" ? juce::String ("Ctrl") : juce::String (juce::CharPointer_UTF8 ("\xe2\x8c\x83")), "click pad"),
                                       "Event step: plays by the EVENT chance");
        w.key (plus (kShift, "drag pad"), "Step velocity");
        w.key (plus (kAlt, "drag pad"),   "Step pitch");
        w.key ("right-click pad",      "Condition, ratchet, reset pitch");
        w.key ("click lane number",    "Lane menu (also the arrow under it)");
        w.key ("click note box",       "Solo the lane");
        w.key (plus (kShift, "click note box"), "Mute the lane");
        w.key ("right-click STRM",     "Strum Shape");
        w.key ("right-click A B C D",  "Copy / clear a pattern slot");
        w.key ("click a key",          "Hold / release that note");
        w.key ("right-click keyboard", "Release every clicked note");
        w.key ("drag MIDI",            "The pattern as a MIDI clip");
        w.key ("wheel on a menu",      "Step through its values");

        w.heading ("Lane");
        w.item ("MIDI", "The note the lane plays, ALL for a chord / strum lane. Filled: from MIDI, hollow: latched, ringed number: clicked on the keyboard.");
        w.item ("STRM", "Strum width and direction for chord lanes (right-click: Strum Shape).");
        w.item ("OCT", "Octave, " + kDash + "2 to +2.");
        w.item ("GRIDS", "The steps. Bar above a pad: note length (split for ratchets). Line under: the playhead.");
        w.item ("#GRID", "Number of steps, 2 to 32.");
        w.item ("SPEED", "Length of a step, as a note value.");
        w.item ("DURATION", "Length of each note.");
        w.item ("VEL", "Lane velocity (each step's velocity scales it).");
        w.item ("EVENT", "Chance that Event steps play. Lit only on lanes that have Event steps.");
        w.item ("SHUFFLE", "Swing for the lane.");
        w.item ("DELAY", "Moves the lane earlier or later, up to an eighth note.");
        w.item ("CHAN", "The lane's MIDI channel (when Ch is on).");

        w.heading ("Under the lanes");
        w.item ("Latch", "Keeps the chord after you let go; the next key replaces it.");
        w.item ("Fill", "While held, FILL steps play and !FILL steps rest.");
        w.item ("A B C D", "Pattern slots. Blinks until every lane has switched.");
        w.item ("Key / Scale", "Scale lock for every lane.");
        w.item ("Keyboard", "Held notes in their lane's colour, dots for the notes sounding.");
        w.item ("MIDI", "Drag out the pattern; click for length (1 " + kDash + " 16 bars) and Save as.");
        w.item ("Undo / Redo", "Every edit.");

        w.heading ("Global");
        w.item ("InBSynth", "A simple built-in synth to hear the lanes (not in the MIDI FX version).");
        w.item ("Sorted / FirstIn", "Sorted: held notes go to the lanes low to high. FirstIn: a new note takes the first free lane.");
        w.item ("SHUFFLE", "Swing added to every lane.");
        w.item ("Rnd 1 " + kDash + " 5", "Random steps for that lane.");
        w.item ("MIDI port", "An external MIDI output (used when Ch is on).");
        w.item ("Fixed / Own / Midi Vel", "Velocity from the lane's VEL knob, or from the notes you play.");
        w.item ("RESYNC BAR", "Every this many bars all lanes start again together.");
        w.item ("Row of knobs", "Sets that column for every lane at once.");
        w.item ("Ch", "Per-lane MIDI channels and the external MIDI port.");
        w.item ("Presets", "Menu with folders, " + juce::String (juce::CharPointer_UTF8 ("\xe2\x97\x80 \xe2\x96\xb6")) + " to step, Folder (choose / default / show), Save, Delete.");
        w.item ("Reset", "Lanes, steps and settings back to the defaults (asks first, undoable).");

        w.heading ("Satellite");
        w.text ("Each lane as a ring: its steps around it, the lit marker where it is playing.");
    }

    // Two tabs over one scrolling text view, in the plugin's colours.
    class HelpContent : public juce::Component
    {
    public:
        HelpContent()
        {
            // its own look-and-feel, so the scrollbar is in the plugin's colours too
            look.setColour (juce::ScrollBar::thumbColourId, Theme::accent.withAlpha (0.7f));
            look.setColour (juce::ScrollBar::trackColourId, Theme::well);
            setLookAndFeel (&look);
            for (auto* b : { &newsTab, &controlsTab, &closeButton })
            {
                b->setColour (juce::TextButton::buttonColourId, Theme::surfaceAlt);
                b->setColour (juce::TextButton::textColourOffId, Theme::textSecondary);
                b->setColour (juce::ComboBox::outlineColourId, Theme::hairline);
                addAndMakeVisible (*b);
            }
            newsTab.onClick     = [this] { showTab (0); };
            controlsTab.onClick = [this] { showTab (1); };
            closeButton.onClick = [this] { if (auto* w = findParentComponentOfClass<juce::DialogWindow>()) w->closeButtonPressed(); };

            text.setMultiLine (true, true);
            text.setReadOnly (true);
            text.setCaretVisible (false);
            text.setScrollbarsShown (true);
            text.setColour (juce::TextEditor::backgroundColourId, Theme::well);
            text.setColour (juce::TextEditor::outlineColourId, Theme::hairline);
            text.setColour (juce::TextEditor::focusedOutlineColourId, Theme::hairline);
            text.setBorder ({ 8, 12, 8, 12 });
            addAndMakeVisible (text);
            setSize (600, 560);
            showTab (0);
        }

        void showTab (int tab)
        {
            text.clear();
            HelpWriter w { text };
            if (tab == 0) writeNews (w); else writeControls (w);
            text.moveCaretToTop (false);
            for (auto* b : { &newsTab, &controlsTab })
            {
                const bool on = (b == &newsTab) == (tab == 0);
                b->setColour (juce::TextButton::buttonColourId, on ? Theme::accent : Theme::surfaceAlt);
                b->setColour (juce::TextButton::textColourOffId, on ? Theme::screen : Theme::textSecondary);
            }
        }

        ~HelpContent() override { setLookAndFeel (nullptr); }

        void paint (juce::Graphics& g) override { g.fillAll (Theme::panel); }

        void resized() override
        {
            auto b = getLocalBounds().reduced (10);
            auto tabs = b.removeFromTop (28);
            newsTab.setBounds (tabs.removeFromLeft (tabs.getWidth() / 2).reduced (2, 0));
            controlsTab.setBounds (tabs.reduced (2, 0));
            b.removeFromTop (8);
            closeButton.setBounds (b.removeFromBottom (26).removeFromRight (90));
            b.removeFromBottom (8);
            text.setBounds (b);
        }

    private:
        juce::LookAndFeel_V4 look;
        juce::TextButton newsTab { "What's New in " + juce::String (ProjectInfo::versionString) },
                         controlsTab { "Controls & Shortcuts" }, closeButton { "Close" };
        juce::TextEditor text;
    };
}

void EditorContent::showHelp()
{
    if (helpWindow != nullptr) { helpWindow->toFront (true); return; }
    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned (new HelpContent());
    o.dialogTitle = "TugMidiSeq " + juce::String (ProjectInfo::versionString);
    o.dialogBackgroundColour = Theme::panel;
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = true;
    o.componentToCentreAround = this;   // on the plugin's screen, not the main one
    helpWindow = o.launchAsync();

    // and kept inside that screen when the plugin sits near its edge
    if (helpWindow != nullptr)
        if (auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
            helpWindow->setBounds (helpWindow->getBounds().constrainedWithin (display->userArea));
}
