/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
const String OFF_TEXT = "Press to start recording";
const String ON_TEXT = "Recording... Press to classify";
const String CLASSIFYING_TEXT = "Classifying...";

typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

class EmotionClassificationPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EmotionClassificationPluginAudioProcessorEditor (EmotionClassificationPluginAudioProcessor&);
    ~EmotionClassificationPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EmotionClassificationPluginAudioProcessor& audioProcessor;

    TextButton recButton; // Button tied to the recording state parameter
    std::unique_ptr<ButtonAttachment> recButtonAttachment;  // Attachment between the button and the recording state parameter

    // Toggle the color and text of the rec button (with the attacment, this matches the state of the recording state parameter)
    void toggle() {
        // std::cout << "Toggle: " << recButton.getState() << std::endl;

        if (recButton.getToggleState() ) {
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

    Label status;

    typedef std::function<void (void)> TimerEventCallback;
    class PollingTimer : public Timer
    {
    public:
        PollingTimer(TimerEventCallback cb) { this->cb = cb; }
        void startPolling() { Timer::startTimer(this->interval); }
        void timerCallback() override { cb(); }
    private:
        TimerEventCallback cb;
        int interval = 3;
    };

    void pollingRoutine()
    {
        status.setText(audioProcessor.extractorState, dontSendNotification);
    }

    PollingTimer pollingTimer{[this]{pollingRoutine();}};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmotionClassificationPluginAudioProcessorEditor)
};

