#include "PluginProcessor.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "PluginEditor.h"

// #define DUMMY_INFERENCE

// #define VERBOSE_PRINT

#define VERBOSE_EXTRACT_CLASSIFY true

// Load the model and init the interpreter
// Load either from a file in the filesystem or from JUCE binary data
// The second is suggested for cross-platform compatibility, as the first depends on the model being on a path that is local to the target machine
#ifdef ELK_OS_ARM
    #define LOAD_MODEL_FROM_FILE 0

    #ifdef ELECTRIC_GUITAR_MODEL
        #define BIN_DATA_MODELNAME ELECTRIC_GUITAR_MODELNAME
    #endif
    #ifdef ACOUSTIC_GUITAR_MODEL
        #define BIN_DATA_MODELNAME ACOUSTIC_GUITAR_MODELNAME
    #endif
    #ifdef PIANO_MODEL
        #define BIN_DATA_MODELNAME PIANO_MODELNAME
    #endif
#else
    #define LOAD_MODEL_FROM_FILE 1
#endif

#ifdef VERBOSE_PRINT
    #include "verbose_utils.h"
#endif

// #define DO_REMOVE_OLD_RECORDINGS
#define GENERATE_AUDACITY_LABELS
#define VERBOSE_CLASSIFICATION false

#define STATE_IDLE 0
#define STATE_RECORDING 1
#define STATE_CLASSIFYING 2

// If defined, STARTSTOP_KEEP_LAST_CHUNK will keep the last chunk of audio in the non-silent section of the recording even if up to 50% of it is in the silent section (math rounding).
#define STARTSTOP_KEEP_LAST_CHUNK

std::map<size_t, std::string> emotions = {
    {0, "Aggressive"},
    {1, "Relaxed"},
    {2, "Happy"},
    {3, "Sad"}};

InferenceEngine::InterpreterPtr ECProcessor::tfliteClassifier = nullptr;

//==============================================================================
ECProcessor::ECProcessor()
    : valueTreeState(*this, nullptr, "PARAMETERS", createParameterLayout()),
      oscReceiver(RX_PORT, [&](const juce::OSCMessage &m) { this->oscMessageReceived(m); })
#ifndef JucePlugin_PreferredChannelConfigurations
      ,
      AudioProcessor(BusesProperties()
    #if !JucePlugin_IsMidiEffect
        #if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::mono(), true)
        #endif
                         .withOutput("Output", juce::AudioChannelSet::mono(), true)
    #endif
      )
#endif
{
    rtlogger.logInfo("ECProcessor constructor");
    rtlogger.logInfo(((std::string("WrapperType: ") + getWrapperTypeDescription(wrapperType)).c_str()));
    suspendProcessing(true);

    this->compileDate = __DATE__;
    this->compileTime = __TIME__;

    rtlogger.logInfo("Storing silence threshold value in atomic");
    silenceThreshold.store(SD_THRESHOLD);

    appendToGUIstate("\nECProcessor constructor");
    // Try to create the output folder if it doesn't exist
    if (!saveDir.exists()) {
        rtlogger.logInfo("Creating output save directory at ", saveDir.getFullPathName().toStdString().c_str());
        appendToGUIstate("\nCreating output save directory at " + saveDir.getFullPathName().toStdString());
        if (saveDir.createDirectory().failed()) {
            std::cout << "Error: could not create output save directory at " << saveDir.getFullPathName().toStdString() << std::endl;
            appendToGUIstate("\nError: could not create output save directory at " + saveDir.getFullPathName().toStdString());
        }
        // throw std::runtime_error("Error: could not create output save directory at " + saveDir.getFullPathName().toStdString());
    } else if (saveDir.existsAsFile()) {
        rtlogger.logInfo("Error: output save directory exists as a file. Please delete the file at ", saveDir.getFullPathName().toStdString().c_str());
        std::cout << "Error: output save directory exists as a file. Please delete the file at " << saveDir.getFullPathName().toStdString() << std::endl;
        appendToGUIstate("\nError: output save directory exists as a file. Please delete the file at " + saveDir.getFullPathName().toStdString());
    } else {
        rtlogger.logInfo("Output save directory already exists at ", saveDir.getFullPathName().toStdString().c_str());
    }

    // throw std::runtime_error("Error: output save directory exists as a file. Please delete the file at " + saveDir.getFullPathName().toStdString());

#ifndef ELK_OS_ARM
    extractorState.reserve(1048576);
#endif

#ifndef DUMMY_INFERENCE
    #ifdef ELK_OS_ARM
    rtlogger.logInfo("Loading model from binary data (from constructor since we are running on ELK OS)");
        #if LOAD_MODEL_FROM_FILE
            #ifdef ELECTRIC_GUITAR_MODEL
    std::string MODEL_PATH = "/udata/emotionModel_electric.tflite";
            #endif
            #ifdef ACOUSTIC_GUITAR_MODEL
    std::string MODEL_PATH = "/udata/emotionModel_acoustic.tflite";
            #endif
            #ifdef PIANO_MODEL
    std::string MODEL_PATH = "/udata/emotionModel_piano.tflite";
            #endif

    std::cout << std::setfill('-') << std::setw(40) << "" << std::endl;
    std::cout << std::left << std::setfill('.') << std::setw(30) << "ElkPlugin-Loading model from file: " << std::right << std::setfill('.') << std::setw(10) << MODEL_PATH << std::endl;
    std::cout << std::setfill('-') << std::setw(40) << "" << std::endl;

    rtlogger.logInfo("Loading model from file: " + MODEL_PATH);

    this->loadModel(MODEL_PATH, VERBOSE_CLASSIFICATION);
        #else
    rtlogger.logInfo("Loading model from binary data, name: ", BIN_DATA_MODELNAME.c_str());
    juce::String modelBinaryDataFilename = BIN_DATA_MODELNAME;
    this->loadModelFromBinaryData(modelBinaryDataFilename, VERBOSE_CLASSIFICATION);
        #endif
    #endif

#endif

    // Set up the OSC receiver to accept specific messages
    rtlogger.logInfo("Setting up OSC receiver");
    std::vector<juce::String> oscMessages = {"/handshake", "/disconnect"};
    for (auto oscMessage : oscMessages)
        oscReceiver.addOSCListener(oscMessage);
    rtlogger.logInfo("OSC receiver set up");

    topPredictedEmotions.resize(this->NUM_EMOTIONS);

    rtlogger.logInfo("ECProcessor constructor finished");
    suspendProcessing(false);
    rtlogger.logInfo("audio processing restored");
}

