/*
  ==============================================================================

    Grids.cpp
    Created: 13 May 2022 3:35:51pm
    Author:  Tuğrul Akyüz

  ==============================================================================
*/


#include "Grids.h"
 
extern ChangeBroadcaster myGridChangeListener;


Grids::Grids(TugMidiSeqAudioProcessor& p,int line)  : audioProcessor (p) , stepArrow("",
                                                                                   0.0f,
                                                                                   juce::Colours::orange)
{
    myLine = line;
    step = 0;
    myLineLabel.setText(std::to_string(myLine + 1), juce::NotificationType::dontSendNotification);
    myLineLabel.setColour(juce::Label::ColourIds::textColourId, colourarray[myLine]);
    myLineLabel.setJustificationType(Justification::right);
    
    addAndMakeVisible(myLineLabel);
    addAndMakeVisible(octaveSlider);
    addAndMakeVisible(stepArrow);
    addAndMakeVisible(gridNumberSlider);
    addAndMakeVisible(gridSpeedCombo);
    
    octaveSlider.setSliderStyle(juce::Slider::LinearVertical);
    octaveSlider.setColour(Slider::textBoxOutlineColourId , juce::Colours::black.withAlpha(0.0f));
    //octaveSlider.gette
    octaveSlider.setLookAndFeel(&myLookAndFeel);
    octaveSlider.setTextBoxStyle(juce::Slider::TextBoxRight, true, 40, 25);
    octaveSlider.setColour(Slider::textBoxTextColourId,  Colours::orange);
    addAndMakeVisible(midiInNote);
    midiInNote.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    gridSpeedCombo.setLookAndFeel(&myLookAndFeel);
    //gridSpeedCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::orange);
    addAndMakeVisible(gridDurationCombo);
    gridDurationCombo.setLookAndFeel(&myLookAndFeel);
    gridDurationCombo.getLookAndFeel().setColour (ComboBox::textColourId, Colours::lightgrey);
    gridDurationCombo.getLookAndFeel().setColour (PopupMenu::backgroundColourId, Colours::black);
    gridDurationCombo.getLookAndFeel().setColour (ComboBox::backgroundColourId, Colours::black);
    
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
        addAndMakeVisible (buttons.add (new MultiStateButton ()));
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
    
    tmp_s.clear();
    tmp_s << valueTreeNames[EVENT] << line;
    gridEventSliderAttachment =  std::make_unique  <AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.valueTreeState, tmp_s, gridEventSlider);
    
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

    octaveSlider.setColour (Slider::ColourIds::backgroundColourId, Colours::lightgrey);
    octaveSlider.setColour (Slider::ColourIds::trackColourId, Colours::lightgrey);
    octaveSlider.setColour (Slider::ColourIds::thumbColourId, Colours::orange);
    //Slider::ColourIds::thumbColourId
    //octaveSlider.setRange(-2, 2,1);
   // octaveSlider.setValue(0);
   
    audioProcessor.valueTreeState.addParameterListener(valueTreeNames[SHUFFLE], this);
 
    myGridChangeListener.addChangeListener(this);
    
}
void Grids::paint (juce::Graphics& g)
{
   
    //g.fillAll (  Colour(0xff404040));
    g.fillAll (  Colours::darkgrey.withAlpha(0.0f));
    auto y = getLocalBounds().getHeight();
    auto x = getLocalBounds().getWidth();
    g.setColour(juce::Colours::orange.withAlpha(0.7f));
    g.drawLine(0, y, x, y, 2);
    if(myLine == 4)
        g.drawLine(0, 0, x, 0, 2);
   

    g.setColour(juce::Colours::orange.withAlpha(0.7f));
    g.drawLine(octaveSlider.getRight(), 4,octaveSlider.getRight(), getHeight()- 4,1);
    g.drawLine(gridNumberSlider.getX() -4  ,4,gridNumberSlider.getX() - 4, getHeight() - 4,1);
    
    /*
    if(midiInNote.getButtonText() != "")
    {
        auto b = midiInNote.getBounds();
        ColourGradient gf(Colours::lightgreen , b.getCentreX(),  b.getCentreY(), Colours::lightgreen.withAlpha(0.0f), b.getX()-20,b.getY()-20, true);
        g.setGradientFill(gf);
        
        g.fillRect(b.expanded(10, 10));
    }
     */
   // if(ratio >= 0)
   // g.fillRect(  subGrids->getX(), 0, (int)(ratio*(subGrids->getWidth()-20)), getHeight());
    
}
void Grids::resized()
{
    float marjin =  1;
 
    auto area = getLocalBounds();
    gridDelaySlider.setBounds( area.removeFromRight(50));
    gridShuffleSlider.setBounds( area.removeFromRight(50));
    gridEventSlider.setBounds( area.removeFromRight(50));
    gridVelSlider.setBounds( area.removeFromRight(50));
    gridDurationCombo.setBounds(area.removeFromRight(56).reduced(2,8)/*.withHeight(area.getHeight()-10)*/);
    gridSpeedCombo.setBounds(area.removeFromRight(56).reduced(2,8)/*.withHeight(area.getHeight()-)*/);
    gridNumberSlider.setBounds( area.removeFromRight(50)/*.withHeight(area.getHeight()+5)*/);
    
    
    //auto tmp =
    myLineLabel.setBounds(area.removeFromLeft(25));
    midiInNote.setBounds(area.removeFromLeft(40).reduced(0,10));
    octaveSlider.setBounds(area.removeFromLeft(50));
    
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
        auto r = audioProcessor.getSfuffleRatios(myLine,i);
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
    float ratio =   audioProcessor.getGridContinousRatio(myLine);
    
     DropShadow ds(juce::Colours::lightgreen.withAlpha(1.0f), 2, {0,0});
    float thickness = 2;
    Rectangle<int>  area;
    if(ratio >= 0)
    {
        area = Rectangle<int>  (10,getHeight() -7,ratio*(getWidth() -20),2);
 
        ds.drawForRectangle(g, area);
        
      //  g.drawLine(10 + ratio*(getWidth() -20), getHeight() -10 , 10 + ratio*(getWidth() -20), getHeight() - 5);
         Line<float> line(10 + ratio*(getWidth() -20),getHeight() -6, 20 + ratio*(getWidth() -20) ,getHeight() -6);
        g.drawArrow(line, 4, 4, 5);
    
    }
    float len = audioProcessor.getGridSampleLen( myLine);
    int  s_x = 0;
    Rectangle<int> area2;
    bool passed = false;
    for(int i = ownerGrid.getParam(GETNUMOF) - 1 ; i >= 0; i--)
    {
        s_x = ownerGrid.getParam(GETCOORDOFBUTTON,i);
        auto sr = audioProcessor.getSfuffleRatios(myLine,  i);
        if(s_x != -1)
        {
            
            g.setColour(colourarray[myLine].withAlpha(0.6f));
            area2 = Rectangle<int> (s_x, 4 ,  (int)(0.95*len*sr*getWidth()), 5);
            g.fillRect(area2);
            if(area.getRight() < area2.getRight() && area.getRight() > area2.getX()  && audioProcessor.midiState[myLine] == true && passed == false)
            {
                /*
                ColourGradient gf(colourarray[myLine],  area2.getCentreX(),  area2.getCentreY(), colourarray[myLine].withAlpha(0.0f), area2.getX()-10,-2, true);
                g.setGradientFill(gf);
                
                g.fillRect(area2.expanded(20, 2));
                 */
                g.setColour(colourarray[myLine].withAlpha(1.0f));
                g.fillRect(area2);
                passed = true;
                
                
            }
         //   g.setColour(colourarray[myLine].withAlpha(0.6f));
         //   g.drawRect(area2);
        }

    }
    
    
}

void  SubGrids::resized ()
{
    
    
}
