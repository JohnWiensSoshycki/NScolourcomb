/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FilmstripLookAndFeel.h"
#include "BinaryData.h"
#include <bit>

static juce::Image loadImageFromBinary(const void* data, int size)
{
    return juce::ImageFileFormat::loadFrom(data, size);
}

//==============================================================================
ColourCombV4AudioProcessorEditor::ColourCombV4AudioProcessorEditor(ColourCombV4AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),

    qValLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                                 BinaryData::knob_strip_pngSize),
             /*frameW*/72, /*frameH*/72, /*numFrames*/100, /*vertical*/ false),

    gainLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                                 BinaryData::knob_strip_pngSize),
             72, 72, 100, false),
    mixLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                             BinaryData::knob_strip_pngSize),
         72, 72, 100, false),
    focusLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                         BinaryData::knob_strip_pngSize),
         72, 72, 100, false),
    lowCutLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                     BinaryData::knob_strip_pngSize),
         72, 72, 100, false),
    highCutLNF (loadImageFromBinary(BinaryData::knob_strip_png,
                     BinaryData::knob_strip_pngSize),
         72, 72, 100, false)
{
    setSize(512, 512);
    //old size 512 512 -> 512 542
    
    
    
    // Q value knob
    qValKnob.setSliderStyle(juce::Slider::Rotary);
    //changing (juce::Slider::NoTextBox, false, 90, 0) to (juce::Slider::NoTextBox, false, 0, 0)
    qValKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    qValKnob.setPopupDisplayEnabled(true, true, this);
    qValKnob.setLookAndFeel(&qValLNF);
    
    

    qLabel.setText("Q Knob", juce::dontSendNotification);
    qLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    qLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(qValKnob);
    addAndMakeVisible(qLabel);
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "q", qValKnob);

    // Makeup gain knob
    knobFactory(-60.f, 6.0f, 0.5f, " MakeUp", 0.0f, makeupKnob);
    makeupLabel.setText("Makeup Gain", juce::dontSendNotification);
    makeupLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    makeupLabel.setJustificationType(juce::Justification::centred);
    makeupKnob.setLookAndFeel(&gainLNF);
    addAndMakeVisible(makeupKnob);
    addAndMakeVisible(makeupLabel);
    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "makeup", makeupKnob);

    //focusSlider
    knobFactory(0.f, 100.0f, 1.0f, " Focus", 0.0f, focusSlider);
    //focusSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    focusSlider.setLookAndFeel(&focusLNF);
    addAndMakeVisible(focusSlider);
    focusAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "focusValue", focusSlider);
    labelFactory("Focus", focusLabel);
    addAndMakeVisible(focusLabel);

    // Mix knob
    knobFactory(0.0f, 100.0f, 0.2f, " Mix", 100.0f, mixKnob);
    mixKnob.setLookAndFeel(&mixLNF);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(mixLabel);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixKnob);

    // Q Function combobox
    functionBox.addItem("Sine", 1);
    functionBox.addItem("Inv Sine", 2);
    functionBox.addItem("Constant", 3);
    functionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "qFunction", functionBox);
    addAndMakeVisible(functionBox);
    //functionBox.setSelectedId(1);

    
    //lowCut knob
    knobFactory(0.f, 8000.f, 10.f, " lowCut", 0.f, lowCutKnob);
    lowCutKnob.setLookAndFeel(&lowCutLNF);
    labelFactory("Low Cut", lowCutLabel);
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowCut", lowCutKnob);
    addAndMakeVisible(lowCutKnob);
    addAndMakeVisible(lowCutLabel);
    
    //highCut knob
    knobFactory(8000.f, 22000.f, 10.0f, " highCut", 22000.f, highCutKnob);
    highCutKnob.setLookAndFeel(&highCutLNF);
    labelFactory("High Cut", highCutLabel);
    highCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "highCut", highCutKnob);
    addAndMakeVisible(highCutKnob);
    addAndMakeVisible(highCutLabel);

    
    
    
    
    setOnClicks();
    setToggleable();

    // Key buttons
    
    addAndMakeVisible(cKey);
    addAndMakeVisible(dKey);
    addAndMakeVisible(eKey);
    addAndMakeVisible(fKey);
    addAndMakeVisible(gKey);
    addAndMakeVisible(aKey);
    addAndMakeVisible(bKey);

    addAndMakeVisible(cSharpKey);
    addAndMakeVisible(dSharpKey);
    addAndMakeVisible(fSharpKey);
    addAndMakeVisible(gSharpKey);
    addAndMakeVisible(aSharpKey);
   
    //Testing Labels
    numActiveNotesLabel.setText(vectorToString(), juce::dontSendNotification);
    numActiveNotesLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    //addAndMakeVisible(numActiveNotesLabel);
    rebuildLabel.setText(audioProcessor.lastRebuild, juce::dontSendNotification);
    rebuildLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    //addAndMakeVisible(rebuildLabel);
    
    
    backgroundBaseplate = loadImageFromBinary(BinaryData::Baseplate_png, BinaryData::Baseplate_pngSize);
    logo = loadImageFromBinary(BinaryData::ColourcombLogoFirstAttempt_png, BinaryData::ColourcombLogoFirstAttempt_pngSize);
    startTimerHz(3);
    
    
}

