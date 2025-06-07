/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ColourCombV4AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ColourCombV4AudioProcessorEditor(ColourCombV4AudioProcessor&);
    ~ColourCombV4AudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ColourCombV4AudioProcessor& audioProcessor;

    juce::Rectangle<int> spectrumAnalyzer;
    juce::TextButton cKey{ "C" };
    juce::TextButton cSharpKey{ "C#" };
    juce::TextButton dKey{ "D" };
    juce::TextButton dSharpKey{ "D#" };
    juce::TextButton eKey{ "E" };
    juce::TextButton fKey{ "F" };
    juce::TextButton fSharpKey{ "F#" };
    juce::TextButton gKey{ "G" };
    juce::TextButton gSharpKey{ "G#" };
    juce::TextButton aKey{ "A" };
    juce::TextButton aSharpKey{ "A#" };
    juce::TextButton bKey{ "B" };

    juce::Slider qValKnob;
    juce::Slider makeupKnob;
    juce::Slider mixKnob;

    juce::Label qLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;

    juce::ComboBox functionBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> functionAttachment;

    void knobFactory(float rangeFloor, float rangeCeiling, float increments, std::string suffixVal, float defaultValue, juce::Slider& knob);
    void labelFactory(std::string tag, juce::Label& label);
    void setOnClicks();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourCombV4AudioProcessorEditor)
};