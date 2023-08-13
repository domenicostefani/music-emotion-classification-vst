#pragma once

#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>
#include <essentia/scheduler/network.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/ringbufferinput.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <juce_audio_processors/juce_audio_processors.h>

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
protected:
    int frameSize = 512;
    int hopSize = 256;        // This is hopSize, however in JPawels code it is called stepSize, keeping for consistency
    float threshold = -60.0;  // dBFS
    float _threshold;         // power -----> (10 ** (threshold in dB / 10)
    std::vector<float> tmpBlock;
    std::vector<bool> tmpRes;

    bool isSilent = false;

    // Structure for ring buffer
    std::vector<float> ringBuffer;
    int head = 0;
    size_t appendedFromLastCompute = 0, storedSamples = 0;

public:
    // RTisSilent() : ringBuffer(frameSize, 0.0f), threshold(-60), _threshold(pow(10.0f, threshold / 10.0f)) {}
    RTisSilent(size_t frameSize, size_t hopSize, float threshold_dB) : ringBuffer(frameSize, 0.0f), hopSize(hopSize), threshold(threshold_dB), _threshold(pow(10.0f, threshold / 10.0f)), tmpBlock(hopSize), tmpRes(100) {}
    ~RTisSilent() = default;

    void setThreshold(float threshold_dB) {
        threshold = threshold_dB;
        _threshold = pow(10.0f, threshold / 10.0f);
    }

    bool storeBlockAndCompute(const float block[], size_t blockLength, bool &isSilent, bool _verbose = false) {
        // Append properly to ring buffer
        int numSamplesToAppend = blockLength;

        if (blockLength > ringBuffer.size()) {  // If block is larger than ring buffer, we only append ringBuffer.size() samples
            printf("|WARNING|: blockLength > ringBuffer.size() (blockLength: %zd, ringBuffer.size(): %zd)\n", blockLength, ringBuffer.size());
            numSamplesToAppend = ringBuffer.size();
        }

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

            isSilent = power < _threshold;
            this->isSilent = isSilent;

            // Reset
            appendedFromLastCompute = 0;
            return true;
        }
        return false;
    }

    bool storeBlockAndCompute(const std::vector<float> &block, bool &isSilent, bool _verbose = false) {
        return storeBlockAndCompute(block.data(), block.size(), isSilent, _verbose);
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

    void processBlock(juce::AudioBuffer<float> &buffer, size_t channel = 0, bool _verbose = false) {
        // buffer.addFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
        if (buffer.getNumSamples() > hopSize) {
            // If there are more samples than a single hopsize we compute the isSilent flag for each hopsize
            // buffer.getNumSamples() / hopSize
            // Furthermore, we store in this->isSilent the majority vote
            size_t numHops = buffer.getNumSamples() / hopSize;
            for (size_t i = 0; i < numHops; ++i) {
                bool res;
                storeBlockAndCompute(buffer.getReadPointer(channel) + i * hopSize, hopSize, res, _verbose);
                this->tmpRes[res] = res;
            }
            // Majority vote
            size_t numSilent = std::count(this->tmpRes.begin(), this->tmpRes.begin() + numHops, true);
            size_t numNotSilent = std::count(this->tmpRes.begin(), this->tmpRes.begin() + numHops, false);
            this->isSilent = (numSilent > numNotSilent);
        } else {
            // If there are less samples than a single hopsize we store the whole block. The isSilent flag is eventually set by storeBlockAndCompute
            storeBlockAndCompute(buffer.getReadPointer(channel), buffer.getNumSamples(), this->isSilent, _verbose);
        }
    }

    bool computeIsSilent() {
        return isSilent;
    }

    // Getters
    float getFrameSize() const {
        return frameSize;
    }
    size_t getHopSize() const {
        return hopSize;
    }

    float getThreshold() const {
        return threshold;
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

    void reset() {
        head = 0;
        actual_size = 0;
        currentSilenceState = true;
    }

    bool filter(bool valueToAppend) {
        this->push_back(valueToAppend);
        return this->computeFilter();
    }
};

class FilteredRTisSilent : public RTisSilent {
    SilenceFilter silenceFilter;

public:
    FilteredRTisSilent(size_t frameSize, size_t hopSize, float threshold_dB, size_t filterLength, float trueToFalseTransitionRatio, float alphaDecay = 0.1) : RTisSilent(frameSize, hopSize, threshold_dB), silenceFilter(filterLength, trueToFalseTransitionRatio, alphaDecay) {
    }

    void processBlock(juce::AudioBuffer<float> &buffer, size_t channel=0, bool _verbose = false) {
        // buffer.addFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
        if (buffer.getNumSamples() > hopSize) {
            // If there are more samples than a single hopsize we compute the isSilent flag for each hopsize
            // buffer.getNumSamples() / hopSize
            // Furthermore, we store in this->isSilent the majority vote
            size_t numHops = buffer.getNumSamples() / hopSize;
            if (_verbose) std::cout << "Repeting Silence computation " << numHops << " times" << std::endl;
            for (size_t i = 0; i < numHops; ++i) {
                bool res;
                storeBlockAndCompute(buffer.getReadPointer(channel) + i * hopSize, hopSize, res, _verbose);
                if (_verbose) std::cout << "res[" << i << "] = " << res;
                res = silenceFilter.filter(res);
                if (_verbose) std::cout << " filtered = " << res << std::endl;
                this->tmpRes[i] = res;
            }
            // Majority vote
            size_t numSilent = std::count(this->tmpRes.begin(), this->tmpRes.begin() + numHops, true);
            size_t numNotSilent = std::count(this->tmpRes.begin(), this->tmpRes.begin() + numHops, false);
            this->isSilent = (numSilent > numNotSilent);
            if (_verbose) std::cout << "Majority vote: " << isSilent << std::endl;
        } else {
            // If there are less samples than a single hopsize we store the whole block. The isSilent flag is eventually set by storeBlockAndCompute

            if (_verbose) std::cout << "Single silence computation for " << buffer.getNumSamples() << " samples" << std::endl;
            bool res;
            if (storeBlockAndCompute(buffer.getReadPointer(channel), buffer.getNumSamples(), res, _verbose)) {
                if (_verbose) std::cout << " -> Computed... res:" << res;
                this->isSilent = silenceFilter.filter(res);
                if (_verbose) std::cout << " filtered:" << this->isSilent << std::endl;
            } else if (_verbose)
                std::cout << " -> Not enough samples for computation" << std::endl;
        }
    }

    void resetFilter() {
        silenceFilter.reset();
    }
};

/**
 * @brief Offline version of FilteredRTisSilentm which loads an audio recording from file
 *
 */
class FilteredIsSilent {
    FilteredRTisSilent rtSilenceDetector;
    float sampleRate;

    std::vector<float> essentiaLoad(std::string audioFilename, float sampleRate) {
        std::vector<essentia::Real> signalVec;
        essentia::streaming::AlgorithmFactory &factory = essentia::streaming::AlgorithmFactory::instance();

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

public:
    FilteredIsSilent(float sampleRate, size_t frameSize, size_t hopSize, float threshold_dB, size_t filterLength, float trueToFalseTransitionRatio, float alphaDecay = 0.1) : rtSilenceDetector(frameSize, hopSize, threshold_dB, filterLength, trueToFalseTransitionRatio, alphaDecay), sampleRate(sampleRate) {}

    void resetFilter() {
        rtSilenceDetector.resetFilter();
    }

    void setThreshold(float threshold_dB) {
        rtSilenceDetector.setThreshold(threshold_dB);
    }

    std::vector<bool> computeFromFile(std::string audioFilename) {
        rtSilenceDetector.resetFilter();
        std::vector<bool> res;
        std::vector<float> signal = essentiaLoad(audioFilename, this->sampleRate);
        for (size_t i = 0; i < signal.size(); i += rtSilenceDetector.getHopSize()) {
            size_t numSamples = std::min(rtSilenceDetector.getHopSize(), signal.size() - i);
            juce::AudioBuffer<float> buffer(1, numSamples);
            buffer.copyFrom(0, 0, signal.data() + i, numSamples);
            rtSilenceDetector.processBlock(buffer);
            res.push_back(rtSilenceDetector.computeIsSilent());
        }
        return res;
    }
};

class PerformanceStartStop {
    FilteredIsSilent fis;

public:
    PerformanceStartStop(float sampleRate, size_t frameSize, size_t hopSize, float threshold_dB, size_t filterLength, float trueToFalseTransitionRatio, float alphaDecay = 0.1) : fis(sampleRate, frameSize, hopSize, threshold_dB, filterLength, trueToFalseTransitionRatio, alphaDecay) {}

    std::pair<size_t, size_t> computeFromFile(std::string audioFilename, float threshold_dB, bool _verbose = false) {
        fis.setThreshold(threshold_dB);
        fis.resetFilter();
        std::vector<bool> silenceVec = fis.computeFromFile(audioFilename);

        // Set the first few values to true

        size_t numToSetSilentAtStart = 4;
        if (silenceVec[numToSetSilentAtStart] == true)
            for (size_t i = 0; i < numToSetSilentAtStart; ++i)
                silenceVec[i] = true;  // TODO: Fix this bug with the silence detector where sometimes the first is not silent.

        if (_verbose)
            for (auto e : silenceVec) std::cout << (e ? '1' : '0');
        std::cout << std::endl;

        // Take the first non silent block (false) as start index and the last non silent block as stop index
        size_t startIdx = std::find(silenceVec.begin(), silenceVec.end(), false) - silenceVec.begin();
        size_t stopIdx = std::find(silenceVec.rbegin(), silenceVec.rend(), false) - silenceVec.rbegin();
        stopIdx = silenceVec.size() - stopIdx;

        if (startIdx >= stopIdx) {
            if (_verbose) std::cout << "Silent recording" << std::endl;
            return std::make_pair(0, 0);
        }

        if (_verbose) std::cout << "startIdx = " << startIdx << " | stopIdx = " << stopIdx << std::endl;

        return std::make_pair(startIdx, stopIdx);
    }
};

}  // namespace emosmi
