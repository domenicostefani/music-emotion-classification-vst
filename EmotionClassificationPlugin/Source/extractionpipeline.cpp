/*
 * Extraction pipeline for the instrument emotion recognition project/emotionally aware Smart Musical Instruments
 * project Essentia: http://essentia.upf.edu. [1]
 *
 * [1] Bogdanov, D., Wack N., Gómez E., Gulati S., Herrera P., Mayor O., et al. (2013). ESSENTIA: an Audio Analysis
 * Library for Music Information Retrieval. International Society for Music Information Retrieval Conference (ISMIR'13).
 * 493-498. [2] Turchet, L., & Pauwels, J. (2021). Music emotion recognition: intention of composers-performers versus
 * perception of musicians, non-musicians, and listening machines. IEEE/ACM Transactions on Audio, Speech, and Language
 * Processing, 30, 305-316. [3] Pons, J., & Serra, X. (2019). musicnn: Pre-trained convolutional neural networks for
 * music audio tagging. arXiv preprint arXiv:1909.06654.
 *
 * Domenico Stefani 2023
 */

#include "extractionpipeline.h"

using namespace essentia;
using namespace essentia::streaming;
using namespace essentia::scheduler;

namespace emosmi {

MusicnnFeatureExtractor::MusicnnFeatureExtractor() {
    essentia::init();
}
MusicnnFeatureExtractor::~MusicnnFeatureExtractor() {
    essentia::shutdown();
}

void MusicnnFeatureExtractor::extractFromFile(const std::string &audioFilename, std::vector<std::vector<essentia::Real>> &bandsVec) const {
    // Output is a matrix of 96 features x 62.5 frames for every second of audio (265 hop size at 16kHz)
    // To pick 3 seconds of audio (Required for the algorithm in [2]), we need 3*62.5 = 187.5 frames

    AlgorithmFactory &factory = streaming::AlgorithmFactory::instance();

    // This loads the audio file from disk, downsamples it to 16kHz mono and outputs it
    Algorithm *audio =
        factory.create("MonoLoader", "filename", audioFilename, "sampleRate", sampleRate, "downmix", "left");

    // This cuts the audio into frames of frameSize samples, with stepSize samples overlapping between consecutive
    // frames
    Algorithm *fc = factory.create("FrameCutter", "frameSize", frameSize, "hopSize", stepSize, "startFromZero", true,
                                   "validFrameThresholdRatio", 1);

    // This is a compound feature extractor developed for MusiCNN [3]
    Algorithm *extractor = factory.create("TensorflowInputMusiCNN");

    // ---
    // Now we connect the algorithms and output to a std::vector<std::vector<essentia::Real>>
    // ---
    audio->output("audio") >> fc->input("signal");
    // std::vector<essentia::Real> test; //TODO: remove
    // audio->output("audio") >> test; //TODO: remove
    
    fc->output("frame") >> extractor->input("frame");
    extractor->output("bands") >> bandsVec;  // Send the extractor's output to a std::vector
    Network n(audio);

    n.run();
    n.clear();

    // std::cout << "\n\n--------------------------------------------------" << std::endl;
    // std::cout << "Test vector length: " << test.size() << std::endl;
    // std::cout << "Theoretical minutes:seconds.milliseconds = " << test.size() / 16000 / 60 << ":"
    //           << (test.size() / 16000) % 60 << "." << (test.size() % 16000) / 16 << std::endl;
    // std::cout << "--------------------------------------------------\n\n\n" << std::endl;
}

}  // namespace emosmi