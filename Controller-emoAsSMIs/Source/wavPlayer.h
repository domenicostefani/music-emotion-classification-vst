#pragma once

#include <JuceHeader.h>

class WavPlayer : public juce::ChangeListener {
public:
    WavPlayer() : state(Stopped) {
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
    }

    ~WavPlayer() = default;

    void play(const char* filename) {
        std::cout << "I should now play " << filename << std::endl;  // TODO: remove
        std::cout << "Creating file" << std::endl;                   // TODO: remove
        juce::File file(filename);
        std::cout << "Done" << std::endl
                  << std::flush;  // TODO: remove

        if (!file.existsAsFile()) {
            std::cout << "File does not exist" << std::endl
                      << std::flush;  // TODO: remove
            return;
        }
        std::cout << "File exists" << std::endl
                  << std::flush;  // TODO: remove

        if (file != juce::File{}) {
            std::cout << "Creating reader" << std::endl;  // TODO: remove
            auto* reader = formatManager.createReaderFor(file);
            std::cout << "Done" << std::endl;  // TODO: remove

            if (reader != nullptr) {
                std::cout << "Reader is not null" << std::endl;  // TODO: remove
                std::cout << "Creating newSource" << std::endl;  // TODO: remove
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                std::cout << "Done\nSetting transportSource" << std::endl;  // TODO: remove
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                std::cout << "Done\nResetting readerSource" << std::endl;  // TODO: remove
                // playButton.setEnabled (true);
                readerSource.reset(newSource.release());
            } else {
                std::cout << "Reader is null!!!" << std::endl;  // TODO: remove
            }
        }

        std::cout << "File opened, now playing..." << std::endl;  // TODO: remove

        changeState(Starting);
    }

    void stop() {
        changeState(Stopping);
    }

    bool isPlaying() {
        return state == Playing;
    }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    enum TransportState {
        Stopped,
        Starting,
        Playing,
        Stopping
    } state;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &transportSource) {
            if (transportSource.isPlaying())
                changeState(Playing);
            else
                changeState(Stopped);
        }
    }

    void changeState(TransportState newState) {
        if (state != newState) {
            state = newState;

            switch (state) {
                case Stopped:
                    std::cout << "Changing to Stopped" << std::endl;  // TODO: remove
                    // stopButton.setEnabled (false);
                    // playButton.setEnabled (true);
                    transportSource.setPosition(0.0);
                    break;

                case Starting:
                    std::cout << "Changing to Starting" << std::endl;  // TODO: remove
                    // playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    std::cout << "Changing to Playing" << std::endl;  // TODO: remove
                    // stopButton.setEnabled (true);
                    break;

                case Stopping:
                    std::cout << "Changing to Stopping" << std::endl;  // TODO: remove
                    transportSource.stop();
                    break;
            }
        }
    }

public:
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        auto ainfo = juce::AudioSourceChannelInfo(buffer);

        if (readerSource.get() == nullptr)
        {
            ainfo.clearActiveBufferRegion();
            return;
        }
        transportSource.getNextAudioBlock(ainfo);
    }

    void prepareToPlay(double sampleRate, int samplesPerBlockExpected) {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() {
        transportSource.releaseResources();
    }
};
