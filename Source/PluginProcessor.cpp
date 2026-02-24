/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using Notch = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                 juce::dsp::IIR::Coefficients<float>>;
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
    parameters.addParameterListener("focusValue", this);
    parameters.addParameterListener("cascade", this);
    parameters.addParameterListener("activeKeyMask", this);
    parameters.addParameterListener("lowCut", this);
    parameters.addParameterListener("highCut", this);

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
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();


    setFrequencyBounds(400.0f, 4000.0f);
    filterBank.clear();
    filterBank.reserve(noteFrequencies.size());
    createBank();
    
    auto coeffs1 = juce::dsp::IIR::Coefficients<float>::makeLowShelf(spec.sampleRate, 200, 1.f, 0.f);
    *lowerShelf.state = *coeffs1;
    auto coeffs2 = juce::dsp::IIR::Coefficients<float>::makeHighShelf(spec.sampleRate, 18000, 1.f, 0.f);
    *upperShelf.state = *coeffs2;
    lowerShelf.prepare(spec);
    upperShelf.prepare(spec);

   

    auto* p= parameters.getRawParameterValue("activeKeyMask");
    if (p){
        setActiveKeyMask(getMaskValue());
    }

    isPrepared.store(true);
    rebuild();


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

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    float lowCut = getLowCutValue();
    float highCut = getHighCutValue();
    float debug = 0.f;
    for (int note = 0; note < (int) activeFreqs.size(); ++note)
    {
        if (activeFreqs[note] != 1) continue;

        auto& band = filterBank[note];
        int counter = 0;
        
        for (auto& notch : band){
            debug = noteFrequencies[note][counter];
            if ((noteFrequencies[note][counter] > lowCut) && (highCut > noteFrequencies[note][counter])){
                notch.process(ctx);
            }
            counter++;
        }
    }
    lowerShelf.process(ctx);
    upperShelf.process(ctx);
    
    float wet = getMixValue();
    float dry = 1.0f - wet;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.applyGain(ch, 0, buffer.getNumSamples(), wet);
        buffer.addFrom(ch, 0, dryBuffer, ch, 0, dryBuffer.getNumSamples(), dry);
    }
    buffer.applyGain(juce::Decibels::decibelsToGain(getMakeupGainValue()));
}















//============State Information=================================================
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
    if (tree.isValid()) {
        parameters.replaceState(tree);
 
    }
    setStateFlag.store(true);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ColourCombV4AudioProcessor();}









//***************************** MASKS *****************************
void ColourCombV4AudioProcessor::setActiveKeyMask(int mask){
    numOfActiveFreqs = 0;
        for (int i = 0; i < activeFreqs.size(); ++i)
        {
            activeFreqs[i] = (mask & (1 << i)) ? 1 : 0;
            if (activeFreqs[i])
                ++numOfActiveFreqs;

        }


}













//*****************************
//**********GETTERS****************
float ColourCombV4AudioProcessor::getMixValue() const {return parameters.getRawParameterValue("mix")->load() / 100.0f;}
float ColourCombV4AudioProcessor::getMakeupGainValue() const {return parameters.getRawParameterValue("makeup")->load();}
float ColourCombV4AudioProcessor::getQValue() const {return parameters.getRawParameterValue("q")->load();}
int ColourCombV4AudioProcessor::getCurrentKey() const {return static_cast<int>(parameters.getRawParameterValue("key")->load());}
int ColourCombV4AudioProcessor::getCurrentFunction() const {return static_cast<int>(parameters.getRawParameterValue("qFunction")->load());}
float ColourCombV4AudioProcessor::getFocusValue() const {return parameters.getRawParameterValue("focusValue")->load();}
int ColourCombV4AudioProcessor::getCascadeValue() const {return static_cast<int>(parameters.getRawParameterValue("cascade")->load());}
int ColourCombV4AudioProcessor::getMaskValue() const {return parameters.getRawParameterValue("activeKeyMask")->load();}
float ColourCombV4AudioProcessor::getLowCutValue() const {return parameters.getRawParameterValue("lowCut")->load();}
float ColourCombV4AudioProcessor::getHighCutValue() const {return parameters.getRawParameterValue("highCut")->load();}



//*********EXTRA__SETTERS*****


void ColourCombV4AudioProcessor::setFrequencyBounds(float floorhz, float ceilinghz) {
    frequencyFloor = floorhz;
    frequencyCeiling = ceilinghz;
}



