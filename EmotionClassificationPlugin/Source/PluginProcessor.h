/**
 * @file PluginProcessor.h
 * @author Domenico Stefani (domenico [dot] stefani [at] unitn [dot] it)
 * @brief This is the main processor of the plugin. It contains all the code needed to extract features from the audio, classify intended emotion and to communicate with the controller through OSC messages.
 * @version 0.1
 * @date 2023-05-29
 *  * 
 */
#pragma once

#include <JuceHeader.h>

#include "AudioRecorder.h"
#include "OSCreceiver.h"
#include "OSCsender.h"
#include "Poller.h"
#include "extractionpipeline.h"
#include "tflitewrapper.h"

#ifdef JUCE_ARM
    #define ELK_OS_ARM 1
#endif

// OSC ports
#define RX_PORT 8042
#define TX_PORT 9042

class ECProcessor : public juce::AudioProcessor {
public:
    //==============================================================================
    ECProcessor();
    ~ECProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    std::unique_ptr<FileLogger> logger;

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

    const unsigned int NUM_CHANNELS = 1;
    File lastRecording, lastRecording2;
    std::string audioFilename = "";
    double sampleRate = 0;
    void startRecording(unsigned int numChannels);
    void stopRecording();

    const String RECSTATE_ID = "recstate", RECSTATE_NAME = "recstate",
                 GAIN_ID = "gain", GAIN_NAME = "gain";
    bool oldRecState = false;
    AudioProcessorValueTreeState valueTreeState;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateRecState();
    void updateGain();

    std::string extractorState = "Idle.";

    void extractAndClassify(std::string audioFilePath);
    const size_t FRAMES_IN_3_SECONDS = 187;
    const size_t NUM_EMOTIONS = 4;

    void setSaveFolder(const File& saveFolder);
    juce::File& getSaveFolder();

    static ClassifierPtr tfliteClassifier;  // Tensorflow interpreter

    std::atomic<bool> recordingStopped{false}, labelsWritten{false};

    std::vector<std::string> outputLabels;
    std::vector<size_t> outputLabelsInt;

private:
#if (JUCE_ANDROID || JUCE_IOS)
    juce::File saveDir = File::getSpecialLocation(File::tempDirectory);
#elif (ELK_OS_ARM)
    juce::File saveDir = File("/udata/emoAwSMIs-recordings/unnamed/");
#else
    juce::File saveDir = File::getSpecialLocation(File::userHomeDirectory);
#endif

    // Metering
    LinearSmoothedValue<float> rmsValue, peakValue;

public:
    float getRMSValue() const;
    float getPeakValue() const;
    std::unique_ptr<Poller> oscMeterPoller;
    void oscSendPollingRoutine();

    // Input gain
    float inputGain = 1.0f;
    float lastInputGain = 1.0f;
    void setInputGain(float newGain);

    // OSC communication on side port (8082 incoming, 9042 outgoing)
    cOSC::HandshakedSender oscSender;
    cOSC::Receiver oscReceiver;
    void oscMessageReceived(const juce::OSCMessage& message);

    std::vector<bool> topPredictedEmotions;

    // Feature extraction
    emosmi::MusicnnFeatureExtractor extractionPipeline;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ECProcessor)
};
