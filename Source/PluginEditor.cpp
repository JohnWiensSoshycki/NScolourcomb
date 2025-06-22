/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ColourCombV4AudioProcessorEditor::ColourCombV4AudioProcessorEditor(ColourCombV4AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(896, 512);

    // Q value knob
    qValKnob.setSliderStyle(juce::Slider::Rotary);
    qValKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    qValKnob.setPopupDisplayEnabled(true, false, this);

    qLabel.setText("Q Knob", juce::dontSendNotification);
    qLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    qLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(qValKnob);
    addAndMakeVisible(qLabel);
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "q", qValKnob);

    // Makeup gain knob
    knobFactory(-60.f, 6.0f, 0.5f, " MakeUp", 0.0f, makeupKnob);
    makeupLabel.setText("Makeup Gain", juce::dontSendNotification);
    makeupLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    makeupLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(makeupKnob);
    addAndMakeVisible(makeupLabel);
    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "makeup", makeupKnob);

    // Mix knob
    knobFactory(0.0f, 100.0f, 0.2f, " Mix", 100.0f, mixKnob);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(mixLabel);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixKnob);

    // Q Function combobox
    functionBox.addItem("Sine", 1);
    functionBox.addItem("Inv Sine", 2);
    functionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "qFunction", functionBox);
    addAndMakeVisible(functionBox);
    functionBox.setSelectedId(1);

    spectrumAnalyzer = juce::Rectangle<int>(40, 50, 400, 200);

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
}

ColourCombV4AudioProcessorEditor::~ColourCombV4AudioProcessorEditor(){}

//==============================================================================
void ColourCombV4AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);
    g.setFont(juce::FontOptions(15.0f));
    g.drawFittedText("ColourComb", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    g.drawRect(spectrumAnalyzer, 2);
}

void ColourCombV4AudioProcessorEditor::resized()
{
    qValKnob.setBounds(480, 330, 100, 100);
    qLabel.setBounds(480, 310, 100, 40);

    makeupKnob.setBounds(610, 330, 100, 100);
    makeupLabel.setBounds(610, 310, 100, 40);

    mixKnob.setBounds(740, 330, 100, 100);
    mixLabel.setBounds(740, 310, 100, 40);

    functionBox.setBounds(470, 50, 200, 50);

    auto xIncrement = 50;
    auto whiteKeyXBase = 60;
    auto whiteKeyYBase = 370;
    auto blackKeyXBase = 85;
    auto blackKeyYBase = 315;

    cKey.setBounds(60, 370, 45, 80);
    cSharpKey.setBounds(85, 315, 45, 80);
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

void ColourCombV4AudioProcessorEditor::labelFactory(std::string tag, juce::Label& label) {
    label.setText(tag, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::black);
    label.setJustificationType(juce::Justification::centred);
}

void ColourCombV4AudioProcessorEditor::knobFactory(float rangeFloor, float rangeCeiling, float increments, std::string suffixVal, float defaultValue, juce::Slider& knob) {
    knob.setSliderStyle(juce::Slider::Rotary);
    knob.setRange(rangeFloor, rangeCeiling, increments);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    knob.setPopupDisplayEnabled(true, false, this);
    knob.setTextValueSuffix(suffixVal);
    knob.setValue(defaultValue);
}

void ColourCombV4AudioProcessorEditor::setOnClicks() {
    
    auto setKey = [this](int keyIndex) {
        audioProcessor.parameters.getParameter("key")->setValueNotifyingHost(keyIndex / 11.0f);
        audioProcessor.toggleActiveFreq(keyIndex);
        audioProcessor.updateVectorProcessorChain();
        //figure out how to toggle the current key off if we try to press a sixth key, might need it as a parameter in the function
    };

    cKey.onClick = [setKey] { setKey(0); };
    cSharpKey.onClick = [setKey] { setKey(1); };
    dKey.onClick = [setKey] { setKey(2); };
    dSharpKey.onClick = [setKey] { setKey(3); };
    eKey.onClick = [setKey] { setKey(4); };
    fKey.onClick = [setKey] { setKey(5); };
    fSharpKey.onClick = [setKey] { setKey(6); };
    gKey.onClick = [setKey] { setKey(7); };
    gSharpKey.onClick = [setKey] { setKey(8); };
    aKey.onClick = [setKey] { setKey(9); };
    aSharpKey.onClick = [setKey] { setKey(10); };
    bKey.onClick = [setKey] { setKey(11); };
}

void ColourCombV4AudioProcessorEditor::setToggleable() {
    cKey.setClickingTogglesState(true);
    cSharpKey.setClickingTogglesState(true);
    dKey.setClickingTogglesState(true);
    dSharpKey.setClickingTogglesState(true);
    eKey.setClickingTogglesState(true);
    fKey.setClickingTogglesState(true);
    fSharpKey.setClickingTogglesState(true);
    gKey.setClickingTogglesState(true);
    gSharpKey.setClickingTogglesState(true);
    aKey.setClickingTogglesState(true);
    aSharpKey.setClickingTogglesState(true);
    bKey.setClickingTogglesState(true);
}