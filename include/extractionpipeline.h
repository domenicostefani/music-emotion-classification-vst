#pragma once

#include <vector>
#include <string>

#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/scheduler/network.h>


namespace emosmi {
class MusicnnFeatureExtractor {
private:
    essentia::Pool pool;
    essentia::Real sampleRate = 16000.0;
    int frameSize = 512;
    int stepSize = 256; // This is hopSize, however in JPawels code it is called stepSize, keeping for consistency
public:
    MusicnnFeatureExtractor();
    ~MusicnnFeatureExtractor();
    void extractFromFile(const std::string& audioFilename, std::vector<std::vector<essentia::Real>> &bandsVec) const;
    std::vector<std::vector<essentia::Real>>& extractFromFile(const std::string& audioFilename) const;
};

}