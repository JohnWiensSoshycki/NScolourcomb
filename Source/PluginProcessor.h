/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
#include <vector>
#include <cmath>

//==============================================================================
/**
*/
class ColourCombV4AudioProcessor : public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener,  private juce::AsyncUpdater
{
public:
    ColourCombV4AudioProcessor();
    ~ColourCombV4AudioProcessor() noexcept override;

    // Core plugin lifecycle
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // GUI
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Metadata
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Program state
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // State persistence
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS
    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Parameter access
    float getMixValue() const;
    float getMakeupGainValue() const;
    float getQValue() const;
    int getCurrentKey() const;
    int getCurrentFunction() const;
    float getFocusValue() const;
    int getCascadeValue() const;
    int getMaskValue() const;

    //void setTargetFrequencies(const std::vector<float>& freqs);
    void setFrequencyBounds(float floorhz, float ceilinghz);

    // Listener callback
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    std::vector<int> activeFreqs = { 0,0,0,0,0,0,0,0,0,0,0,0,0 };
    //void toggleActiveFreq(int x, int indexPosition);
    int numOfActiveFreqs = 0;
    //was 1
    //void updateVectorProcessorChain();
    juce::dsp::ProcessSpec spec;
    
    float determineQ (float freq, float qRatio, int funcType);
    void createBank();
    std::string lastRebuild = "No Rebuild";
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColourCombV4AudioProcessor)

        juce::dsp::ProcessorChain<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>> filterChainLeft, filterChainRight;
   

  
    
    float frequencyFloor = 200.0f;
    float frequencyCeiling = 10000.0f;
    

 

    std::vector<std::vector<float>> noteFrequencies = {
        {130.81f, 261.63f, 523.25f, 1046.50f, 2093.00f, 4186.01f, 8372.02f},
        {138.59f, 277.18f, 554.37f, 1108.73f, 2217.46f, 4434.92f, 8869.84f},
        {146.83f, 293.66f, 587.33f, 1174.66f, 2349.32f, 4698.64f, 9397.27f},
        {155.56f, 311.13f, 622.25f, 1244.51f, 2489.02f, 4978.03f, 9956.06f},
        {164.81f, 329.63f, 659.26f, 1318.51f, 2637.02f, 5274.04f, 10548.08f},
        {174.61f, 349.23f, 698.46f, 1396.91f, 2793.83f, 5587.65f, 11175.30f},
        {185.00f, 369.99f, 739.99f, 1479.98f, 2959.96f, 5919.91f, 11839.82f},
        {196.00f, 392.00f, 783.99f, 1567.98f, 3135.96f, 6271.93f, 12543.86f},
        {207.65f, 415.30f, 830.61f, 1661.22f, 3322.44f, 6644.88f, 13289.75f},
        {220.00f, 440.00f, 880.00f, 1760.00f, 3520.00f, 7040.00f, 14080.00f},
        {233.08f, 466.16f, 932.33f, 1864.66f, 3729.31f, 7458.62f, 14917.24f},
        {246.94f, 493.88f, 987.77f, 1975.53f, 3951.07f, 7902.13f, 15804.26f}
    };

    //vector chain for multiplenotes
    bool useVectorChain = true;  // Set this from UI or private test toggle
    
    
    
    
    using Notch = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                     juce::dsp::IIR::Coefficients<float>>;
    
    using Shelf = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Coefficients<float>>;
    
    std::vector<std::vector<Notch>> filterBank;
    std::atomic<int>   cascadeFactor {1}; // 1 or 2

  
    void handleAsyncUpdate() override;
    void createFilterBank();
    std::vector<Notch> createHarmonicGroup(int targetNote);
    void updateHarmonicGroup(int targetNote);
    void rebuild();
    void setActiveKeyMask(int mask);
 
    
    Shelf lowerShelf;
    Shelf upperShelf;
    std::atomic<bool> shouldRebuild = false;
    std::atomic<bool> dspReady = false;
    std::atomic<bool> isPrepared = false;
    std::atomic<bool> setStateFlag = false;
    int activeKeyMask = 0;
        
    

};





