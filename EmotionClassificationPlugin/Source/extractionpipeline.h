#pragma once

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/scheduler/network.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>

#include <cmath>
#include <string>
#include <vector>

namespace emosmi {
class MusicnnFeatureExtractor {
private:
    essentia::Pool pool;
    essentia::Real sampleRate = 16000.0;
    int frameSize = 512;
    int stepSize = 256;  // This is hopSize, however in JPawels code it is called stepSize, keeping for consistency
public:
    MusicnnFeatureExtractor();
    ~MusicnnFeatureExtractor();
    void extractFromFile(const std::string &audioFilename, std::vector<std::vector<essentia::Real>> &bandsVec) const;
    std::vector<std::vector<essentia::Real>> &extractFromFile(const std::string &audioFilename) const;
};

class RTisSilent {
private:
    essentia::Pool pool;
    essentia::Real sampleRate = 16000.0;
    int frameSize = 512;
    int hopSize = 256;        // This is hopSize, however in JPawels code it is called stepSize, keeping for consistency
    float threshold = -60.0;  // dBFS
    float _threshold;         // power -----> (10 ** (threshold in dB / 10)

    // Structure for ring buffer
    std::vector<float> ringBuffer;
    int head = 0;
    size_t appendedFromLastCompute = 0, storedSamples = 0;

public:
    RTisSilent() : ringBuffer(frameSize, 0.0f), threshold(-60), _threshold(pow(10.0f, threshold / 10.0f)) {}
    RTisSilent(size_t frameSize, size_t hopSize, float threshold_dB) : ringBuffer(frameSize, 0.0f), hopSize(hopSize), threshold(threshold_dB), _threshold(pow(10.0f, threshold / 10.0f)) {}
    ~RTisSilent() = default;
    void setThreshold(float threshold_dB) {
        threshold = threshold_dB;
        _threshold = pow(10.0f, threshold / 10.0f);
    }

    bool storeBlockAndCompute(const std::vector<float> &block, bool &isSilent, bool _verbose = false) {
        // Append properly to ring buffer
        int blockLength = block.size();
        int numSamplesToAppend = blockLength;

        if (blockLength > ringBuffer.size()) {  // If block is larger than ring buffer, we only append ringBuffer.size() samples
            printf("|WARNING|: blockLength > ringBuffer.size() (blockLength: %d, ringBuffer.size(): %zd)\n", blockLength, ringBuffer.size());
            numSamplesToAppend = ringBuffer.size();
        }

        int savehead = head;                            // TODO: remove
        for (int i = 0; i < numSamplesToAppend; ++i) {  // Append block to ring buffer
            ringBuffer[head] = block[i];
            head = (head + 1) % ringBuffer.size();
        }
        storedSamples += numSamplesToAppend;
        appendedFromLastCompute += numSamplesToAppend;

        bool computed = false;
        // Second condition allows to avoid precomputing when the ring buffer is not full
        if (appendedFromLastCompute >= hopSize && storedSamples >= ringBuffer.size()) {
            // Compute
            float power = instantPower(ringBuffer);

            if (_verbose) {
                std::cout << "_threshold: " << _threshold << std::endl;
                std::cout << "power: " << power << std::endl;
            }
            if (power < _threshold) {
                isSilent = true;
            }

            // Reset
            appendedFromLastCompute = 0;
            return true;
        }
        return false;
    }

    // returns the sum of the squared array = the energy of the array
    template <typename T>
    T energy(const std::vector<T> &array) {
        if (array.empty())
            throw std::logic_error("trying to calculate energy of empty array");

        return inner_product(array.begin(), array.end(), array.begin(), (T)0.0);
    }

    // returns the instantaneous power of an array
    template <typename T>
    T instantPower(const std::vector<T> &array) {
        return energy(array) / array.size();
    }
};

/**
 * @brief This class implements a filter for the silence detection algorithm.
 * It uses a circular buffer to keep track of the last frames and it computes
 * the percentage of frames in which the energy is below a certain threshold.
 * If the percentage is above a certain threshold, it sets the isSilent flag to true.
 * If a specific alphaDecay parameter is provided, the percentage is computed as a weighted average.
 * where the current frame f(n) has a weight of 1 and the previous i frames have a weight of alphaDecay^(n-i)
 *
 * alpha         = 0.5
 * filter_length = 4
 * filter_buffer = [0, 0, 1, 1]
 * weights       = [0.5**3, 0.5**2, 0.5**1, 1]
 * tot           = 0.5**3 + 0.5**2 + 0.5**1 + 1 = 1.875
 *
 * true_ratio    = (0.5**3*0 + 0.5**2 * 0 + 0.5**1 * 1 + 1 * 1)/tot = 1.5/1.875 = 0.8
 *
 *
 */
class SilenceFilter {
    size_t filterLength;             // Circular buffer/ filter window
    std::vector<bool> filterBuffer;  // Assume silence at the beginning
    size_t head = 0,                 // Head of the circular buffer
        actual_size = 0;             // Used until buffer gets to steady state (full)

    float trueToFalseTransitionRatio,  // Ratio required to  switch from true (silence) to false (activity)
        falseToTrueTransitionRatio,
        alphaDecay = 1.0;
    bool currentSilenceState;

    void push_back(bool value) {
        filterBuffer[head] = value;
        head = (head + 1) % filterLength;
        if (actual_size < filterLength)
            actual_size++;
    }

    float getTrueDecayedRatio(size_t _verbose = 0) {
        float count = 0.0f, tot = 0.0;
        std::string vecstr = "[ ";
        if (_verbose >= 2) {
            std::cout << "actual_size: " << actual_size << std::endl;
        }
        for (size_t i = 0; i < actual_size; ++i) {
            size_t idx = (i + head + filterLength - actual_size) % filterLength;
            if (_verbose >= 2)
                std::cout << "head: " << head << " i: " << i << "  idx: " << idx << " value: " << filterBuffer[idx] << std::endl;
            if (_verbose >= 1)
                vecstr = vecstr + (filterBuffer[idx] ? "1" : "0") + " ";
            if (filterBuffer[idx])
                count = count + pow(alphaDecay, actual_size - i - 1);
            if (_verbose >= 2)
                std::cout << "count: " << count << std::endl;

            tot = tot + pow(alphaDecay, actual_size - i - 1);
        }
        if (_verbose > 1) std::cout << vecstr << "] -> returning truecount = " << (float)count / tot << std::endl;
        return (float)count / tot;
    }

    bool computeFilter(size_t _verbose = 0) {
        float trueCount = this->getTrueDecayedRatio();
        if (_verbose >= 1) std::cout << "trueCount:" << trueCount << " currentSilenceState:" << currentSilenceState;
        if (currentSilenceState) {
            if ((1 - trueCount) > trueToFalseTransitionRatio)
                currentSilenceState = false;
        } else {
            if (trueCount > falseToTrueTransitionRatio)
                currentSilenceState = true;
        }
        if (_verbose >= 1) std::cout << "   -> res: " << currentSilenceState << std::endl;
        return currentSilenceState;
    }

public:
    SilenceFilter(size_t filterLength, float trueToFalseTransitionRatio, float alphaDecay = 0.1) : filterLength(filterLength), trueToFalseTransitionRatio(trueToFalseTransitionRatio), alphaDecay(alphaDecay) {
        filterBuffer.resize(filterLength);
        falseToTrueTransitionRatio = 1 - trueToFalseTransitionRatio;
        currentSilenceState = true;
    }
    ~SilenceFilter() = default;

    bool filter(bool valueToAppend) {
        this->push_back(valueToAppend);
        return this->computeFilter();
    }
};

}  // namespace emosmi