bool ECProcessor::loadModel(std::string modelPath, bool verbose) {
    rtlogger.logInfo("Loading model from file: ", modelPath.c_str());
    // Check that file exists
    std::ifstream f(modelPath);
    if (!f.good())
        return false;
    if (modelPath.substr(modelPath.find_last_of(".") + 1) != "tflite")
        return false;

    try {
        if (ECProcessor::tfliteClassifier != nullptr) {
            std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
            std::cout << std::left << std::setfill('.') << std::setw(80) << "Deleting Intepreter" << std::endl;
            InferenceEngine::deleteInterpreter(ECProcessor::tfliteClassifier);
            ECProcessor::tfliteClassifier = nullptr;
        }
        std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
        std::cout << std::left << std::setfill('.') << std::setw(50) << "Loading model from file:" << std::right << std::setfill('.') << std::setw(30) << modelPath << std::endl;
        std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
        ECProcessor::tfliteClassifier = InferenceEngine::createInterpreter(modelPath, VERBOSE_CLASSIFICATION);  // true = verbose cout
        if (verbose) std::cout << "Created classifier and loaded model: '" << modelPath << "'" << std::endl;
        rtlogger.logInfo("Created classifier and loaded model: ", modelPath.c_str());
    } catch (std::exception &e) {
        std::cout << "Error: could not load model: '" << modelPath << "'" << std::endl;
        return false;
    }

    Value mpProperty = valueTreeState.state.getPropertyAsValue("SV_MODEL_PATH", nullptr, true);
    mpProperty.setValue(juce::String(modelPath));
    // class field modelPath is set to confirm which model has been loaded
    this->modelPath = modelPath;

    this->enableRec.store(true);
    return true;
}

bool ECProcessor::loadModelFromBuffer(const char *buffer, size_t bufferSize, bool verbose) {
    try {
        if (ECProcessor::tfliteClassifier != nullptr) {
            std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
            std::cout << std::left << std::setfill('.') << std::setw(80) << "Deleting Intepreter" << std::right << std::endl;
            InferenceEngine::deleteInterpreter(ECProcessor::tfliteClassifier);
            ECProcessor::tfliteClassifier = nullptr;
        }
        ECProcessor::tfliteClassifier = InferenceEngine::createInterpreterFromBuffer(buffer, bufferSize, VERBOSE_CLASSIFICATION);
        if (verbose) std::cout << "Created classifier and loaded model from buffer" << std::endl;
    } catch (std::exception &e) {
        std::cout << "Error: could not load model! '" << e.what() << "'" << std::endl;
        return false;
    }

    this->enableRec.store(true);
    return true;
}

