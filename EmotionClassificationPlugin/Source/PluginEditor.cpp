/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

void EmotionClassificationPluginAudioProcessorEditor::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Folder where to save results...",
                                                    File::getSpecialLocation (File::userHomeDirectory));                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File{}) {
            audioProcessor.setSaveFolder(file);
            selectSaveFolderButton.setButtonText("Select save folder\n(current: \""+file.getFullPathName().toStdString()+"\")");
        }
    });
}

EmotionClassificationPluginAudioProcessorEditor::EmotionClassificationPluginAudioProcessorEditor (EmotionClassificationPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, true);
    setSize (500, 350);

 

    // myChooser = std::make_unique<FileChooser> ("Please select the folder to save files to...",
    //                                         File::getSpecialLocation (File::userHomeDirectory));

    // auto folderChooserFlags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectDirectories;

    // myChooser->launchAsync (folderChooserFlags, [this] (const FileChooser& chooser)
    // {
    //     File selFolder (chooser.getResult());
    //     std::cout << "Selected folder: " + selFolder.getFullPathName();
    // });

    // addAndMakeVisible(myChooser);

    selectSaveFolderButton.setButtonText("Select save folder\nDefault: \""+audioProcessor.getSaveFolder().getFullPathName().toStdString()+"\"");
    selectSaveFolderButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(selectSaveFolderButton);




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
    status.setJustificationType(Justification::left);
    // status.setFont(Font(16.0f, Font::bold));

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

    // Rectangle<int> area = getLocalBounds();
    Rectangle<int> area = getLocalBounds();
    auto bottomStrip = area.removeFromBottom(18);
    Rectangle<int> usable = area.reduced(20);


    // g.setColour (juce::Colours::white);
    // g.setFont (17.0f);
    // g.drawFittedText ("Emotionally Aware Smart Musical Instruments\nClassification Plugin", usable.removeFromTop(usable.getHeight() * 0.6), juce::Justification::centred, 1);


    g.setFont (13.0f);
    g.setColour (juce::Colours::lightgrey);
    std::string date(__DATE__);
    std::string time(__TIME__);
    g.drawFittedText ("Domenico Stefani, Luca Turchet, Johan Pauwels", bottomStrip.removeFromLeft(bottomStrip.getWidth() * 0.5), juce::Justification::left, 1);
    g.drawFittedText ("Compiled on: "+date+" at "+time, bottomStrip, juce::Justification::right, 1);


    selectSaveFolderButton.setBounds(usable.removeFromTop(usable.getHeight() * 0.3).reduced(10));
    recButton.setBounds(usable.removeFromTop(usable.getHeight() * 0.3).reduced(15));

    g.setFont(Font(17.0f, Font::bold));
    g.drawFittedText ("Status:", usable.removeFromTop(18), juce::Justification::left, 1);
    status.setBounds(usable);
}

void EmotionClassificationPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
