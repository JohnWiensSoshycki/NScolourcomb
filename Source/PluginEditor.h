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
    
    class EditableFilmstripSlider : public juce::Slider
    {
    public:
        std::unique_ptr<juce::TextEditor> valueEditor;
        EditableFilmstripSlider()
        {
            setTextBoxStyle (juce::Slider::NoTextBox, false, 50, 20);
            setTextBoxIsEditable (true);
            
            
        }

        void mouseDoubleClick (const juce::MouseEvent& e) override
        {
            showEditor();
        }
        
        void showEditor(){
            if (valueEditor != nullptr)
                return;
            
            valueEditor = std::make_unique<juce::TextEditor>();
            addAndMakeVisible(*valueEditor);
            
            valueEditor->setText(juce::String(getValue()), false);
            valueEditor->setJustification(juce::Justification::centred);
            valueEditor->selectAll();
            valueEditor->grabKeyboardFocus();
            
            valueEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2b2b2b));
            
            auto compRect = getLocalBounds();
            valueEditor->setBounds(compRect.getX() + (compRect.getWidth()*0.2),
            compRect.getY() + (compRect.getHeight()*0.3), compRect.getWidth()*0.6, compRect.getHeight()*0.4);
            
            //return values
            valueEditor->onReturnKey = [this](){
                commitEditorValue();
                hideEditor();
            };
            valueEditor->onEscapeKey = [this](){
                hideEditor();
            };
            valueEditor->onFocusLost = [this](){
              hideEditor();
            };
            
        
        }
        
        void commitEditorValue(){
            if (valueEditor == nullptr)
                return;
            
            auto text = valueEditor->getText();
            double newValue = text.getDoubleValue();
            
            newValue = juce::jlimit(getMinimum(), getMaximum(), newValue);
            setValue(newValue, juce::sendNotificationSync);
            
            //hideEditor();
        }
        
        void hideEditor(){
            if (valueEditor != nullptr){
                valueEditor.reset();
            }
        }
        
        
    };

private:
    ColourCombV4AudioProcessor& audioProcessor;
    
    FilmstripLookAndFeel qValLNF;     // filmstrip for q knob
    FilmstripLookAndFeel gainLNF;
    FilmstripLookAndFeel mixLNF;
    FilmstripLookAndFeel focusLNF;
    FilmstripLookAndFeel lowCutLNF;
    FilmstripLookAndFeel highCutLNF;
    


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
    
    EditableFilmstripSlider qValKnob;
    EditableFilmstripSlider makeupKnob;
    EditableFilmstripSlider mixKnob;
    EditableFilmstripSlider focusSlider;
    EditableFilmstripSlider lowCutKnob;
    EditableFilmstripSlider highCutKnob;
    
    //juce::Slider mixKnob;
    /*
     juce::Slider qValKnob;
     juce::Slider makeupKnob;
    juce::Slider focusSlider;
    juce::Slider lowCutKnob;
    juce::Slider highCutKnob;
     */

    juce::Label qLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;
    juce::Label focusLabel;
    
    juce::Label numActiveNotesLabel;
    juce::Label filterVectorLabel;
    juce::Label rebuildLabel;

    juce::Label lowCutLabel;
    juce::Label highCutLabel;
    
    juce::ComboBox functionBox;
    juce::Image backgroundBaseplate;
    juce::Image logo;
    
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