bool ECProcessor::loadModelFromBinaryData(const juce::String &modelName, bool verbose) {
    rtlogger.logInfo("Loading model from binary data, name: ", modelName.toStdString().c_str());
    size_t binresource_idx = 0;
    for (int i = 0; i < BinaryData::namedResourceListSize; i++)
        if (BinaryData::originalFilenames[i] == modelName.toStdString().c_str())
            binresource_idx = i;

    const char *binNameUTF8 = BinaryData::namedResourceList[binresource_idx];

    // Get model content
    int size;
    auto model_content = BinaryData::getNamedResource(binNameUTF8, size);
    if (ECProcessor::tfliteClassifier != nullptr) {
        std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
        std::cout << std::left << std::setfill('.') << std::setw(80) << "Deleting Intepreter" << std::right << std::endl;
        InferenceEngine::deleteInterpreter(ECProcessor::tfliteClassifier);
        ECProcessor::tfliteClassifier = nullptr;
    }
    std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;
    std::cout << std::left << std::setfill('.') << std::setw(50) << "Loading model from BinaryData:" << std::right << std::setfill('.') << std::setw(30) << modelName.toStdString() << std::endl;
    std::cout << std::setfill('-') << std::setw(80) << "" << std::endl;

    if (this->loadModelFromBuffer(model_content, size, true)) {
        Value mpProperty = valueTreeState.state.getPropertyAsValue("SV_MODEL_PATH", nullptr, true);
        mpProperty.setValue(juce::String("BINARY_" + modelName.toStdString()));
        // class field modelPath is set to confirm which model has been loaded
        this->modelPath = "BINARY_" + modelName.toStdString();
        std::cout << "modelPath: " << this->modelPath << std::endl;
        rtlogger.logInfo("Loaded model from binary data, name: ", modelName.toStdString().c_str());
        return true;
    }
    return false;
}

std::string ECProcessor::getModelPath() {
    return this->modelPath;
}

ECProcessor::~ECProcessor() {
}

void ECProcessor::oscMessageReceived(const juce::OSCMessage &message) {
    std::cout << "Received message " << message.getAddressPattern().toString().toStdString() << std::endl;
    if (message.getAddressPattern() == juce::OSCAddressPattern("/handshake")) {
        std::cout << "-> Handshake!" << std::endl;
        if (message.size() == 1 && message[0].isString()) {
            std::cout << "-> ip: " << message[0].getString() << std::endl;
            this->oscSender.enableAndReplyToHandshake(message[0].getString(), TX_PORT);
            oscMeterPoller = std::make_unique<Poller>(24, [&]() { this->oscSendPollingRoutine(); });
            this->oscSender.sendMessage("/state", (int)STATE_IDLE);
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/disconnect")) {
        std::cout << "-> disconnect" << std::endl;
        oscMeterPoller.reset();
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/gain")) {
        if (message.size() == 1 && message[0].isFloat32()) {
            std::cout << "-> gain: " << message[0].getFloat32() << std::endl;
        }
    }
}

void ECProcessor::oscSendPollingRoutine() {
    float peak = getPeakValue();
    oscSender.sendMessage("/meter", peak);

    bool isSilent = this->isSilent.load();
    oscSender.sendMessage("/silence", isSilent);
}

void ECProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    this->sampleRate = sampleRate;
    recorder.prepareToPlay(sampleRate);
    // Metering
    rmsValue.reset(sampleRate,
                   0.2);  // Decay in seconds
    rmsValue.setCurrentAndTargetValue(-100.0);

    peakValue.reset(sampleRate, 0.2);
    peakValue.setCurrentAndTargetValue(-100.0);
}

void ECProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages) {
    // If more than one channel, sum all channels to mono and write to first channel
    if (buffer.getNumChannels() > 1)
        for (int i = 1; i < buffer.getNumChannels(); i++)
            buffer.addFrom(0, 0, buffer, i, 0, buffer.getNumSamples(), 1.0);

    // Fist apply amp gain to the input
    updateGain();
    updateSilenceThresh();
    buffer.applyGainRamp(0, buffer.getNumSamples(), lastInputGain, inputGain);
    lastInputGain = inputGain;

    rmsValue.skip(buffer.getNumSamples());                                                          // Advance time for smoothing
    const auto value = Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));  // get RMS value
    if (value < rmsValue.getCurrentValue()) {                                                       // Smooth only if level is decreasing
        rmsValue.setTargetValue(value);
    } else {
        rmsValue.setCurrentAndTargetValue(value);  // Don't smooth if level is increasing (keep meter responsive)
    }

    peakValue.skip(buffer.getNumSamples());
    const auto peak = Decibels::gainToDecibels(buffer.getMagnitude(0, 0, buffer.getNumSamples()));
    if (peak < peakValue.getCurrentValue()) {
        peakValue.setTargetValue(peak);
    } else {
        peakValue.setCurrentAndTargetValue(peak);
    }

    silenceDetector.processBlock(buffer);
    isSilent.store(silenceDetector.computeIsSilent());

    updateRecState();
    recorder.writeBlock(buffer.getArrayOfReadPointers(), buffer.getNumSamples());

    float localoutgain = outgain.load();
    for (size_t chan = 0; chan < buffer.getNumChannels(); chan++) {
        for (size_t i = 0; i < buffer.getNumSamples(); i++) {
            buffer.setSample(chan, i, buffer.getSample(chan, i) * localoutgain);
        }
    }
}

