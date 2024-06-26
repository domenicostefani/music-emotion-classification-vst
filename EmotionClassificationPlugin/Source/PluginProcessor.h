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
#include "rtSafeLogger.h"

#ifdef JUCE_ARM
    #define ELK_OS_ARM 1
#endif

// OSC ports
#define RX_PORT 8042
#define TX_PORT 9042

#define SD_FRAME_SIZE 512
#define SD_HOP_SIZE 256
#define SD_SAMPLERATE 16000.0
#define SD_THRESHOLD -60.0f
#define SD_FILTER_LENGTH 187
#define SD_TRUE_TO_FALSE_TRANSITION_RATIO 0.0001
#define SD_ALPHA 0.5

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
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}

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
                 GAIN_ID = "gain", GAIN_NAME = "gain",
                 SILENCE_THRESH_ID = "silenceThresh", SILENCE_THRESH_NAME = "silenceThresh";
    bool oldRecState = false;
    AudioProcessorValueTreeState valueTreeState;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateRecState();
    void updateGain();
    void updateSilenceThresh();

#ifndef ELK_OS_ARM
    std::string extractorState = "Idle.";
#endif

    void extractAndClassify(std::string audioFilePath, bool _verbose = false);
    const size_t FRAMES_IN_3_SECONDS = 187;
    const size_t NUM_EMOTIONS = 4;

    void setSaveFolder(const File& saveFolder);
    juce::File& getSaveFolder();

    static InferenceEngine::InterpreterPtr tfliteClassifier;  // Tensorflow interpreter

    std::atomic<bool> recordingStopped{false}, labelsWritten{false};

    std::vector<std::tuple<float, float, std::string>> outputLabels;
    std::vector<size_t> outputLabelsInt;

    const std::string ELECTRIC_GUITAR_MODELNAME =  "electric_model.tflite";
    const std::string ACOUSTIC_GUITAR_MODELNAME =  "acoustic_model.tflite";
    const std::string PIANO_MODELNAME =  "piano_model.tflite";

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
    inline std::string getStrEmotion(const std::vector<bool>& predictedEmotions, std::map<size_t, std::string> emotions);

    // Feature extraction
    emosmi::MusicnnFeatureExtractor extractionPipeline;

    // Silence detection for RT gui
    emosmi::FilteredRTisSilent silenceDetector{SD_FRAME_SIZE, SD_HOP_SIZE, SD_THRESHOLD, SD_FILTER_LENGTH, SD_TRUE_TO_FALSE_TRANSITION_RATIO, SD_ALPHA};
    std::atomic<bool> isSilent{false};
    std::atomic<float> silenceThreshold;

    // Actual Offline silence detection
    // emosmi::FilteredIsSilent offlineSilenceDetector {16000.0, SD_FRAME_SIZE, SD_HOP_SIZE, SD_THRESHOLD, SD_FILTER_LENGTH, SD_TRUE_TO_FALSE_TRANSITION_RATIO,SD_ALPHA};
    emosmi::PerformanceStartStop performanceStartStop{16000.0, SD_FRAME_SIZE, SD_HOP_SIZE, SD_THRESHOLD, SD_FILTER_LENGTH, SD_TRUE_TO_FALSE_TRANSITION_RATIO, SD_ALPHA};

    bool loadModel(std::string modelPath, bool verbose = false);
    bool loadModelFromBuffer(const char *buffer, size_t bufferSize, bool verbose = false);
    bool loadModelFromBinaryData(const juce::String& modelName, bool verbose = false);

    std::string getModelPath();
#ifdef ELK_OS_ARM
    std::atomic<bool> enableRec{true};
#else
    std::atomic<bool> enableRec{false};
#endif

    inline void appendToGUIstate(std::string text) {
#ifndef ELK_OS_ARM
        this->extractorState += text;
#endif
    }

    inline void clearGUIstate() {
#ifndef ELK_OS_ARM
        this->extractorState = "";
#endif
    }

    inline void popBackGUIstate() {
#ifndef ELK_OS_ARM
        this->extractorState.pop_back();
#endif
    }

    inline std::string getGUIstate() {
#ifndef ELK_OS_ARM
        return this->extractorState;
#else
        return "";
#endif
    }

    std::string modelPath = "";

    void muteOutput(bool mute);
    std::atomic<float> outgain {1.0f};


    std::string compileDate,compileTime;

    rtutils::RealTimeLogger rtlogger {"emotion-classification-offline-plugin","/tmp/",1};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ECProcessor)
};