//**********AVPTS__PARAMETERS*********
void ColourCombV4AudioProcessor::parameterChanged(const juce::String& parameterID, float newValue) {
    if (!isPrepared){
        return;
    }
    
    if (parameterID == "q" || parameterID == "key" || parameterID == "qFunction"
        || parameterID == "focusValue") {
        handleAsyncUpdate();
    }

    else if (parameterID == "activeKeyMask"){
        
        int mask = (int) parameters.getRawParameterValue("activeKeyMask")->load();
        setActiveKeyMask(mask);
        handleAsyncUpdate();
    }
    
     
     
}

juce::AudioProcessorValueTreeState::ParameterLayout ColourCombV4AudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("q",1), "Q", juce::NormalisableRange<float>(1.0f, 100.0f, 2.0f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix",1), "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("makeup",1), "Makeup", juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("key",1), "Key", juce::StringArray({ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }), 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("qFunction",1), "Q Function", juce::StringArray({ "Sine", "Inv Sine" , "Constant"}), 0));
    //added a pushback for the layout
    params.push_back(std::make_unique <juce::AudioParameterFloat>(juce::ParameterID("focusValue",1), "Focus Value", juce::NormalisableRange<float>(0.1f, 1.0f, 0.05f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("cascade",1), "Cascade", juce::StringArray({ "x1", "x2"}), 0));
    //new
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("activeKeyMask",1),"Active Key Mask",0,(1<<12)-1,0));
    
    //Two float parameters can have full range, set to 0 and max hz for low and high respectively.  Logic in parameterchanged
    //will forceably correct them such that lowcut < highcut.  Processblock will check to see if a note enabled
    //lies in between, if not we simply skip the process call
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lowCut",1),"Low Cut",juce::NormalisableRange<float>(1.0f, 8000.0f, 10.0f),0.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("highCut",1),"High Cut",juce::NormalisableRange<float>(8000.0f, 22000.0f, 10.0f),22000.0f));
    
    
    return { params.begin(), params.end() };
    
}
































void ColourCombV4AudioProcessor::createBank(){
    
    for (int x=0; x < noteFrequencies.size(); x++){
        std::vector<Notch> band = createHarmonicGroup(x);
        filterBank.push_back(std::move(band));
    }
}

std::vector<Notch> ColourCombV4AudioProcessor::createHarmonicGroup(int targetNote){
    std::vector<Notch> notes;
    for (int y=0; y < noteFrequencies[targetNote].size(); y++){
        Notch newFilter;
        float targetFreq = noteFrequencies[targetNote][y];
        float q = determineQ(targetFreq,getQValue(),0);
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeNotch(getSampleRate(), targetFreq, q);
        *newFilter.state = *coeffs;
        newFilter.prepare(spec);
        notes.push_back(std::move(newFilter));
    }
    return notes;
}

void ColourCombV4AudioProcessor::updateHarmonicGroup(int targetNote){
    for (int y=0; y < filterBank[targetNote].size(); y++){
        Notch &filter = filterBank[targetNote][y];
        float targetFreq = noteFrequencies[targetNote][y];
        float q = determineQ(targetFreq,getQValue(),getCurrentFunction());
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeNotch(getSampleRate(), targetFreq, q);
        *filter.state = *coeffs;
    }
}




float ColourCombV4AudioProcessor::determineQ(float freq, float qRatio, int funcType){
    //sine
    float q = 1.0f;
    if (funcType == 0){
        float freqMapping = (900 * std::sin((juce::MathConstants<float>::pi * freq) / spec.sampleRate)) / (qRatio/2);
        q = juce::jlimit(0.4f, 5.0f, freqMapping);}
    //1.0f, 50.f
    //inv sine
    else if (funcType == 1){
        float freqMapping = (600 * (1.0f-std::sin((juce::MathConstants<float>::pi * freq) / spec.sampleRate))) / qRatio;
        q = juce::jlimit(1.0f, 50.0f, freqMapping);}
    //constant
    else if (funcType == 2){
        float freqMapping = 400/ qRatio;
        q = juce::jlimit(1.0f, 50.0f, freqMapping);}
    return q;
}



void ColourCombV4AudioProcessor::rebuild(){

    
    
    for (int x=0; x < activeFreqs.size(); x++){
        if (activeFreqs[x]==1){
            updateHarmonicGroup(x);
        }
    }
    auto coeffs1 = juce::dsp::IIR::Coefficients<float>::makeLowShelf(spec.sampleRate, 200, 1.f, (1.1f-getFocusValue()));
    *lowerShelf.state = *coeffs1;
    auto coeffs2 = juce::dsp::IIR::Coefficients<float>::makeHighShelf(spec.sampleRate, 18000, 1.f,(1.1f-getFocusValue()));
    *upperShelf.state = *coeffs2;
}

void ColourCombV4AudioProcessor::handleAsyncUpdate(){
    rebuild();
}