std::string getStrTime() {
    auto curtime = Time::getCurrentTime();

    std::string timestr = std::to_string(curtime.getYear());
    const size_t FIELD_LENGTH = 2;
    std::string toWrite = "";
    toWrite = std::to_string(curtime.getMonth() + 1);  // +1 because months are 0-indexed
    timestr += std::string(FIELD_LENGTH - std::min(FIELD_LENGTH, toWrite.length()), '0') + toWrite;
    toWrite = std::to_string(curtime.getDayOfMonth());
    timestr += std::string(FIELD_LENGTH - std::min(FIELD_LENGTH, toWrite.length()), '0') + toWrite;
    timestr += "-";
    toWrite = std::to_string(curtime.getHours());
    timestr += std::string(FIELD_LENGTH - std::min(FIELD_LENGTH, toWrite.length()), '0') + toWrite + ":";
    toWrite = std::to_string(curtime.getMinutes());
    timestr += std::string(FIELD_LENGTH - std::min(FIELD_LENGTH, toWrite.length()), '0') + toWrite + ":";
    toWrite = std::to_string(curtime.getSeconds());
    timestr += std::string(FIELD_LENGTH - std::min(FIELD_LENGTH, toWrite.length()), '0') + toWrite;

    return timestr;
}

void ECProcessor::startRecording(unsigned int numChannels) {
#ifdef RECORDER_DEBUG_LOG
    logText("Recording started");
#endif
    if (!RuntimePermissions::isGranted(RuntimePermissions::writeExternalStorage)) {
        RuntimePermissions::request(RuntimePermissions::writeExternalStorage,
                                    [this, numChannels](bool granted) mutable {
                                        if (granted) {
                                            this->startRecording(numChannels);
                                        }
                                    });
        return;
    }

    // Get current time
    std::string timestr = getStrTime();

#ifdef ELECTRIC_GUITAR_MODEL
    timestr = "electric-" + timestr;
#endif
#ifdef ACOUSTIC_GUITAR_MODEL
    timestr = "acoustic-" + timestr;
#endif
#ifdef PIANO_MODEL
    timestr = "piano-" + timestr;
#endif

    this->lastRecording = saveDir.getNonexistentChildFile("EmoAwSMIs-recording-" + timestr, ".wav");
    this->lastRecording2 = this->lastRecording;
    clearGUIstate();
    appendToGUIstate("Recording to disk: \"");
    this->audioFilename = lastRecording.getFullPathName().toStdString();
    appendToGUIstate(audioFilename + "\"...");

#ifdef RECORDER_DEBUG_LOG
    logText("Recording " + lastRecording.getFullPathName().toStdString());
#endif

    // std::cout << "Recording " << lastRecording.getFullPathName().toStdString() << std::endl;

    recorder.startRecording(lastRecording, numChannels);
    this->oscSender.sendMessage("/state", (int)STATE_RECORDING);
}

void ECProcessor::stopRecording() {
#ifdef RECORDER_DEBUG_LOG
    logText("Recording stopped");
#endif
    recorder.stop();

    appendToGUIstate("\nStopped recording.");

#if (JUCE_ANDROID || JUCE_IOS)
    SafePointer<AudioRecordingDemo> safeThis(this);
    File fileToShare = lastRecording;

    ContentSharer::getInstance()->shareFiles(Array<URL>({URL(fileToShare)}),
                                             [safeThis, fileToShare](bool success, const String &error) {
    #ifdef RECORDER_DEBUG_LOG
                                                 logText("Android/iOS ContentSharer callback called");
    #endif
                                                 if (fileToShare.existsAsFile()) {
    #ifdef RECORDER_DEBUG_LOG
                                                     logText("File existed, deleting...");
    #endif
                                                     fileToShare.deleteFile();
                                                 }

    #ifdef RECORDER_DEBUG_LOG
                                                 logText("SUCCESS: " + (success ? "true" : "false"));
                                                 logText("ERROR: " + error.toStdString());
    #endif
                                                 if (!success && error.isNotEmpty()) {
                                                     NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                                                           "Sharing Error",
                                                                                           error);
                                                 }
                                             });
#endif

    lastRecording = File();
}

void ECProcessor::muteOutput(bool mute) {
    if (mute) {
        outgain.store(0.0f);
    } else {
        outgain.store(1.0f);
    }
}

void classifyChunk(const std::vector<std::vector<float>> &input, std::vector<float> &output) {
    // Flatten the input
    size_t n_rows = input.size();
    size_t n_cols = input[0].size();
    std::vector<float> flat_input(n_rows * n_cols);
    for (size_t i = 0; i < n_rows; ++i) {
        if (input[i].size() != n_cols)
            throw std::runtime_error("Input is not a matrix (Row " + std::to_string(i) + " has " + std::to_string(input[i].size()) + " columns instead of " + std::to_string(n_cols) + ")");
        for (size_t j = 0; j < n_cols; ++j)
            flat_input[i * n_cols + j] = input[i][j];
    }

#ifdef DUMMY_INFERENCE
    output[0] = 0.0f;
    output[1] = 1.0f;
    output[2] = 0.9f;
    output[3] = 0.0f;
#else
    InferenceEngine::invokeFlat2D(ECProcessor::tfliteClassifier,
                                  flat_input.data(),
                                  n_rows,
                                  n_cols,
                                  output.data(),
                                  output.size(), VERBOSE_CLASSIFICATION);
#endif
}

