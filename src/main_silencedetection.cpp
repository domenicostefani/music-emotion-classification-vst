

#include <iostream>

#include "extractionpipeline.h"

using namespace std;
using namespace essentia;

void usage(char *progname) {
    cout << "Error: wrong number of arguments" << endl;
    cout << "Usage: " << progname << " input_audiofile [silence_threshold_db]" << endl;

    exit(1);
}

std::vector<float> essentiaLoad(std::string audioFilename, float sampleRate) {
    std::vector<essentia::Real> signalVec;
    essentia::streaming::AlgorithmFactory &factory = streaming::AlgorithmFactory::instance();

    // This loads the audio file from disk, downsamples it to 16kHz mono and outputs it
    essentia::streaming::Algorithm *audio =
        factory.create("MonoLoader", "filename", audioFilename, "sampleRate", sampleRate, "downmix", "left");

    std::vector<std::vector<essentia::Real>> framesVec;

    audio->output("audio") >> signalVec;  // Send the extractor's output to a std::vector

    essentia::scheduler::Network n(audio);
    n.run();
    n.clear();

    return signalVec;
}

std::vector<bool> essentiaComputeSR(std::string audioFilename, float sampleRate, int frameSize, int stepSize, float threshold_dB) {
    std::vector<essentia::Real> essThresholds = {pow(10.0f, threshold_dB * 0.1f)}, essentiaSilence;

    essentia::streaming::AlgorithmFactory &factory = streaming::AlgorithmFactory::instance();

    // This loads the audio file from disk, downsamples it to 16kHz mono and outputs it
    essentia::streaming::Algorithm *audio =
        factory.create("MonoLoader", "filename", audioFilename, "sampleRate", sampleRate, "downmix", "left");

    essentia::streaming::Algorithm *fc = factory.create("FrameCutter", "frameSize", frameSize, "hopSize", stepSize, "startFromZero", true,
                                                        "validFrameThresholdRatio", 1);

    // This is a compound feature extractor developed for MusiCNN [3]
    essentia::streaming::Algorithm *silenceRate = factory.create("SilenceRate", "thresholds", essThresholds);

    std::vector<std::vector<essentia::Real>> framesVec;

    audio->output("audio") >> fc->input("signal");
    fc->output("frame") >> silenceRate->input("frame");
    silenceRate->output("threshold_0") >> essentiaSilence;

    essentia::scheduler::Network n(audio);
    n.run();
    n.clear();

    std::vector<bool> essentiaSilenceBool;
    for (int i = 0; i < essentiaSilence.size(); i++) {
        essentiaSilenceBool.push_back(essentiaSilence[i] > 0.5f);
    }
    return essentiaSilenceBool;
}