ColourCombV4AudioProcessorEditor::~ColourCombV4AudioProcessorEditor(){}



















//==============================================================================
void ColourCombV4AudioProcessorEditor::paint(juce::Graphics& g)
{

    g.fillAll(juce::Colours::black);
    g.drawImageAt(backgroundBaseplate,0,0);
    /*
    g.setColour(juce::Colours::black);
    g.fillRect(0, 0, 512, 50);
     */
    g.drawImageAt(logo,0,10);
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.setFont(juce::FontOptions(15.0f));
}

void ColourCombV4AudioProcessorEditor::resized()
{
    //the bonus +10 for the top and second row are to account for logo
    int topRow = 40 + 10;
    qValKnob.setBounds(80, topRow, 100, 100);
    qLabel.setBounds(80, topRow+80, 100, 40);

    makeupKnob.setBounds(210, topRow, 100, 100);
    makeupLabel.setBounds(210, topRow+80, 100, 40);

    mixKnob.setBounds(340, topRow, 100, 100);
    mixLabel.setBounds(340, topRow+80, 100, 40);
    
    int secondRow = topRow + 92 + 10;

    focusSlider.setBounds(80, secondRow, 100, 100);
    focusLabel.setBounds(80, secondRow+80, 100, 40);
    
    lowCutKnob.setBounds(210, secondRow, 100, 100);
    lowCutLabel.setBounds(210, secondRow+80, 100, 40);
    
    highCutKnob.setBounds(340, secondRow, 100, 100);
    highCutLabel.setBounds(340, secondRow+80, 100, 40);
    
    //any +30 at the end are to test the total size
    int thirdRow= 380+25 + 40;
    functionBox.setBounds(156, thirdRow, 200, 50);

    numActiveNotesLabel.setBounds(350, 10, 100, 50);
    filterVectorLabel.setBounds(350, 360, 100, 80);

    auto xIncrement = 50;
    auto whiteKeyXBase = 85;
    auto whiteKeyYBase = 240+75+30;
    auto blackKeyXBase = 110;
    auto blackKeyYBase = 185+75+30;

    cKey.setBounds(whiteKeyXBase, whiteKeyYBase, 45, 80);
    cSharpKey.setBounds(blackKeyXBase, blackKeyYBase, 45, 80);
    dKey.setBounds(whiteKeyXBase + xIncrement, whiteKeyYBase, 45, 80);
    dSharpKey.setBounds(blackKeyXBase + xIncrement, blackKeyYBase, 45, 80);
    eKey.setBounds(whiteKeyXBase + 2 * xIncrement, whiteKeyYBase, 45, 80);
    fKey.setBounds(whiteKeyXBase + 3 * xIncrement, whiteKeyYBase, 45, 80);
    fSharpKey.setBounds(blackKeyXBase + 3 * xIncrement, blackKeyYBase, 45, 80);
    gKey.setBounds(whiteKeyXBase + 4 * xIncrement, whiteKeyYBase, 45, 80);
    gSharpKey.setBounds(blackKeyXBase + 4 * xIncrement, blackKeyYBase, 45, 80);
    aKey.setBounds(whiteKeyXBase + 5 * xIncrement, whiteKeyYBase, 45, 80);
    aSharpKey.setBounds(blackKeyXBase + 5 * xIncrement, blackKeyYBase, 45, 80);
    bKey.setBounds(whiteKeyXBase + 6 * xIncrement, whiteKeyYBase, 45, 80);
}















