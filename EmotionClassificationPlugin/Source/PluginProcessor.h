/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioRecorder.h"

//==============================================================================
/**
*/
class EmotionClassificationPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EmotionClassificationPluginAudioProcessor();
    ~EmotionClassificationPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::unique_ptr<FileLogger> logger;

private:

   #ifdef RECORDER_DEBUG_LOG
    AudioRecorder recorder{logger};
    std::string loggerPath;
    const std::string LOG_FILENAME = "audioRecorder";
    const std::string LOG_EXTENSION = "log";
    void setLogPath(std::string mpath);
    void logText(std::string text);
    int logCounter = 0;
   #else
    AudioRecorder recorder;
   #endif

   const unsigned int NUM_CHANNELS = 2;
    File lastRecording;
    double sampleRate = 0;
    void startRecording(unsigned int numChannels);
    void stopRecording();

    const String RECSTATE_ID = "recstate";
    const String RECSTATE_NAME = "recstate";
    bool oldRecState = false;
    AudioProcessorValueTreeState valueTreeState;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateRecState();


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmotionClassificationPluginAudioProcessor)
};
