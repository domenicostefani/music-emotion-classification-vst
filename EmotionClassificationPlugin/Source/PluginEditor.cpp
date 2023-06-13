#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================

void ECEditor::openButtonClicked() {
    savedirChooser = std::make_unique<juce::FileChooser>("Select a Folder where to save results...",
                                                  File::getSpecialLocation(File::userHomeDirectory));  // [7]
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectDirectories;

    savedirChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File{}) {
            audioProcessor.setSaveFolder(file);
            selectSaveFolderButton.setButtonText("Select save folder\n(current: \"" + file.getFullPathName().toStdString() + "\")");
        }
    });
}

ECEditor::ECEditor(ECProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      silenceLed([&]() { return !audioProcessor.isSilent.load(); }),
#ifdef METER_USE_PEAK
      meter([&]() { return audioProcessor.getPeakValue(); })
#endif
#ifdef METER_USE_RMS
      meter([&]() { return audioProcessor.getRMSValue(); })
#endif
{
    setResizable(true, true);
    setSize(970, 350);
    setResizeLimits(970, 350, 1920, 1080);		

    selectSaveFolderButton.setButtonText("Select save folder\nDefault: \"" + audioProcessor.getSaveFolder().getFullPathName().toStdString() + "\"");
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
    status.setJustificationType(Justification::topLeft);
    // status.setFont(Font(16.0f, Font::bold));

    addAndMakeVisible(waveformDisplayComponent);

    // Metering
    addAndMakeVisible(meter);
    // Input Gain
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    gainSliderAttachment.reset(new SliderAttachment(audioProcessor.valueTreeState, audioProcessor.GAIN_ID, gainSlider));

    // Silence Detection
    addAndMakeVisible(silenceLed);
    addAndMakeVisible(silenceThSlider);
    silenceThSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    silenceThSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    silenceThSlider.setTextValueSuffix (" dB"); 
    silenceThSliderAttachment.reset(new SliderAttachment(audioProcessor.valueTreeState, audioProcessor.SILENCE_THRESH_ID, silenceThSlider));

    pollingTimer.startPolling();
}

ECEditor::~ECEditor() {
}

//==============================================================================
void ECEditor::paint(juce::Graphics& g) {
    g.setFont(juce::Font("Helvetica", 13.0f, juce::Font::plain));
    // getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
    // g.setColour (juce::Colours::black);

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::lightgrey);
    g.setFont(13.0f);

    // Rectangle<int> area = getLocalBounds();
    Rectangle<int> area = getLocalBounds();
    auto bottomStrip = area.removeFromBottom(18);
    bottomStrip.removeFromRight(30);  // Clear resize window icon

    Rectangle<int> leftGainArea = area.removeFromLeft(200);
    Rectangle<int> controlsArea = area.removeFromLeft((970-120) * 0.4);
    Rectangle<int> usableControlArea = controlsArea.reduced(10);

    auto sliderarea = leftGainArea.removeFromLeft(55);
    sliderarea.removeFromLeft(10).removeFromRight(10);
    g.drawFittedText("Gain", sliderarea.removeFromTop(20), juce::Justification::centred, 1);
    gainSlider.setBounds(sliderarea);
    auto meterArea = leftGainArea.removeFromLeft(55);
    meterArea.removeFromTop(25);
    meterArea.removeFromBottom(10);
    meter.setBounds(meterArea);

    auto silenceArea = leftGainArea;
    silenceArea.removeFromLeft(10).removeFromRight(10);
    auto ledArea = silenceArea.removeFromTop(40).reduced(5);
    silenceLed.setBounds(ledArea);
    g.drawFittedText("Silence Threshold", silenceArea.removeFromTop(20), juce::Justification::centred, 1);
    silenceThSlider.setBounds(silenceArea);


    waveformDisplayComponent.setBounds(area.reduced(10));

    std::string date(__DATE__);
    std::string time(__TIME__);
    g.drawFittedText("Domenico Stefani, Luca Turchet, Johan Pauwels", bottomStrip.removeFromLeft(bottomStrip.getWidth() * 0.5), juce::Justification::left, 1);
    g.drawFittedText("Compiled on: " + date + " at " + time, bottomStrip, juce::Justification::right, 1);

    int leftHeight = usableControlArea.getHeight();

    selectSaveFolderButton.setBounds(usableControlArea.removeFromTop(70).reduced(5));
    recButton.setBounds(usableControlArea.removeFromTop(40).reduced(5));

    g.setFont(Font(17.0f, Font::bold));
    g.drawFittedText("Status:", usableControlArea.removeFromTop(18), juce::Justification::left, 1);
    usableControlArea.removeFromTop(0.05 * leftHeight);

    status.setBounds(usableControlArea);
}

void ECEditor::resized() {}
