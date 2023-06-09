

#include <iostream>

#include "extractionpipeline.h"

using namespace std;
using namespace essentia;

emosmi::MusicnnFeatureExtractor extractionPipeline;

void usage(char *progname) {
    cout << "Error: wrong number of arguments" << endl;
    cout << "Usage: " << progname << " input_audiofile" << endl;

    exit(1);
}

void print2DVectorHead(const vector<vector<Real>> &vec, int n = 5, int m = 5) {
    if (vec.size() < n)
        throw EssentiaException("Vector size is smaller than n (" + to_string(vec.size()) + " < " + to_string(n) + ")");
    if (vec[0].size() < m)
        throw EssentiaException("Inner vector size is smaller than n (" + to_string(vec[0].size()) + " < " + to_string(m) + ")");
    for (int i = 0; i < n - 1; i++) {
        printf("[ ");
        for (int j = 0; j < m - 1; j++)
            printf("%.8f ", vec[i][j]);
        printf("... %.8f]", vec[i][vec[i].size() - 1]);

        cout << endl;
    }
    for (int j = 0; j < m - 1; j++)
        printf("     ...        ");
    printf("\n[ ");
    int i = vec.size() - 1;
    for (int j = 0; j < m - 1; j++)
        printf("%.8f ", vec[i][j]);
    printf("... %.8f]\n", vec[i][vec[i].size() - 1]);
}

int main(int argc, char *argv[]) {
    string audioFilename, outputFilename, profileFilename;

    switch (argc) {
        case 2:
            audioFilename = argv[1];
            break;
        default:
            usage(argv[0]);
    }

    cout << "Analyzing " << audioFilename << endl;
    std::vector<std::vector<Real>> bandsVec;
    std::vector<bool> isSilent;

    extractionPipeline.extractFromFile(audioFilename, bandsVec);
    float threshold_dB = -80.0f;
    float threshold = pow(10.0, threshold_dB / 10.0);
    extractionPipeline.areFramesSilent(audioFilename, isSilent, threshold);

    cout << "size(output): " << bandsVec.size() << "x" << bandsVec[0].size() << endl;
    cout << "head(output): " << endl;
    print2DVectorHead(bandsVec);

    cout << "size(isSilent): " << isSilent.size() << endl;
    cout << "isSilent {threshold:" << threshold_dB << "dB (" << threshold << ")"
         << " }: " << endl;
    for (int i = 0; i < isSilent.size(); i++)
        cout << isSilent[i] << " ";

    const float FRAMES_PER_CHUNK = 10;
    const float duration = (256.0f * (FRAMES_PER_CHUNK)) / 16000.0f;
    std::cout << "duration: " << duration << std::endl;

    size_t numChunks = isSilent.size() / FRAMES_PER_CHUNK;
    cout << "numChunks: " << numChunks << endl;

    for (size_t i = 0; i < numChunks; ++i) {
        size_t countForSilent = 0;
        for (size_t j = 0; j < FRAMES_PER_CHUNK; ++j)
            if (isSilent[i * FRAMES_PER_CHUNK + j])
                countForSilent++;

        if (countForSilent > 0.5 * FRAMES_PER_CHUNK)
            cout << std::to_string((float)(i * duration)) + "\t" + std::to_string((float)((i + 1) * duration)) + "\tSilent\n";
        // else
        //     cout << std::to_string((float)(i * duration)) + "\t" + std::to_string((float)((i + 1) * duration)) + "\t\n";

        //     cout << "Chunk " << i << " is not silent" << endl;
    }

    return 0;
}