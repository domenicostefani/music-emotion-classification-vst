/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "OSCreceiver.h"
#include "led.h"
#include "levelMeter.h"

class MainComponent : public Component, public Button::Listener {
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint(Graphics &) override;
    void resized() override;

    TextButton connectBtn;
    // TextEditor oscIp, oscPort1, oscPort2, filename;

    const int OSC_SEND_PORT_1_TOSERVER = 6042,
              OSC_SEND_PORT_2_TOPLUGIN = 8042,
              OSC_SEND_PORT_3_TOSUSHI = 24024;

    Label instrumentLabel;

    TextEditor boardIpText, controllerIpText;
    Label boardIpLabel, controllerIpLabel;

    const int RX_PORT_SERVER = 7042,
              RX_PORT_PLUGIN = 9042;

    cOSC::Receiver oscReceiverFromServer, oscReceiverFromPlugin;
    void oscMessageReceived(const juce::OSCMessage &message);
    void setButtonsEnabled(bool enable);
    void setIPFieldsEnabled(bool enable);
    bool waitForHandshakeWithServer = false, waitForHandshakeWithPlugin = false;

    Slider gainSlider, silenceSlider;
    Label gainLabel, silenceLabel, silenceThresholdLabel;
    Gui::LevelMeter meter;
    float getOscInLevel() const;
    Gui::Led silenceLed;
    bool getOscIsSilent() const;

    TextButton startBtn, stopBtn;
    Label statusLabel;
    void showStatus(int status);  // 0 = disconnected, 1 = idle, 2 = recording, 3 = classifying

    class ReturnedEmotion {
        bool aggressive = false,
             relaxed = false,
             happy = false,
             sad = false;

    public:
        void reset() {
            aggressive = false;
            relaxed = false;
            happy = false;
            sad = false;
        }

        ReturnedEmotion(int oneHotCode) {
            set(oneHotCode);
        }

        void set(int oneHotCode) {
            // 1000 = aggressive
            // 0100 = relaxed
            // 0010 = happy
            // 0001 = sad
            // Combinations allowed
            if (oneHotCode & 0b1000) aggressive = true;
            if (oneHotCode & 0b0100) relaxed = true;
            if (oneHotCode & 0b0010) happy = true;
            if (oneHotCode & 0b0001) sad = true;
        }

        ReturnedEmotion(bool aggressive = false,
                        bool relaxed = false,
                        bool happy = false,
                        bool sad = false) {
            this->aggressive = aggressive;
            this->relaxed = relaxed;
            this->happy = happy;
            this->sad = sad;
        }

        void getAll(bool &agg, bool &rel, bool &hap, bool &sad) {
            agg = this->aggressive;
            rel = this->relaxed;
            hap = this->happy;
            sad = this->sad;
        }

        bool getAggressive() const { return aggressive; }
        bool getRelaxed() const { return relaxed; }
        bool getHappy() const { return happy; }
        bool getSad() const { return sad; }
    } returnedEmotion;
    Gui::Led emoAggressiveLed, emoRelaxedLed, emoHappyLed, emoSadLed;
    Label emoAggressiveLabel, emoRelaxedLabel, emoHappyLabel, emoSadLabel;
    void getOscEmotion(bool &agg, bool &rel, bool &hap, bool &sad);
    Label resultErrorLabel;

    TextButton playExcerptBtn;

    Rectangle<int> origEmoArea, resultStatusArea, wholearea, basearea;

    TextButton renameBtn, nextFilenameBtn, prevFilenameBtn;
    TextEditor filename;

    Label fileCounterLabel;

    void buttonClicked(Button *button);

    void showConnectionErrorMessage(const juce::String &messageText) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Connection error",
                                               messageText,
                                               "OK");
    }

    typedef std::function<void(void)> TimerEventCallback;
    class PollingTimer : public Timer {
    public:
        PollingTimer(TimerEventCallback cb) { this->cb = cb; }
        void startPolling(int msInterval = 24) { Timer::startTimer(msInterval); }
        void stopPolling() { Timer::stopTimer(); }
        void timerCallback() override { cb(); }

    private:
        TimerEventCallback cb;
    };

    void sendPluginHandshakePollingRoutine() {
        oscSender2_toPlugin.send("/handshake", juce::String(controllerIpText.getText().toStdString()));  // Send handshake
    }

    PollingTimer pluginHandshakePoller{[this] { sendPluginHandshakePollingRoutine(); }};

private:
    //==============================================================================
    juce::OSCSender recstateSender, renamingSender, oscSender1_toserver, oscSender2_toPlugin;
    bool isConnectionMade = false;
    std::vector<std::string> read_names_csv(std::string filename);

    void switchToNextFilename();
    void switchToPrevFilename();
    int filenameCounter = -1;
    std::vector<std::string> readnames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
