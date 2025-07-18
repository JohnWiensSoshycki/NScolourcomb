/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ColourCombV4AudioProcessor::ColourCombV4AudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    parameters.addParameterListener("q", this);
    parameters.addParameterListener("mix", this);
    parameters.addParameterListener("makeup", this);
    parameters.addParameterListener("key", this);
    parameters.addParameterListener("qFunction", this);
}

ColourCombV4AudioProcessor::~ColourCombV4AudioProcessor(){}
//==============================================================================
const juce::String ColourCombV4AudioProcessor::getName() const{return JucePlugin_Name;}
bool ColourCombV4AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ColourCombV4AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ColourCombV4AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ColourCombV4AudioProcessor::getTailLengthSeconds() const{return 0.0;}
int ColourCombV4AudioProcessor::getNumPrograms() { return 1; }
int ColourCombV4AudioProcessor::getCurrentProgram() { return 0; }
void ColourCombV4AudioProcessor::setCurrentProgram(int index) {}
const juce::String ColourCombV4AudioProcessor::getProgramName(int index) { return {}; }
void ColourCombV4AudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
void ColourCombV4AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    //juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    filterChainLeft.prepare(spec);
    filterChainRight.prepare(spec);
    

    setFrequencyBounds(400.0f, 4000.0f);
    setTargetFrequencies(noteFrequencies[getCurrentKey()]);
    updateAllFilters();
}

void ColourCombV4AudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ColourCombV4AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif







void ColourCombV4AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    //*****fixedTemplateProcess*************
    if (useVectorChain == false) {
        juce::dsp::AudioBlock<float> block(buffer);
        if (buffer.getNumChannels() >= 1)
            filterChainLeft.process(juce::dsp::ProcessContextReplacing<float>(block.getSingleChannelBlock(0)));
        if (buffer.getNumChannels() >= 2)
            filterChainRight.process(juce::dsp::ProcessContextReplacing<float>(block.getSingleChannelBlock(1)));
    }
    //*******VectorChainProcess**********
    else if (useVectorChain == true) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        for (auto& filter : vectorProcessorChain) {
            filter.process(context);
        }
    }


    float wet = getMixValue();
    float dry = 1.0f - wet;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.applyGain(ch, 0, buffer.getNumSamples(), wet);
        buffer.addFrom(ch, 0, dryBuffer, ch, 0, dryBuffer.getNumSamples(), dry);
    }

    buffer.applyGain(juce::Decibels::decibelsToGain(getMakeupGainValue()));
}

//==============================================================================
bool ColourCombV4AudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ColourCombV4AudioProcessor::createEditor() { return new ColourCombV4AudioProcessorEditor(*this); }

//==============================================================================
void ColourCombV4AudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream(destData, true).writeString(parameters.state.toXmlString());
}

void ColourCombV4AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree tree = juce::ValueTree::fromXml(juce::String::createStringFromData(data, sizeInBytes));
    if (tree.isValid()) parameters.state = tree;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ColourCombV4AudioProcessor();
}








//*****************************
//**********GETTERS****************
float ColourCombV4AudioProcessor::getMixValue() const {
    return parameters.getRawParameterValue("mix")->load() / 100.0f;
}
float ColourCombV4AudioProcessor::getMakeupGainValue() const {
    return parameters.getRawParameterValue("makeup")->load();
}
float ColourCombV4AudioProcessor::getQValue() const {
    return parameters.getRawParameterValue("q")->load();
}
int ColourCombV4AudioProcessor::getCurrentKey() const {
    return static_cast<int>(parameters.getRawParameterValue("key")->load());
}
int ColourCombV4AudioProcessor::getCurrentFunction() const {
    return static_cast<int>(parameters.getRawParameterValue("qFunction")->load());
}

//*********EXTRA__SETTERS*****
void ColourCombV4AudioProcessor::setTargetFrequencies(const std::vector<float>& freqs) {
    currentFrequencies = freqs;
}

void ColourCombV4AudioProcessor::setFrequencyBounds(float floorhz, float ceilinghz) {
    frequencyFloor = floorhz;
    frequencyCeiling = ceilinghz;
}