size_t softVoting(const std::vector<std::vector<float>> &input, const size_t NUM_EMOTIONS) {
    std::vector<float> avg(NUM_EMOTIONS, 0.0f);
    // Sum the SoftMax probabilities for each emotion
    for (auto &v : input)
        for (size_t i = 0; i < NUM_EMOTIONS; ++i)
            avg[i] += v[i];
    // Average
    for (size_t i = 1; i < avg.size(); ++i)
        avg[i] = avg[i] / input.size();

    // Argmax
    size_t best = 0;
    for (size_t i = 1; i < avg.size(); ++i)
        if (avg[i] > avg[best])
            best = i;

    return best;
}

void ambivalentRes(std::vector<float> softmax, std::vector<bool> &resultBest, float thresholdDistance = 1.0f / 7.0f) {
    int largestIndex = 0, secondLargestIndex = -1;
    for (int i = 1; i < softmax.size(); ++i) {
        if (softmax[i] > softmax[largestIndex]) {
            secondLargestIndex = largestIndex;
            largestIndex = i;
        } else if (secondLargestIndex == -1 || softmax[i] > softmax[secondLargestIndex]) {
            secondLargestIndex = i;
        }
    }

    for (size_t i = 0; i < resultBest.size(); ++i)
        resultBest[i] = (softmax[largestIndex] - softmax[i] < thresholdDistance);
}

inline std::string ECProcessor::getStrEmotion(const std::vector<bool> &predictedEmotions, std::map<size_t, std::string> emotions) {
    std::string chunkEmotions = "",
                intemo = "";
    for (size_t j = 0; j < predictedEmotions.size(); ++j) {
        if (predictedEmotions[j]) {
            intemo += (std::to_string(j) + "/");
            chunkEmotions += (emotions[j] + "/");
        }
    }
    if (intemo.size() > 0)
        intemo.pop_back();
    this->appendToGUIstate(intemo);
    std::cout << "Appending \"" << intemo << "\" to GUI state" << std::endl;
    chunkEmotions.pop_back();
    return chunkEmotions;
}

/**
 * @brief Softvoting stategy where the result can be either a single emotion or multiple emotions (ambivalent)
 *
 * @param input Vector of SoftMax probabilities
 * @param NUM_EMOTIONS size of the SoftMax vectors
 * @param thresholdDistance required distance between the highest and second highest probability to consider the result NON-ambivalent
 * @param resultBest Boolean vector of size NUM_EMOTIONS, where the resulting top emotions are stored
 * @return size_t highest probability emotion if the probability is further than distance from the second best, otherwise -1
 */
size_t ambivalentSoftVoting(const std::vector<std::vector<float>> &input, const size_t NUM_EMOTIONS, std::vector<bool> &resultBest, float thresholdDistance = 1.f / 7.f) {
    std::vector<float> avg(NUM_EMOTIONS, 0.0f);
    // Sum the SoftMax probabilities for each emotion
    const bool VERBOSE = false;
    if (VERBOSE) std::cout << "SoftMax inputs:\n";
    for (auto &v : input) {
        if (VERBOSE) std::cout << "[ ";
        for (size_t i = 0; i < NUM_EMOTIONS; ++i) {
            if (VERBOSE) std::cout << v[i] << " ";
            avg[i] += v[i];
        }
        if (VERBOSE) std::cout << "]\n";
    }
    if (VERBOSE) std::cout << "---\n";
    // Average
    for (size_t i = 0; i < avg.size(); ++i)
        avg[i] = avg[i] / input.size();

    if (VERBOSE) {
        std::cout << "SoftMax averages: ";
        for (auto &v : avg)
            std::cout << v << " ";
        std::cout << std::endl;
    }

    // Argmax
    int largestIndex = 0, secondLargestIndex = -1;

    for (int i = 1; i < avg.size(); ++i) {
        if (avg[i] > avg[largestIndex]) {
            secondLargestIndex = largestIndex;
            largestIndex = i;
        } else if (secondLargestIndex == -1 || avg[i] > avg[secondLargestIndex]) {
            secondLargestIndex = i;
        }
    }

    if (VERBOSE) printf("Best: [%d] %.3f\n", largestIndex, avg[largestIndex]);
    if (VERBOSE) printf("SecondBest: [%d] %.3f\n", secondLargestIndex, avg[secondLargestIndex]);

    // Check if the distance between the best and the second best is smaller than the thresholdDistance
    if (avg[largestIndex] - avg[secondLargestIndex] < thresholdDistance) {
        if (VERBOSE) printf("Ambivalent\n");
        // If the distance is smaller than the thresholdDistance, then we are ambivalent
        // Then we return -1 and store the emotions that fall into the thresholdDistance distance in resultBest
        for (size_t i = 0; i < resultBest.size(); ++i)
            resultBest[i] = (avg[largestIndex] - avg[i] < thresholdDistance);

        return -1;
    }
    if (VERBOSE) printf("Not ambivalent\n");
    // If the distance is larger than the thresholdDistance, then we are not ambivalent
    // Return the best
    for (size_t i = 0; i < resultBest.size(); ++i)
        resultBest[i] = (largestIndex == i);
    return largestIndex;
}

