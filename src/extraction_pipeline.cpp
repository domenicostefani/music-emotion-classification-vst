/*
 * Extraction pipeline for the instrument emotion recognition project/emotionally aware Smart Musical Instruments project
 * Essentia: http://essentia.upf.edu. 
 * Essentia Paper: Bogdanov, D., Wack N., Gómez E., Gulati S., Herrera P., Mayor O., et al. (2013). ESSENTIA: an Audio Analysis Library for Music Information Retrieval. International Society for Music Information Retrieval Conference (ISMIR'13). 493-498.
 * 
 * Domenico Stefani 2023
 */

#include <iostream>
#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/vectoroutput.h>

#include <essentia/scheduler/network.h>

using namespace std;
using namespace essentia;
using namespace essentia::streaming;
using namespace essentia::scheduler;

void usage(char *progname)
{
    cout << "Error: wrong number of arguments" << endl;
    cout << "Usage: " << progname << " input_audiofile" << endl;

    exit(1);
}

// void print2DVectorHead(const vector<vector<Real>> &vec, int n = 5, int m = 5)
// {
//     cout << "Printing first " << m << " elements of the first " << n << " rows of 2D vector" << endl;
//     if (vec.size() < n)
//         throw EssentiaException("Vector size is smaller than n ("+to_string(vec.size())+" < " + to_string(n) + ")");
//     if (vec[0].size() < m)
//         throw EssentiaException("Inner vector size is smaller than n ("+to_string(vec[0].size())+" < " + to_string(m) + ")");
//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < m; j++)
//             printf("%.8f ", vec[i][j]);

//         cout << endl;
//     }
// }


void print2DVectorHead(const vector<vector<Real>> &vec, int n = 5, int m = 5)
{
    if (vec.size() < n)
        throw EssentiaException("Vector size is smaller than n ("+to_string(vec.size())+" < " + to_string(n) + ")");
    if (vec[0].size() < m)
        throw EssentiaException("Inner vector size is smaller than n ("+to_string(vec[0].size())+" < " + to_string(m) + ")");
    for (int i = 0; i < n-1; i++) {
        printf("[ ");
        for (int j = 0; j < m-1; j++)
            printf("%.8f ", vec[i][j]);
        printf("... %.8f]", vec[i][vec[i].size()-1]);

        cout << endl;
    }
    for (int j = 0; j < m-1; j++)
        printf("     ...        ");
    printf("\n[ ");
    int i = vec.size()-1;
    for (int j = 0; j < m-1; j++)
        printf("%.8f ", vec[i][j]);
    printf("... %.8f]\n", vec[i][vec[i].size()-1]);
}

int main(int argc, char *argv[])
{

    string audioFilename, outputFilename, profileFilename;

    switch (argc)
    {
    case 2:
        audioFilename = argv[1];
        break;
    default:
        usage(argv[0]);
    }

    essentia::init();

    Pool pool;

    int melBands = 96;
    int numFrames = 187;
    Real sampleRate = 16000.0;
    int frameSize = 512;
    int stepSize = 256; // This is hopSize, however in JPawels code it is called stepSize, keeping for consistency

    AlgorithmFactory &factory = streaming::AlgorithmFactory::instance();

    Algorithm *audio = factory.create("MonoLoader",
                                      "filename", audioFilename,
                                      "sampleRate", sampleRate,
                                      "downmix", "left");

    Algorithm *fc = factory.create("FrameCutter",
                                   "frameSize", frameSize,
                                   "hopSize", stepSize,
                                   "startFromZero", true,
                                   "validFrameThresholdRatio", 1);

    Algorithm *extractor = factory.create("TensorflowInputMusiCNN");


    audio->output("audio") >> fc->input("signal");
    fc->output("frame") >> extractor->input("frame");
    // Send the extractor's output to a std::vector
    vector<vector<Real>> bandsVec;
    extractor->output("bands") >> bandsVec;
    

    cout << "Analyzing " << audioFilename << endl;

    Network n(audio);
    n.run();

    cout << "size(output): " << bandsVec.size() << "x" << bandsVec[0].size() << endl;
    cout << "head(output): " << endl;
    print2DVectorHead(bandsVec);

    n.clear();
    essentia::shutdown();

    return 0;
}
