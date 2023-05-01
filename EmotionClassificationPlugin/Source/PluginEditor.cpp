/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EmotionClassificationPluginAudioProcessorEditor::EmotionClassificationPluginAudioProcessorEditor (EmotionClassificationPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 250);

    addAndMakeVisible(recButton);
    recButton.setButtonText("Record");
    recButton.setColour(TextButton::buttonColourId, Colours::red);
    recButton.setColour(TextButton::buttonOnColourId, Colours::green);
    recButton.setColour(TextButton::textColourOnId, Colours::white);
    recButton.setColour(TextButton::textColourOffId, Colours::white);

    // recButton.onClick = [this] { audioProcessor.record(); };
    recButton.onClick = [this] { toggle(); };
    recButton.setClickingTogglesState(true);

    recButtonAttachment.reset(new ButtonAttachment(audioProcessor.valueTreeState, audioProcessor.RECSTATE_ID, recButton));

    addAndMakeVisible(status);
    status.setColour(Label::textColourId, Colours::white);
    status.setJustificationType(Justification::centred);
    status.setFont(Font(16.0f, Font::bold));

    pollingTimer.startPolling();


}



EmotionClassificationPluginAudioProcessorEditor::~EmotionClassificationPluginAudioProcessorEditor()
{
}

//==============================================================================
void EmotionClassificationPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
    // g.setColour (juce::Colours::black);



    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    Rectangle<int> area = getLocalBounds();
    Rectangle<int> usable = area.reduced(10);
    g.setColour (juce::Colours::white);

    g.setFont (17.0f);
    std::string date(__DATE__);
    std::string time(__TIME__);
    g.drawFittedText ("Emotionally Aware Smart Musical Instruments\nClassification Plugin\n---\n Domenico Stefani, Luca Turchet, Johan Pauwels\nCompilation Date: "+date+"-"+time, usable.removeFromTop(usable.getHeight() * 0.6), juce::Justification::centred, 1);

    recButton.setBounds(usable.removeFromTop(usable.getHeight() * 0.3));

    status.setBounds(usable);
}

void EmotionClassificationPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