void ECProcessor::extractAndClassify(std::string audioFilePath, bool _verbose) {
    this->oscSender.sendMessage("/state", (int)STATE_CLASSIFYING);
#ifdef VERBOSE_PRINT
    std::cout << "Extracting and classifying " << audioFilePath << std::endl;
    std::cout << "File \"" << audioFilePath << "\"" << audioFilePath << std::endl;
#endif
    std::vector<std::vector<essentia::Real>> featvec;

    // Check that the file exists
    File tmpfile = File(audioFilePath);
    if (!tmpfile.exists()) {
#ifdef VERBOSE_PRINT
        std::cout << "File " << audioFilePath << " DOES NOT EXIST\n"
                  << std::flush;
#endif
// throw std::logic_error("File " + audioFilePath + " does not exist!");
#ifdef ELK_OS_ARM
        std::cerr << "File " << audioFilePath << " does not exist! Check that the recording savedir exists\n"
                  << std::flush;
#else
        appendToGUIstate("\nRec File " + audioFilePath + " does not exist! Check that the recording savedir exists\n");
#endif

    }
#ifdef VERBOSE_PRINT
    else {
        std::cout << "File " << audioFilePath << " exists\n"
                  << std::flush;
    }
#endif

    // Extract features

    extractionPipeline.extractFromFile(audioFilePath, featvec);
#ifdef VERBOSE_PRINT
    DBG("Extracted features:\n");
#endif

    appendToGUIstate("\nExtracted " + std::to_string(featvec[0].size()) + " features from " + std::to_string(featvec.size()) + " frames");

#ifdef VERBOSE_PRINT
    print2DVectorHead(featvec);
#endif

    std::pair<size_t, size_t> startstop = performanceStartStop.computeFromFile(audioFilePath, silenceThreshold);
    size_t startIdx = startstop.first;
    size_t stopIdx = startstop.second;

    // Now we split the feature matrix in 3 second chunks (187 frames), call the classifier and then call voting subroutine
    size_t numFrames = stopIdx - startIdx;
#ifdef STARTSTOP_KEEP_LAST_CHUNK
    size_t numChunks = roundf((float)numFrames / FRAMES_IN_3_SECONDS);      // Take ceiling to include the last chunk even if it's 50% silent (math round)
    numChunks = std::min(numChunks, featvec.size() / FRAMES_IN_3_SECONDS);  // If the last chunk is too small, then don't include it
    //
#else
    size_t numChunks = numFrames / FRAMES_IN_3_SECONDS;
#endif

    if (numChunks == 0) {
        appendToGUIstate("\nNot enough frames to classify (Please record for more than 3 NON-SILENT seconds)");
        this->oscSender.sendMessage("/emotion", (int)-1);
        this->oscSender.sendMessage("/state", (int)STATE_IDLE);
        return;
    }

    appendToGUIstate("\nSplitting into " + std::to_string(numChunks) + " chunks of 3 seconds each");

    std::vector<std::vector<float>> res;
    res.resize(numChunks);
    appendToGUIstate("\nPer Chunk winners: [ ");

#ifdef GENERATE_AUDACITY_LABELS
    std::vector<std::string> allPerChunkEmotions;
    std::vector<std::string> softmaxOutsPrintable;
#endif
    outputLabels.clear();
    outputLabelsInt.clear();

    for (size_t i = 0; i < numChunks; ++i) {
        res.at(i).resize(NUM_EMOTIONS);
        if (_verbose) std::cout << "Classifying chunk " << i << " which goes from " << (i)*FRAMES_IN_3_SECONDS + startIdx << " to " << (i + 1) * FRAMES_IN_3_SECONDS + startIdx << std::endl;
        classifyChunk(std::vector<std::vector<float>>(featvec.begin() + (i)*FRAMES_IN_3_SECONDS + startIdx, featvec.begin() + (i + 1) * FRAMES_IN_3_SECONDS + startIdx), res.at(i));
        // Check if the result is ambivalent or only one class prevails (if the result is ambivalent, the resultBest vector will contain true values corresponding the emotions that fall into the thresholdDistance distance)
        ambivalentRes(res.at(i), this->topPredictedEmotions);
        // Add to log all the emotions that fall into the thresholdDistance distance

#ifdef GENERATE_AUDACITY_LABELS
        std::string softmaxOut = "[ ";
        for (size_t j = 0; j < NUM_EMOTIONS; ++j)
            softmaxOut += std::to_string(res.at(i).at(j)) + " ";
        softmaxOut += "]";
        softmaxOutsPrintable.push_back(softmaxOut);
#endif
        auto todoremove = emotions;
        std::string chunkEmotions = getStrEmotion(this->topPredictedEmotions, emotions);

        float startTime = (startIdx * SD_HOP_SIZE / (float)SD_SAMPLERATE) + (i * 3.0f);
        float endTime = startTime + 3.0f;
        this->outputLabels.push_back(std::make_tuple(startTime, endTime, chunkEmotions));
        if (std::count(topPredictedEmotions.begin(), topPredictedEmotions.end(), true) > 1)
            this->outputLabelsInt.push_back(NUM_EMOTIONS);
        else {
            size_t topEmotion = 0;
            for (size_t j = 0; j < topPredictedEmotions.size(); ++j)
                if (topPredictedEmotions[j])
                    topEmotion = j;
            this->outputLabelsInt.push_back(topEmotion);
        }

#ifdef GENERATE_AUDACITY_LABELS
        allPerChunkEmotions.push_back(chunkEmotions);
#endif

        appendToGUIstate(" ");
    }
    appendToGUIstate("]");

    // Now we call the voting subroutine, which will return either a single int >= 0 or -1 if ambivalent (in this case, the resultBest vector will contain true values corresponding the emotions that fall into the thresholdDistance distance)
    int result = ambivalentSoftVoting(res, this->NUM_EMOTIONS, this->topPredictedEmotions);

    appendToGUIstate("\nSoftVotingResult: ");
    if (result == -1) {  // ambivalent
        appendToGUIstate("Ambivalent! (");
        for (size_t v = 0; v < topPredictedEmotions.size(); ++v)
            if (topPredictedEmotions[v])
                appendToGUIstate(emotions[v] + ",");
        popBackGUIstate();
        appendToGUIstate(")");
    } else {
        appendToGUIstate(emotions[result] + " (id: " + std::to_string(result) + ")");
    }

    labelsWritten = true;
    // this->oscSender.sendMessage("/emotion",topPredictedEmotions);
    this->oscSender.send4Bool("/emotion", topPredictedEmotions[0],
                              topPredictedEmotions[1],
                              topPredictedEmotions[2],
                              topPredictedEmotions[3]);
    this->oscSender.sendMessage("/state", (int)STATE_IDLE);

#ifdef GENERATE_AUDACITY_LABELS
    std::string emotionLabels = "", softmaxLabels = "";

    float firstStartTime = 0.0f, lastEndTime = 0.0f;
    for (size_t i = 0; i < numChunks; ++i) {
        float startTime = std::get<0>(outputLabels[i]);
        float endTime = std::get<1>(outputLabels[i]);
        if (i == 0)
            firstStartTime = startTime;
        lastEndTime = endTime;
        // float startTime = (startIdx * SD_HOP_SIZE / (float)SD_SAMPLERATE) + (i * 3.0f);
        // float endTime = startTime + 3.0f;
        if (_verbose) std::cout << "Labeling chunk " << i << " which goes from " << startTime << " to " << endTime << std::endl;
        emotionLabels += std::to_string(startTime) + "\t" + std::to_string(endTime) + "\t" + allPerChunkEmotions[i] + "\n";
        softmaxLabels += std::to_string(startTime) + "\t" + std::to_string(endTime) + "\t" + softmaxOutsPrintable[i] + "\n";
    }
    std::string topemo = getStrEmotion(this->topPredictedEmotions, emotions);
    emotionLabels += std::to_string(firstStartTime) + "\t" + std::to_string(lastEndTime) + "\t" + topemo + "\n";

    // std::cout << emotionLabels << std::endl;
    // std::cout << softmaxLabels << std::endl;

    std::replace( topemo.begin(), topemo.end(), '/', '+'); 
    std::cout << "\"" << topemo << "\"" << std::endl;

    // remove extension from audioFilePath, append -audacity-emotion-labels.txt and write emotionLabels
    std::string audacityEmotionLabelsFilePath = audioFilePath.substr(0, audioFilePath.find_last_of(".")) + "-res"+topemo+"-audacity-emotion-labels.txt";
    std::ofstream audacityEmotionLabelsFile(audacityEmotionLabelsFilePath);
    audacityEmotionLabelsFile << emotionLabels;
    audacityEmotionLabelsFile.close();

    // remove extension from audioFilePath, append -audacity-softmax-labels.txt and write softmaxLabels
    std::string audacitySoftmaxLabelsFilePath = audioFilePath.substr(0, audioFilePath.find_last_of(".")) + "-res"+topemo+"-audacity-softmax-labels.txt";
    std::ofstream audacitySoftmaxLabelsFile(audacitySoftmaxLabelsFilePath);
    audacitySoftmaxLabelsFile << softmaxLabels;
    audacitySoftmaxLabelsFile.close();

    // // remove extension from audioFilePath, append -result.txt and write result
    // std::string audacityResultFilePath = audioFilePath.substr(0, audioFilePath.find_last_of(".")) + "-result.txt";
    // std::ofstream audacityResultFile(audacityResultFilePath);

#endif
}

