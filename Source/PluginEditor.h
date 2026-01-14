/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilmstripLookAndFeel.h"

//==============================================================================
/**
*/
class ColourCombV4AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    ColourCombV4AudioProcessorEditor(ColourCombV4AudioProcessor&);
    ~ColourCombV4AudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    //void timerCallback();

private:
    ColourCombV4AudioProcessor& audioProcessor;
    
    FilmstripLookAndFeel qValLNF;     // filmstrip for q knob
    FilmstripLookAndFeel gainLNF;
    FilmstripLookAndFeel mixLNF;
    FilmstripLookAndFeel focusLNF;
    FilmstripLookAndFeel lowCutLNF;
    FilmstripLookAndFeel highCutLNF;
    

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
    
    std::vector<juce::TextButton*> allKeys = {&cKey,&cSharpKey, &dKey, &dSharpKey,
     &eKey,&fKey,&fSharpKey,&gKey,&gSharpKey,&aKey,&aSharpKey,&bKey};
    
    juce::Slider qValKnob;
    juce::Slider makeupKnob;
    juce::Slider mixKnob;
    juce::Slider focusSlider;
    juce::Slider lowCutKnob;
    juce::Slider highCutKnob;

    juce::Label qLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;
    juce::Label focusLabel;
    
    juce::Label numActiveNotesLabel;
    juce::Label filterVectorLabel;
    juce::Label rebuildLabel;

    juce::ComboBox functionBox;
    juce::Image backgroundBaseplate;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> functionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> focusAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    


    void knobFactory(float rangeFloor, float rangeCeiling, float increments, std::string suffixVal, float defaultValue, juce::Slider& knob);
    void labelFactory(std::string tag, juce::Label& label);
    void setOnClicks();
    void setToggleable();
    bool isItOn(int keyNum);
    
    std::string vectorToString();
    //juce::TimedCallback myTimedCallback;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourCombV4AudioProcessorEditor)
};

