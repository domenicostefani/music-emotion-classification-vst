/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "extractionpipeline.h"

// #define DUMMY_INFERENCE // TODO: remove this

// #define VERBOSE_PRINT

#ifdef VERBOSE_PRINT
#include "verbose_utils.h"
#endif

// #define DO_REMOVE_OLD_RECORDINGS
#define GENERATE_AUDACITY_LABELS
#define VERBOSE_CLASSIFICATION true

#ifdef JUCE_ARM
#define ELK_OS_ARM
#endif

std::map<size_t, std::string> emotions = {
    {0, "Aggressive"},
    {1, "Relaxed"},
    {2, "Happy"},
    {3, "Sad"}};

ClassifierPtr EmotionClassificationPluginAudioProcessor::tfliteClassifier = nullptr;

//==============================================================================
EmotionClassificationPluginAudioProcessor::EmotionClassificationPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      valueTreeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    suspendProcessing(true);

   #ifndef DUMMY_INFERENCE
    // std::string MODEL_PATH = "/home/cimil-01/Develop/emotionally-aware-SMIs/EmotionClassificationPlugin/Builds/linux-amd64/MSD_musicnn.tflite";
    // std::string MODEL_PATH = "/home/cimil-01/Develop/emotionally-aware-SMIs/EmotionClassificationPlugin/convdense_testmodel.tflite";
    
    std::string MODEL_PATH = "/home/cimil-01/Develop/instrument_emotion_recognition/keras_audio_models/tflite/convdense_testmodel.tflite";



    // Check that file exists
    std::ifstream f(MODEL_PATH);
    if (!f.good())
    {
        std::cout << "Model file not found at " << MODEL_PATH << std::endl;
        exit(1);
    } 
    EmotionClassificationPluginAudioProcessor::tfliteClassifier = createClassifier(MODEL_PATH, true); // true = verbose cout
   #endif
    suspendProcessing(false);
}

EmotionClassificationPluginAudioProcessor::~EmotionClassificationPluginAudioProcessor()
{
}

void EmotionClassificationPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    recorder.prepareToPlay(sampleRate);
}

void EmotionClassificationPluginAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
    updateRecState();
    recorder.writeBlock(buffer.getArrayOfReadPointers(), buffer.getNumSamples());
}

std::string getStrTime()
{
    auto curtime = Time::getCurrentTime();

    std::string timestr = std::to_string(curtime.getYear());
    const size_t FIELD_LENGTH = 2;
    std::string toWrite = "";
    toWrite = std::to_string(curtime.getMonth());
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

void EmotionClassificationPluginAudioProcessor::startRecording(unsigned int numChannels)
{
#ifdef RECORDER_DEBUG_LOG
    logText("Recording started");
#endif
    if (!RuntimePermissions::isGranted(RuntimePermissions::writeExternalStorage))
    {
        RuntimePermissions::request(RuntimePermissions::writeExternalStorage,
                                    [this, numChannels](bool granted) mutable
                                    {
                                        if (granted)
                                            this->startRecording(numChannels);
                                    });
        return;
    }


    // Get current time
    std::string timestr = getStrTime();
    this->lastRecording = saveDir.getNonexistentChildFile("EmoAwSMIs-recording-" + timestr, ".wav");
    this->lastRecording2 = this->lastRecording;
    this->extractorState = "Recording to disk: \"";
    this->audioFilename = lastRecording.getFullPathName().toStdString();
    this->extractorState = this->extractorState + audioFilename;
    this->extractorState = this->extractorState + "\"...";

#ifdef RECORDER_DEBUG_LOG
    logText("Recording " + lastRecording.getFullPathName().toStdString());
#endif

    // std::cout << "Recording " << lastRecording.getFullPathName().toStdString() << std::endl;

    recorder.startRecording(lastRecording, numChannels);
}

void EmotionClassificationPluginAudioProcessor::stopRecording()
{
#ifdef RECORDER_DEBUG_LOG
    logText("Recording stopped");
#endif
    recorder.stop();

    this->extractorState = this->extractorState + "\nStopped recording.";

#if (JUCE_ANDROID || JUCE_IOS)
    SafePointer<AudioRecordingDemo> safeThis(this);
    File fileToShare = lastRecording;

    ContentSharer::getInstance()->shareFiles(Array<URL>({URL(fileToShare)}),
                                             [safeThis, fileToShare](bool success, const String &error)
                                             {
#ifdef RECORDER_DEBUG_LOG
                                                 logText("Android/iOS ContentSharer callback called");
#endif
                                                 if (fileToShare.existsAsFile())
                                                 {
#ifdef RECORDER_DEBUG_LOG
                                                     logText("File existed, deleting...");
#endif
                                                     fileToShare.deleteFile();
                                                 }

#ifdef RECORDER_DEBUG_LOG
                                                 logText("SUCCESS: " + (success ? "true" : "false"));
                                                 logText("ERROR: " + error.toStdString());
#endif
                                                 if (!success && error.isNotEmpty())
                                                 {
                                                     NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                                                           "Sharing Error",
                                                                                           error);
                                                 }
                                             });
#endif

    lastRecording = File();
}