//**********AVPTS__PARAMETERS*********
void ColourCombV4AudioProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    if (parameterID == "q" || parameterID == "mix" || parameterID == "makeup" || parameterID == "key" || parameterID == "qFunction") {
        //std::cout << "Parameter changed: " << parameterID << " = " << newValue << std::endl;
        //juce::Logger::writeToLog("Q changed to: " + juce::String(getQValue()));
        setTargetFrequencies(noteFrequencies[getCurrentKey()]);
        updateAllFilters();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ColourCombV4AudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("q", "Q", juce::NormalisableRange<float>(10.0f, 300.0f, 2.0f), 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("makeup", "Makeup", juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("key", "Key", juce::StringArray({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }), 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("qFunction", "Q Function", juce::StringArray({ "Sine", "Inv Sine" }), 0));

    return { params.begin(), params.end() };
}

//***********FILTER__UPDATES******
void ColourCombV4AudioProcessor::updateAllFilters() {
    updateFilterChainForChannel(filterChainLeft, currentFrequencies);
    updateFilterChainForChannel(filterChainRight, currentFrequencies);
}

void ColourCombV4AudioProcessor::resetFilters() {
    filterChainLeft.reset();
    filterChainRight.reset();
}

void ColourCombV4AudioProcessor::updateFilterChainForChannel(
    juce::dsp::ProcessorChain<
    juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Filter<float>
    >& chain,
    const std::vector<float>& freqs)
{
    //juce::Logger::writeToLog("function changing to: " + juce::String(getCurrentFunction()));
    updateFilterChainRecursive(chain, freqs, getSampleRate(), getQValue(), getCurrentFunction());
}







//****************MultiNoteUpdateVectorProcessChain**********
void ColourCombV4AudioProcessor::updateVectorProcessorChain() {
    vectorProcessorChain.clear();
    //filter through the thirteen possible keynotes
    for (int keyIndex = 0; keyIndex < activeFreqs.size(); ++keyIndex) {
        //if a key note is 1, active, we create a filter for its harmonics
        if (activeFreqs[keyIndex] == 1) {
            //loop thorugh all the possible harmonics that we have stored in the noteFrequencyTable
            for (int harmonicIndex = 0; harmonicIndex < 6; ++harmonicIndex) {
                auto specificFreq = noteFrequencies[keyIndex][harmonicIndex];

                //so long as the harmonic is range make a filter for it
                if (frequencyFloor <= specificFreq && specificFreq <= frequencyCeiling) {
                    // Add filter for this specificFreq here
                    float qratio = getQValue();
                    float q = 10;
                    auto pi = juce::MathConstants<float>::pi;
                    if (getCurrentFunction() == 0) {
                        float freqRad = ((specificFreq - 15000) / 10000) * (1 / 180) * pi;
                        float freqMapping = ((specificFreq / 1.6) - ((5000 * sin(freqRad)) + 5000)) / qratio;
                        q = juce::jlimit(0.2f, 30.0f, freqMapping);
                    }
                    else if (getCurrentFunction() == 1) {
                        float scaledFreq = juce::jmap(specificFreq, 60.0f, 18000.0f, 0.0f, 1.0f);  // Normalize frequency range to 0�1
                        float shaped = std::sin(scaledFreq * juce::MathConstants<float>::pi); // sin(pi * x), peak at 0.5 (midrange)
                        float dipFactor = 1.0f - shaped;  // Invert to have lowest value at midrange
                        float freqMapping = ((specificFreq / 2.0f) - (dipFactor * 6000.0f)) / qratio;
                        q = juce::jlimit(0.2f, 30.0f, freqMapping);
                    }
                    juce::dsp::ProcessorDuplicator<
                        juce::dsp::IIR::Filter<float>,
                        juce::dsp::IIR::Coefficients<float>> newFilter;
                    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeNotch(getSampleRate(), specificFreq, q);
                    *newFilter.state = *coeffs;
                    newFilter.prepare(spec);
                    vectorProcessorChain.push_back(std::move(newFilter));
                }
            }
        }
    }
}


void ColourCombV4AudioProcessor::toggleActiveFreq(int x) {
    if (activeFreqs[x] == 0 && numOfActiveFreqs < 5) {
        activeFreqs[x] = 1;
        numOfActiveFreqs++;
    }
    else if (activeFreqs[x] == 1) {
        activeFreqs[x] = 0;
        numOfActiveFreqs--;
    }
}