/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EProcessor::EProcessor()
     : valueTreeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#ifndef JucePlugin_PreferredChannelConfigurations
     ,AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

juce::AudioProcessorValueTreeState::ParameterLayout EProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "gain",
                                                               juce::NormalisableRange<float>(0.0f,
                                                                                        5.0f,
                                                                                        0.0001,
                                                                                        0.4,
                                                                                        false),
                                                               1.f));
    return {parameters.begin(), parameters.end()};
}

EProcessor::~EProcessor()
{
}

//==============================================================================
const juce::String EProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EProcessor::getCurrentProgram()
{
    return 0;
}

void EProcessor::setCurrentProgram (int index)
{
}

const juce::String EProcessor::getProgramName (int index)
{
    return {};
}

void EProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    wavPlayer.prepareToPlay(sampleRate, samplesPerBlock);
}

void EProcessor::releaseResources()
{
    wavPlayer.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void EProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
 
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    wavPlayer.processBlock(buffer, midiMessages);

    playbackGain = ((juce::AudioParameterFloat *)valueTreeState.getParameter("gain"))->get();
    buffer.applyGainRamp(0, buffer.getNumSamples(), lastPlaybackGain, playbackGain);
    lastPlaybackGain = playbackGain;
}

//==============================================================================
bool EProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EProcessor::createEditor()
{
    return new PEditor (*this);
}

//==============================================================================
void EProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EProcessor();
}