void classifyChunk(const std::vector<std::vector<float>> &input, std::vector<float> &output)
{
    // Flatten the input
    size_t n_rows = input.size();
    size_t n_cols = input[0].size();
    std::vector<float> flat_input(n_rows * n_cols);
    for (size_t i = 0; i < n_rows; ++i)
    {
        if (input[i].size() != n_cols)
            throw std::runtime_error("Input is not a matrix (Row " + std::to_string(i) + " has " + std::to_string(input[i].size()) + " columns instead of " + std::to_string(n_cols) + ")");
        for (size_t j = 0; j < n_cols; ++j)
            flat_input[i * n_cols + j] = input[i][j];
    }

   #ifdef DUMMY_INFERENCE
    output[0] = 0.0f;
    output[1] = 1.0f;
    output[2] = 0.0f;
    output[3] = 0.0f;
   #else
    classifyFlat2D(EmotionClassificationPluginAudioProcessor::tfliteClassifier,\
                    flat_input.data(),\
                    n_rows,\
                    n_cols,\
                    output.data(),\
                    output.size(), VERBOSE_CLASSIFICATION);
   #endif
}

size_t softVoting(const std::vector<std::vector<float>> &input, const size_t NUM_EMOTIONS)
{
    std::vector<float> sum(NUM_EMOTIONS, 0.0f);
    // Sum the SoftMax probabilities for each emotion
    for (auto &v : input)
        for (size_t i = 0; i < NUM_EMOTIONS; ++i)
            sum[i] += v[i];
    // Argmax
    size_t best = 0;
    for (size_t i = 1; i < NUM_EMOTIONS; ++i)
        if (sum[i] > sum[best])
            best = i;

    return best;
}

void EmotionClassificationPluginAudioProcessor::extractAndClassify(std::string audioFilePath)
{
    // TODO: implement
#ifdef VERBOSE_PRINT
    std::cout << "Extracting and classifying " << audioFilePath << std::endl;
    std::cout << "File \"" << audioFilePath << "\"" << audioFilePath << std::endl;
#endif
    std::vector<std::vector<essentia::Real>> featvec;

    // Check that the file exists
    File tmpfile = File(audioFilePath);
    if (!tmpfile.exists())
    {
#ifdef VERBOSE_PRINT
        printf("File %s DOES NOT EXIST\n", audioFilePath.c_str());
#endif
        std::cout << std::flush;
        throw std::logic_error("File " + audioFilePath + " does not exist!");
    }
    else
    {
#ifdef VERBOSE_PRINT
        printf("File %s exists\n", audioFilePath.c_str());
#endif
        std::cout << std::flush;
    }

    // Extract features

    extractFromFile(audioFilePath, featvec);
#ifdef VERBOSE_PRINT
    DBG("Extracted features:\n");
#endif

    extractorState = extractorState + "\nExtracted " + std::to_string(featvec[0].size()) + " features from " + std::to_string(featvec.size()) + " frames";

#ifdef VERBOSE_PRINT
    print2DVectorHead(featvec);
#endif

    // Now we split the feature matrix in 3 second chunks (187 frames), call the classifier and then call voting subroutine
    size_t numFrames = featvec.size();
    size_t numChunks = numFrames / FRAMES_IN_3_SECONDS;

    if (numChunks == 0)
    {
        extractorState = extractorState + "\nNot enough frames to classify (Please record for more than 3 seconds)";
        return;
    }

    extractorState = extractorState + "\nSplitting into " + std::to_string(numChunks) + " chunks of 3 seconds each";

    std::vector<std::vector<float>> res;
    res.resize(numChunks);
    extractorState = extractorState + "\nPer Chunk winners: [ ";
    for (size_t i = 0; i < numChunks; ++i)
    {
        res.at(i).resize(NUM_EMOTIONS);
        classifyChunk(std::vector<std::vector<float>>(featvec.begin() + i * FRAMES_IN_3_SECONDS, featvec.begin() + (i + 1) * FRAMES_IN_3_SECONDS), res.at(i));
        // Argmax for log
        int maxIndex = 0;
        for (int j = 0; j < NUM_EMOTIONS; ++j)
            if (res.at(i).at(j) > res.at(i).at(maxIndex))
                maxIndex = j;
        extractorState = extractorState + std::to_string(maxIndex) + " ";
    }
    extractorState = extractorState + "]";

    int result = softVoting(res, NUM_EMOTIONS);
    extractorState = extractorState + "\nSoftVotingResult: " + emotions[result] + " (id: " + std::to_string(result) + ")";

#ifdef GENERATE_AUDACITY_LABELS
    std::string emotionLabels = "";
    std::string softmaxLabels = "";
    for (size_t i = 0; i < numChunks; ++i)
    {
        // Argmax for log
        int maxIndex = 0;
        for (int j = 0; j < NUM_EMOTIONS; ++j)
            if (res.at(i).at(j) > res.at(i).at(maxIndex))
                maxIndex = j;
        emotionLabels += std::to_string((float)(i * 3)) + "\t" + std::to_string((float)((i + 1) * 3)) + "\t" + emotions[maxIndex] + "\n";
        std::string softmaxOut = "[ ";
        for (size_t j = 0; j < NUM_EMOTIONS; ++j)
            softmaxOut += std::to_string(res.at(i).at(j)) + " ";
        softmaxOut += "]";
        softmaxLabels += std::to_string((float)(i * 3)) + "\t" + std::to_string((float)((i + 1) * 3)) + "\t" + softmaxOut + "\n";
    }

    // std::cout << emotionLabels << std::endl;
    // std::cout << softmaxLabels << std::endl;
    // remove extension from audioFilePath, append -audacity-emotion-labels.txt and write emotionLabels
    std::string audacityEmotionLabelsFilePath = audioFilePath.substr(0, audioFilePath.find_last_of(".")) + "-audacity-emotion-labels.txt";
    std::ofstream audacityEmotionLabelsFile(audacityEmotionLabelsFilePath);
    audacityEmotionLabelsFile << emotionLabels;
    audacityEmotionLabelsFile.close();

    // remove extension from audioFilePath, append -audacity-softmax-labels.txt and write softmaxLabels
    std::string audacitySoftmaxLabelsFilePath = audioFilePath.substr(0, audioFilePath.find_last_of(".")) + "-audacity-softmax-labels.txt";
    std::ofstream audacitySoftmaxLabelsFile(audacitySoftmaxLabelsFilePath);
    audacitySoftmaxLabelsFile << softmaxLabels;
    audacitySoftmaxLabelsFile.close();

#endif
}

