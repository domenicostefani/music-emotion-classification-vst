#pragma once

#include <JuceHeader.h>

class WavPlayer : public juce::ChangeListener {
public:
    WavPlayer() : state(Stopped) {
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
    }

    ~WavPlayer() = default;

    void play(std::string track) {
        this->play(track.c_str());
    }

    void clearPlaylist() {
        trackList.clear();
    }

    void play(const char* filename) {
        std::cout << "I should now play " << filename << std::endl;  // TODO: remove
        juce::File file(filename);
        if (!file.existsAsFile()) {
            std::cout << "File does not exist" << std::endl
                      << std::flush;  // TODO: remove
            return;
        }
        DBG("File exists");

        if (file != juce::File{}) {
            DBG("Creating reader");
            auto* reader = formatManager.createReaderFor(file);
            DBG("Done");

            if (reader != nullptr) {
                DBG("Reader is not null");
                DBG("Creating newSource");
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                DBG("Done\nSetting transportSource");
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                nowPlayingTrack = std::string(filename);
                DBG("Done\nResetting readerSource");
                // playButton.setEnabled (true);
                readerSource.reset(newSource.release());
            } else {
                DBG("Reader is null!!!");
            }
        }

        DBG("File opened, now playing...");

        changeState(Starting);
    }

    void play(std::vector<std::string> tracks) {
        std::reverse(tracks.begin(), tracks.end());
        trackList = tracks;
        playNextTrack();
    }

    void playNextTrack() {
        if (trackList.empty()) {
            DBG("No more tracks to play");
            return;
        }
        auto track = trackList.back();
        trackList.pop_back();
        DBG("Playing track " << track);
        play(track.c_str());
    }

    void stop() {
        trackList.clear();
        nowPlayingTrack = "";
        changeState(Stopping);
    }

    bool isPlaying() {
        return state == Playing;
    }

    std::string getNowPlaying() {
        return nowPlayingTrack;
    }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    std::vector<std::string> trackList;
    std::string nowPlayingTrack {""};

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
                    DBG("Changing to Stopped");
                    // stopButton.setEnabled (false);
                    // playButton.setEnabled (true);
                    transportSource.setPosition(0.0);
                    nowPlayingTrack = "";
                    playNextTrack();
                    break;

                case Starting:
                    DBG("Changing to Starting");
                    // playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    DBG("Changing to Playing");
                    // stopButton.setEnabled (true);
                    break;

                case Stopping:
                    DBG("Changing to Stopping");
                    transportSource.stop();
                    break;
            }
        }
    }

public:
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
        auto ainfo = juce::AudioSourceChannelInfo(buffer);

        if (readerSource.get() == nullptr) {
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
