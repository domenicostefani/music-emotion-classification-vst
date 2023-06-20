#pragma once

#include <JuceHeader.h>

#include "AudioThumbnail.h"
#include "PluginProcessor.h"
#include "led.h"
#include "levelMeter.h"

#define METER_USE_PEAK
// #define METER_USE_RMS

const String OFF_TEXT = "Press to start recording";
const String ON_TEXT = "Recording... Press to classify";
const String CLASSIFYING_TEXT = "Classifying...";

typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

class ECEditor : public juce::AudioProcessorEditor {
public:
    ECEditor(ECProcessor&);
    ~ECEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ECProcessor& audioProcessor;

    TextButton recButton;                                   // Button tied to the recording state parameter
    std::unique_ptr<ButtonAttachment> recButtonAttachment;  // Attachment between the button and the recording state parameter

    // Toggle the color and text of the rec button (with the attacment, this matches the state of the recording state parameter)
    void toggle() {
        // std::cout << "Toggle: " << recButton.getState() << std::endl;

        if (audioProcessor.enableRec.load()) {
            if (recButton.getToggleState()) {
                recButton.setButtonText(ON_TEXT);
            } else {
                // recButton.setButtonText(CLASSIFYING_TEXT);
                // recButton.setColour(TextButton::buttonColourId, Colours::orange);
                // // Disable rec button for 3 seconds
                // recButton.setEnabled(false);
                // Timer::callAfterDelay(3000, [this] { recButton.setEnabled(true); recButton.setButtonText(OFF_TEXT); recButton.setColour(TextButton::buttonColourId, Colours::red); });

                {
                    recButton.setEnabled(true);
                    recButton.setButtonText(OFF_TEXT);
                    recButton.setColour(TextButton::buttonColourId, Colours::red);
                }
            }
        }
    }

    Label status;

    typedef std::function<void(void)> TimerEventCallback;
    class PollingTimer : public Timer {
    public:
        PollingTimer(TimerEventCallback cb) { this->cb = cb; }
        void startPolling() { Timer::startTimer(this->interval); }
        void timerCallback() override { cb(); }

    private:
        TimerEventCallback cb;
        int interval = 3;
    };

    void pollingRoutine() {
        status.setText(audioProcessor.extractorState, dontSendNotification);

        if (audioProcessor.recordingStopped.exchange(false)) {
            juce::File afile(audioProcessor.audioFilename);
            waveformDisplayComponent.setFileSource(afile);
        }

        if (audioProcessor.labelsWritten.exchange(false)) {
            std::vector<juce::Colour> labelColors;
            for (auto& label : audioProcessor.outputLabelsInt) {
                labelColors.push_back(emotionLabelColorMap[label]);
            }
            waveformDisplayComponent.setSectionLabels(audioProcessor.outputLabels, labelColors);
        }
    }

    PollingTimer pollingTimer{[this] { pollingRoutine(); }};

    std::map<size_t, juce::Colour> emotionLabelColorMap = {
        {0, juce::Colours::orangered},
        {1, juce::Colours::lightgreen},
        {2, juce::Colours::orange},
        {3, juce::Colours::lightblue},
        {4, juce::Colours::white}};  // White for ambivalent Emotion

    std::unique_ptr<juce::FileChooser> savedirChooser,modelChooser;
    TextButton selectSaveFolderButton, selectModelButton;
    void openButtonClicked();
    void modelButtonClicked();
    void recordStateChanged();

    // Waveform display
    AudioThumbnailComponent waveformDisplayComponent{"Load a model and press the Record button to start recording."};

    // Metering
    Gui::LevelMeter meter;

    // Gain Slider
    Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainSliderAttachment;

    // Silence Detector
    Gui::Led silenceLed;

    Slider silenceThSlider;
    std::unique_ptr<SliderAttachment> silenceThSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ECEditor)
};