void ECProcessor::updateRecState() {
    bool recState = ((AudioParameterBool *)valueTreeState.getParameter(RECSTATE_ID))->get();
    recState = bool(int(recState) * int(this->enableRec.load()));  // Disable recording if enableRec is false

    if (this->oldRecState != recState) {
        if (recState) {
            startRecording(NUM_CHANNELS);
        } else {
            stopRecording();
            recordingStopped = true;
            extractAndClassify(audioFilename, VERBOSE_EXTRACT_CLASSIFY);
#ifdef DO_REMOVE_OLD_RECORDINGS
            lastRecording2.deleteFile();
#endif
        }
        this->oldRecState = recState;
    }
}

void ECProcessor::updateGain() {
    inputGain = ((AudioParameterFloat *)valueTreeState.getParameter(GAIN_ID))->get();
}
void ECProcessor::updateSilenceThresh() {
    float val = ((AudioParameterFloat *)valueTreeState.getParameter(SILENCE_THRESH_ID))->get();
    if (val != silenceThreshold.load()) {
        silenceThreshold.store(val);
        silenceDetector.setThreshold(silenceThreshold.load());
    }
}

/** Create the parameters to add to the value tree state
 * In this case only the boolean recording state (true = rec, false = stop)
 */
AudioProcessorValueTreeState::ParameterLayout ECProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<AudioParameterBool>(RECSTATE_ID, RECSTATE_NAME, false));
    parameters.push_back(std::make_unique<AudioParameterFloat>(GAIN_ID, GAIN_NAME,
                                                               NormalisableRange<float>(0.0f,
                                                                                        1.0f,
                                                                                        0.0001,
                                                                                        0.4,
                                                                                        false),
                                                               1.f));
    // parameters.push_back(std::make_unique<AudioParameterFloat>(SILENCE_THRESH_ID, SILENCE_THRESH_NAME,
    //                                                            NormalisableRange<float>(-120.0f, 0.0f, 0.5f),
    //                                                            -60.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>(SILENCE_THRESH_ID, SILENCE_THRESH_NAME, -120.0f, 0.0f, -60.0f));

    return {parameters.begin(), parameters.end()};
}

