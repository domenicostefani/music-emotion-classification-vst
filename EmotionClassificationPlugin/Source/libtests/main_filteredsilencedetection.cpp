

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

    const float sampleRate = 16000;
    const int frameSize = 512;
    const int stepSize = 256;

    std::vector<essentia::Real> signalVec;

    essentia::init();

    signalVec = essentiaLoad(audioFilename, sampleRate);

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

    std::cout << "myResults.size(): " << myResults.size() << std::endl;

    essentia::shutdown();

    // |--------------------------------------------------------------|
    // |-- Filter silence burst --------------------------------------|
    // |--------------------------------------------------------------|

    const size_t FILTER_LENGTH = 187;
    emosmi::SilenceFilter silenceFilter(FILTER_LENGTH, 0.0001, 0.5);

    // process myres vector
    std::vector<bool> myResultsFiltered;
    for (size_t i = 0; i < myResults.size(); ++i) {
        myResultsFiltered.push_back(silenceFilter.filter(myResults[i]));
    }

    size_t printto = 80;
    for (size_t i = 0; i < printto; ++i) {
        std::cout << myResults[i] << " ";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < printto; ++i) {
        std::cout << myResultsFiltered[i] << " ";
    }
    std::cout << std::endl;

    // Compress equal-adjacent labels to ranges (int,int) with their value (bool)
    std::vector<std::tuple<int, int, bool>> compactLabels, compactFilteredLabels;
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

    // Compress equal-adjacent labels to ranges (int,int) with their value (bool)
    start = 0;
    end = 0;
    value = myResultsFiltered[0];
    for (size_t idx = 1; idx < myResultsFiltered.size(); ++idx) {
        if (myResultsFiltered[idx] != value) {
            end = idx - 1;
            compactFilteredLabels.push_back(std::make_tuple(start, end, value));
            start = idx;
            value = myResultsFiltered[idx];
        }
    }
    end = myResultsFiltered.size() - 1;
    compactFilteredLabels.push_back(std::make_tuple(start, end, value));

    // Print compact labels
    // for (size_t idx = 0; idx < compactLabels.size(); ++idx) {
    //     std::cout << "[" << std::get<0>(compactLabels[idx]) << ", " << std::get<1>(compactLabels[idx]) << "] = " << std::get<2>(compactLabels[idx]) << std::endl;
    // }

    // Now write the compact labels to an audacity label file
    // frame numbers for start and end of must be converted to time.
    // When doing this, we take into consideration the fact that the fist result comes only after frameSize/sampleRate seconds
    // while all the subsequent come every stepSize/sampleRate seconds after the previous one.

    // lambda to write audacity labels
    auto writeAudacityLabels = [](std::string filename, std::vector<std::tuple<int, int, bool>> labels, float frameSize, float stepSize, float sampleRate) {
        // Open file and write text to it
        std::ofstream outfile;
        outfile.open(filename.c_str());

        int offset_steps = frameSize / stepSize - 1;
        for (size_t idx = 0; idx < labels.size(); ++idx) {
            int frameStartIdx = std::get<0>(labels[idx]),
                frameEndIdx = std::get<1>(labels[idx]);
            bool value = std::get<2>(labels[idx]);

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
        std::cout << "\"" << filename << "\" written" << std::endl;
    };

    // Write audacity labels
    writeAudacityLabels("myResults.labels.txt", compactLabels, frameSize, stepSize, sampleRate);
    writeAudacityLabels("myResultsFiltered.labels.txt", compactFilteredLabels, frameSize, stepSize, sampleRate);

    return 0;
}