//========Factories===============
void ColourCombV4AudioProcessorEditor::labelFactory(std::string tag, juce::Label& label) {
    label.setText(tag, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType(juce::Justification::centred);
}

void ColourCombV4AudioProcessorEditor::knobFactory(float rangeFloor, float rangeCeiling, float increments, std::string suffixVal, float defaultValue, juce::Slider& knob) {
    knob.setSliderStyle(juce::Slider::Rotary);

    knob.setPopupDisplayEnabled(true, true, this);
    knob.setTextBoxIsEditable(true);
    
    knob.setTextValueSuffix(suffixVal);
    //knob.setValue(defaultValue);
}
























//==========text key onclick setup code starts here================
void ColourCombV4AudioProcessorEditor::setOnClicks() {
     
    auto setKey = [this](int keyIndex, juce::TextButton &button) {
        //audioProcessor.toggleActiveFreq(1<<keyIndex, keyIndex);


        
        if (auto* p= audioProcessor.parameters.getRawParameterValue("activeKeyMask")){
            int oldMask = (int) p->load();
            bool isOn = oldMask & (1 << keyIndex);

            int activeCount = __builtin_popcount(oldMask);
            if ((activeCount < 8 && !isOn) || isOn){
                int oldMask = (int) p->load();
                int bit = 1 << keyIndex;   // keyIndex = 1 for C#
                int newMask = oldMask ^ bit; // toggle
                auto* param= audioProcessor.parameters.getParameter("activeKeyMask");
                param->setValueNotifyingHost(param->convertTo0to1((float)newMask));
            }
            
        }
        
        
        const bool shouldBeOn = isItOn(keyIndex);
        button.setToggleState(shouldBeOn, juce::dontSendNotification);
        numActiveNotesLabel.setText(vectorToString(), juce::dontSendNotification);
        if (shouldBeOn)
            audioProcessor.parameters.getParameter("key")->setValueNotifyingHost(keyIndex / 11.0f);
        
        

      
    };
    cKey.onClick = [this, setKey] {setKey(0, cKey);};
    cSharpKey.onClick = [this,setKey] {setKey(1, cSharpKey);};
    dKey.onClick = [this,setKey] {setKey(2, dKey);};
    dSharpKey.onClick = [this,setKey] {setKey(3, dSharpKey);};
    eKey.onClick = [this,setKey] {setKey(4, eKey);};
    fKey.onClick = [this,setKey] {setKey(5, fKey);};
    fSharpKey.onClick = [this,setKey] {setKey(6, fSharpKey);};
    gKey.onClick = [this,setKey] {setKey(7, gKey);};
    gSharpKey.onClick = [this,setKey] {setKey(8, gSharpKey);};
    aKey.onClick = [this,setKey] {setKey(9, aKey);};
    aSharpKey.onClick = [this,setKey] {setKey(10, aSharpKey);};
    bKey.onClick = [this,setKey] {setKey(11, bKey);};
    

 
}

void ColourCombV4AudioProcessorEditor::setToggleable() {
    cKey.setClickingTogglesState(false);
    cSharpKey.setClickingTogglesState(false);
    dKey.setClickingTogglesState(false);
    dSharpKey.setClickingTogglesState(false);
    eKey.setClickingTogglesState(false);
    fKey.setClickingTogglesState(false);
    fSharpKey.setClickingTogglesState(false);
    gKey.setClickingTogglesState(false);
    gSharpKey.setClickingTogglesState(false);
    aKey.setClickingTogglesState(false);
    aSharpKey.setClickingTogglesState(false);
    bKey.setClickingTogglesState(false);
}

std::string ColourCombV4AudioProcessorEditor::vectorToString(){
    std::string vectorString = "";
    for (int x=0; x<audioProcessor.activeFreqs.size(); x++){
        vectorString = vectorString + std::to_string(audioProcessor.activeFreqs[x]) + ",";
    }
    return vectorString;
}

bool ColourCombV4AudioProcessorEditor::isItOn(int keyNum){
    if (audioProcessor.activeFreqs[keyNum] == 1){
        return true;
    }
    return false;
}

 


void ColourCombV4AudioProcessorEditor::timerCallback(){
    
    auto* p= audioProcessor.parameters.getRawParameterValue("activeKeyMask");
    if (!p) return;
    
    int mask = (int) p->load();
    for (int x=0; x<12; x++){
        (allKeys[x])->setToggleState(mask & (1 << x), juce::dontSendNotification);
    }
}