void ECProcessor::releaseResources() {
}

void ECProcessor::getStateInformation(juce::MemoryBlock &destData) {
    juce::MemoryOutputStream stream(destData, false);
    valueTreeState.state.writeToStream(stream);
}

void ECProcessor::setStateInformation(const void *data, int sizeInBytes) {
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid() == false)
        return;

    // DBG("Reading XML:");
    // DBG(tree.toXmlString());

    valueTreeState.replaceState(tree);

    Value saveFolderPath = valueTreeState.state.getPropertyAsValue("REC_SAVEPATH", nullptr, true);
    // saveFolderPath.setValue(saveFolder.getFullPathName()); // Writing the XML after this should show the "foobar.midi"
    // DBG("Save folder path: " + saveFolderPath.getValue().toString());
    juce::File tmpSaveFolder(saveFolderPath.getValue().toString());
    if (tmpSaveFolder.exists() && tmpSaveFolder.isDirectory()) {
        this->setSaveFolder(File(saveFolderPath.getValue().toString()));
    } else {
        DBG("Save folder path does not exist");
    }

    // Same for Model path "SV_MODEL_PATH"
    Value sv_model_path = valueTreeState.state.getPropertyAsValue("SV_MODEL_PATH", nullptr, true);
    std::string restoredModelPath = sv_model_path.getValue().toString().toStdString();

    // if restoredModelPath is empty do nothing
    if (restoredModelPath.empty()) {
        DBG("Model path is empty");
    } else {
        if (restoredModelPath == "BINARY_" + this->ELECTRIC_GUITAR_MODELNAME) {
            this->loadModelFromBinaryData(this->ELECTRIC_GUITAR_MODELNAME);
        } else if (restoredModelPath == "BINARY_" + this->ACOUSTIC_GUITAR_MODELNAME) {
            this->loadModelFromBinaryData(this->ACOUSTIC_GUITAR_MODELNAME);
        } else if (restoredModelPath == "BINARY_" + this->PIANO_MODELNAME) {
            this->loadModelFromBinaryData(this->PIANO_MODELNAME);
        } else if (this->loadModel(restoredModelPath)) {
            DBG("Model loaded successfully");
        } else {
            DBG("Model loading failed");
        }
    }
}

void ECProcessor::setSaveFolder(const File &saveDir) {
    this->saveDir = saveDir;
    Value saveFolderPath = valueTreeState.state.getPropertyAsValue("REC_SAVEPATH", nullptr, true);
    saveFolderPath.setValue(this->saveDir.getFullPathName());  // Writing the XML after this should show the "foobar.midi"
}

File &ECProcessor::getSaveFolder() {
    return this->saveDir;
}

float ECProcessor::getRMSValue() const {
    return rmsValue.getCurrentValue();
}

float ECProcessor::getPeakValue() const {
    return peakValue.getCurrentValue();
}

void ECProcessor::setInputGain(float newGain) {
    this->inputGain = newGain;
}

//==============================================================================
juce::AudioProcessorEditor *ECProcessor::createEditor() { return new ECEditor(*this); }

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new ECProcessor(); }

#ifndef JucePlugin_PreferredChannelConfigurations
bool ECProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
    #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

            // This checks if the input layout matches the output layout
        #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
        #endif

    return true;
    #endif
}
#endif
