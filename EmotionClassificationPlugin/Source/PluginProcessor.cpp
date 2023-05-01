/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "extractionpipeline.h"

#define DO_REMOVE_OLD_RECORDINGS

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

#if (JUCE_ANDROID || JUCE_IOS)
    auto parentDir = File::getSpecialLocation(File::tempDirectory);
#else
    auto parentDir = File::getSpecialLocation(File::userHomeDirectory);
#endif

    this->lastRecording = parentDir.getNonexistentChildFile("AudioRecording", ".wav");
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


void print2DVectorHead(const std::vector<std::vector<essentia::Real>> &vec, int n = 5, int m = 5)
{
    if (vec.size() < n)
        throw essentia::EssentiaException("Vector size is smaller than n ("+std::to_string(vec.size())+" < " + std::to_string(n) + ")");
    if (vec[0].size() < m)
        throw essentia::EssentiaException("Inner vector size is smaller than n ("+std::to_string(vec[0].size())+" < " + std::to_string(m) + ")");
    for (int i = 0; i < n-1; i++) {
        printf("[ ");
        for (int j = 0; j < m-1; j++)
            printf("%.8f ", vec[i][j]);
        printf("... %.8f]", vec[i][vec[i].size()-1]);
        printf("\n");
    }
    for (int j = 0; j < m-1; j++)
        printf("     ...        ");
    printf("\n[ ");
    int i = vec.size()-1;
    for (int j = 0; j < m-1; j++)
        printf("%.8f ", vec[i][j]);
    printf("... %.8f]\n", vec[i][vec[i].size()-1]);
}

void EmotionClassificationPluginAudioProcessor::extractAndClassify(std::string audioFilePath)
{
    // TODO: implement
    std::cout << "Extracting and classifying " << audioFilePath << std::endl;
    std::cout << "File \"" << audioFilePath << "\"" << audioFilePath << std::endl;
    std::vector<std::vector<essentia::Real>> featvec;

    // Check that the file exists
    File tmpfile = File(audioFilePath);
    if(!tmpfile.exists()){
        printf("File %s DOES NOT EXIST\n", audioFilePath.c_str());
        std::cout << std::flush;
        throw std::logic_error("File " + audioFilePath + " does not exist!");
    } else {
        printf("File %s exists\n", audioFilePath.c_str());
        std::cout << std::flush;
    }

    // Extract features

    extractFromFile(audioFilePath,featvec);
    printf("Extracted features:\n");


    extractorState = extractorState + "\nExtracted " + std::to_string(featvec[0].size()) + " features from " + std::to_string(featvec.size()) + " frames";
    

    print2DVectorHead(featvec);
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
            lastRecording2.deleteFile(); //TODO: fix this because something is not working with lastRecording
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

//==============================================================================
const juce::String EmotionClassificationPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EmotionClassificationPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool EmotionClassificationPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool EmotionClassificationPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double EmotionClassificationPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EmotionClassificationPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int EmotionClassificationPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EmotionClassificationPluginAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String EmotionClassificationPluginAudioProcessor::getProgramName(int index)
{
    return {};
}

void EmotionClassificationPluginAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================

void EmotionClassificationPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

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

//==============================================================================
bool EmotionClassificationPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *EmotionClassificationPluginAudioProcessor::createEditor()
{
    return new EmotionClassificationPluginAudioProcessorEditor(*this);
}

//==============================================================================
void EmotionClassificationPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EmotionClassificationPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new EmotionClassificationPluginAudioProcessor();
}