void EmotionClassificationPluginAudioProcessor::updateRecState()
{
    bool recState = ((AudioParameterBool *)valueTreeState.getParameter(RECSTATE_ID))->get();

    if (this->oldRecState != recState)
    {
        if (recState)
        {
            startRecording(NUM_CHANNELS);
        }
        else
        {
            stopRecording();
            // Sleep for 5 seconds
            // extractAndClassify(lastRecording.getFullPathName().toStdString());
            printf("Just stoppend recording %s\n", audioFilename.c_str());
            extractAndClassify(audioFilename);
#ifdef DO_REMOVE_OLD_RECORDINGS
            lastRecording2.deleteFile(); // TODO: fix this because something is not working with lastRecording
#endif
        }
        this->oldRecState = recState;
    }
}

/** Create the parameters to add to the value tree state
 * In this case only the boolean recording state (true = rec, false = stop)
 */
AudioProcessorValueTreeState::ParameterLayout EmotionClassificationPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<AudioParameterBool>(RECSTATE_ID, RECSTATE_NAME, false));
    return {parameters.begin(), parameters.end()};
}

void EmotionClassificationPluginAudioProcessor::releaseResources()
{
}

void EmotionClassificationPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    valueTreeState.state.writeToStream (stream);

    // DBG("Writing to XML:");
    // DBG(valueTreeState.state.toXmlString());
}

void EmotionClassificationPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t (sizeInBytes));
    if (tree.isValid() == false)
        return;

    // DBG("Reading XML:");
    // DBG(tree.toXmlString());

    valueTreeState.replaceState (tree);

    
    Value saveFolderPath = valueTreeState.state.getPropertyAsValue("REC_SAVEPATH", nullptr, true);
    // saveFolderPath.setValue(saveFolder.getFullPathName()); // Writing the XML after this should show the "foobar.midi"
    // DBG("Save folder path: " + saveFolderPath.getValue().toString());
    this->setSaveFolder(File(saveFolderPath.getValue().toString()));
    
}


void EmotionClassificationPluginAudioProcessor::setSaveFolder (const File& saveFolder) {
    Value saveFolderPath = valueTreeState.state.getPropertyAsValue("REC_SAVEPATH", nullptr, true);
    saveFolderPath.setValue(saveFolder.getFullPathName()); // Writing the XML after this should show the "foobar.midi"
    saveDir = saveFolder;
}

File& EmotionClassificationPluginAudioProcessor::getSaveFolder() {
    return saveDir;
}


















//==============================================================================
bool EmotionClassificationPluginAudioProcessor::acceptsMidi() const { return false; }
bool EmotionClassificationPluginAudioProcessor::producesMidi() const { return false; }
bool EmotionClassificationPluginAudioProcessor::isMidiEffect() const { return false; }
double EmotionClassificationPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EmotionClassificationPluginAudioProcessor::getNumPrograms() { return 1; }
int EmotionClassificationPluginAudioProcessor::getCurrentProgram() { return 0; }
void EmotionClassificationPluginAudioProcessor::setCurrentProgram(int index) {}
const juce::String EmotionClassificationPluginAudioProcessor::getProgramName(int index) { return {}; }
void EmotionClassificationPluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {}
bool EmotionClassificationPluginAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor *EmotionClassificationPluginAudioProcessor::createEditor() { return new EmotionClassificationPluginAudioProcessorEditor(*this); }




juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new EmotionClassificationPluginAudioProcessor(); }
const juce::String EmotionClassificationPluginAudioProcessor::getName() const { return JucePlugin_Name; }

#ifndef JucePlugin_PreferredChannelConfigurations
bool EmotionClassificationPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
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