int main(int argc, char *argv[]) {
    string audioFilename, outputFilename, profileFilename;
    float threshold = -80.0f;

    switch (argc) {
        case 2:
            audioFilename = argv[1];
            break;
        case 3:
            audioFilename = argv[1];
            try {
                threshold = stof(argv[2]);
            } catch (std::invalid_argument) {
                usage(argv[0]);
            }
            break;
        default:
            usage(argv[0]);
    }

#define DO_COMPARE_WITH_ESSENTIA

    const float sampleRate = 16000;
    const int frameSize = 512;
    const int stepSize = 256;

    std::vector<essentia::Real> signalVec;

    essentia::init();

    signalVec = essentiaLoad(audioFilename, sampleRate);

#ifdef DO_COMPARE_WITH_ESSENTIA
    std::vector<bool> essentiaSilence = essentiaComputeSR(audioFilename, sampleRate, frameSize, stepSize, threshold);
#endif

    emosmi::RTisSilent isSilent(frameSize, stepSize, sampleRate);
    isSilent.setThreshold(threshold);
    std::vector<bool> myResults;

    // Simulate real time processing
    for (size_t i = 0; i < signalVec.size() / 64; ++i) {
        std::vector<float> audioBlock(64);
        for (size_t j = 0; j < 64; ++j)
            audioBlock[j] = signalVec[i * 64 + j];

        bool res = false;
        const bool verbose = false;
        if (isSilent.storeBlockAndCompute(audioBlock, res, verbose)) {
            // std::cout << "Silence rate (threshold: " << threshold << "dB): " << (res ? "true" : "false") << std::endl;
            myResults.push_back(res);
        }
    }

    // print sizes
    std::cout << "signalVec.size(): " << signalVec.size() << std::endl;
#ifdef DO_COMPARE_WITH_ESSENTIA
    std::cout << "essentiaSilence.size(): " << essentiaSilence.size() << std::endl;
#endif

    std::cout << "myResults.size(): " << myResults.size() << std::endl;

#ifdef DO_COMPARE_WITH_ESSENTIA
    if (myResults.size() != essentiaSilence.size()) {
        std::cerr << "Error: myResults.size() != essentiaSilence.size() (" << myResults.size() << " != " << essentiaSilence.size() << ")" << std::endl;
        return -1;
    }
#endif

#ifdef DO_COMPARE_WITH_ESSENTIA
    std::cout << "Comparing results..." << std::endl;
    for (size_t idx = 0; idx < myResults.size(); ++idx) {
        // std::cout << myResults[idx] << " " <<  (essentiaSilence[idx] >0.5) << std::endl;
        if (myResults[idx] != (essentiaSilence[idx])) {
            std::cerr << "Error: myResults[" << idx << "] != essentiaSilence[" << idx << "]" << std::endl;
            return -1;
        }
    }
    std::cout << "OK" << std::endl;

#endif

    essentia::shutdown();

    // std::cout << "Is frame silent? (threshold: " << threshold << "dB): " << std::endl;
    // for (size_t idx = 0; idx < myResults.size(); ++idx) {
    //     std::cout << "[" << idx << "] = " <<  myResults[idx] << std::endl;
    // }

    // Compress equal-adjacent labels to ranges (int,int) with their value (bool)
    std::vector<std::tuple<int, int, bool>> compactLabels;
    int start = 0;
    int end = 0;
    bool value = myResults[0];
    for (size_t idx = 1; idx < myResults.size(); ++idx) {
        if (myResults[idx] != value) {
            end = idx - 1;
            compactLabels.push_back(std::make_tuple(start, end, value));
            start = idx;
            value = myResults[idx];
        }
    }
    end = myResults.size() - 1;
    compactLabels.push_back(std::make_tuple(start, end, value));

    // Print compact labels
    for (size_t idx = 0; idx < compactLabels.size(); ++idx) {
        std::cout << "[" << std::get<0>(compactLabels[idx]) << ", " << std::get<1>(compactLabels[idx]) << "] = " << std::get<2>(compactLabels[idx]) << std::endl;
    }

    // Now write the compact labels to an audacity label file
    // frame numbers for start and end of must be converted to time.
    // When doing this, we take into consideration the fact that the fist result comes only after frameSize/sampleRate seconds
    // while all the subsequent come every stepSize/sampleRate seconds after the previous one.

    // Open file and write text to it
    std::ofstream outfile;
    outfile.open("labels.txt");

    int offset_steps = frameSize / stepSize - 1;
    for (size_t idx = 0; idx < compactLabels.size(); ++idx) {
        int frameStartIdx = std::get<0>(compactLabels[idx]),
            frameEndIdx = std::get<1>(compactLabels[idx]);
        bool value = std::get<2>(compactLabels[idx]);

        float startTime, endTime;
        if (frameStartIdx == 0) {  // first label
            startTime = 0.0;
        } else {
            startTime = (frameStartIdx + offset_steps) * stepSize / sampleRate;
        }
        endTime = (frameEndIdx + offset_steps + 1) * stepSize / sampleRate;

        // Create char buffer, sprintf audacity labels, and output to file
        char buffer[1000];
        sprintf(buffer, "%.5f\t%.5f\t%s\n", startTime, endTime, value ? "silence" : "");
        outfile << buffer;
    }

    outfile.close();
    std::cout << "\"./labels.txt\" written" << std::endl;

    return 0;
}