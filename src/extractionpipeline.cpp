/*
 * Extraction pipeline for the instrument emotion recognition project/emotionally aware Smart Musical Instruments project
 * Essentia: http://essentia.upf.edu. 
 * Essentia Paper: Bogdanov, D., Wack N., Gómez E., Gulati S., Herrera P., Mayor O., et al. (2013). ESSENTIA: an Audio Analysis Library for Music Information Retrieval. International Society for Music Information Retrieval Conference (ISMIR'13). 493-498.
 * 
 * Domenico Stefani 2023
 */

#include "extractionpipeline.h"

using namespace essentia;
using namespace essentia::streaming;
using namespace essentia::scheduler;

void extractFromFile(std::string audioFilename, std::vector<std::vector<Real>> &bandsVec)
{

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
    extractor->output("bands") >> bandsVec;
    

    Network n(audio);
    n.run();

    n.clear();
    essentia::shutdown();
}
