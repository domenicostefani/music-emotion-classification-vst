#include "PluginEditor.h"

#include <regex>

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

void ECEditor::modelButtonClicked() {
    modelChooser = std::make_unique<juce::FileChooser>("Select a model file...", File(), "*.tflite");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    modelChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file != juce::File{} && file.existsAsFile() && file.hasFileExtension(".tflite")) {
            if (audioProcessor.loadModel(file.getFullPathName().toStdString())) {
                this->recButton.setEnabled(true);
                selectModelButton.setButtonText("Select model\n(current: \"" + file.getFileName().toStdString() + "\")");
            } else {
                this->audioProcessor.clearGUIstate();
                this->audioProcessor.appendToGUIstate("Error loading model. Try again.");
            }
        }
    });
}

void ECEditor::infoButtonClicked() {
    
    // Get README_md index
    size_t readme_index = 0;
    for (int i = 0; i < BinaryData::namedResourceListSize; i++)
        if (BinaryData::originalFilenames[i] == "README.md")
            readme_index = i;

    const char* readmeResNameUTF8 = BinaryData::namedResourceList[readme_index];
    int size;

    // Get README_md content
    std::string readme_content = std::string(BinaryData::getNamedResource(readmeResNameUTF8, size));
    std::regex re;

    // Remove the main title (text following the first #)
    size_t pos = readme_content.find("#");
    if (pos != std::string::npos) {
        size_t pos2 = readme_content.find("\n", pos + 1);
        if (pos2 != std::string::npos)
            readme_content.erase(pos, pos2 - pos);
    }

    // Remove Markdown links
    std::regex link_regex("\\[(.*?)\\]\\((.*?)\\)");
    readme_content = std::regex_replace(readme_content, link_regex, "$1");

    // Remove entire "Dependencies section" (from "## Dependencies", all the lines to the next "##")
    // create lambda to remove section
    auto removeSection = [&readme_content](std::string section_name) {
        size_t pos = readme_content.find("## " + section_name);
        if (pos != std::string::npos) {
            size_t pos2 = readme_content.find("##", pos + 1);
            if (pos2 != std::string::npos)
                readme_content.erase(pos, pos2 - pos);
        }
    };

    removeSection("Dependencies");
    removeSection("Repository");
    // Ignore Markdown comments
    std::regex comment_re("<!--.*?-->");
    readme_content = std::regex_replace(readme_content, comment_re, "");
    // Remove all markdown images
    std::regex image_regex("!\\[(.*?)\\]\\((.*?)\\)");
    readme_content = std::regex_replace(readme_content, image_regex, "");
    // Remove all markdown bold and italic formatting
    std::regex bold_regex("\\*\\*(.*?)\\*\\*");
    readme_content = std::regex_replace(readme_content, bold_regex, "$1");

    // Remove all text beween the first # that follows Section "### GUI version" and the "## References" section
    pos = readme_content.find("### GUI version");
    if (pos != std::string::npos) {
        size_t pos2 = readme_content.find("## References", pos + 10);
        if (pos2 != std::string::npos) {
            size_t pos3 = readme_content.find("#", pos + 10);
            if (pos3 != std::string::npos && pos3 < pos2)
                readme_content.erase(pos3, pos2 - pos3);
        }
    }
    // Insert new text before the "## References" section
    pos = readme_content.find("## References");
    if (pos != std::string::npos) {
        readme_content.insert(pos,"\n\nMore information can be found in the Github project repository, in the README.md file.\n\n");
    }



    std::string infotext = readme_content;

    juce::TextEditor * label = new juce::TextEditor ();
    label->setMultiLine(true);
    label->setScrollbarsShown(true);
    label->setReadOnly(true);
    label->setCaretVisible(false);
    label->setPopupMenuEnabled(false);

    label->setText(infotext, juce::dontSendNotification);


    juce::DialogWindow::LaunchOptions o;      
    o.content.setOwned(label);
    o.content->setSize(600, 450);
    o.dialogTitle = "Emotion Classificatio Plugin Info";
    o.dialogBackgroundColour = o.content->getLookAndFeel().findColour(juce::DocumentWindow::backgroundColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
    o.componentToCentreAround = this;
    
    o.launchAsync();
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
    setSize(970, 380);
    setResizeLimits(970, 350, 1920, 1080);

#ifdef LIGHT_THEME
    setLookAndFeel(&lf);
#endif

    selectSaveFolderButton.setButtonText("Select save folder\nDefault: \"" + audioProcessor.getSaveFolder().getFullPathName().toStdString() + "\"");
    selectSaveFolderButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(selectSaveFolderButton);

    infoButton.setButtonText("Info");
    infoButton.onClick = [this] { infoButtonClicked(); };
    addAndMakeVisible(infoButton);

    std::string loadedModPath = audioProcessor.getModelPath();
    // take only basename with extension
    std::string loadedModName = loadedModPath.substr(loadedModPath.find_last_of("/\\") + 1);
    selectModelButton.setButtonText("Select model\nDefault: \"" + loadedModName);
    selectModelButton.onClick = [this] { modelButtonClicked(); };
    addAndMakeVisible(selectModelButton);

    addAndMakeVisible(recButton);
    recButton.setButtonText("Record");
    recButton.setColour(TextButton::buttonColourId, Colours::red);
    recButton.setColour(TextButton::buttonOnColourId, Colours::green);
    recButton.setColour(TextButton::textColourOnId, Colours::white);
    recButton.setColour(TextButton::textColourOffId, Colours::white);

    // recButton.onClick = [this] { audioProcessor.record(); };
    recButton.onClick = [this] { toggle(); };
    recButton.setClickingTogglesState(true);
    recButton.setEnabled(audioProcessor.enableRec.load());

    recButtonAttachment.reset(new ButtonAttachment(audioProcessor.valueTreeState, audioProcessor.RECSTATE_ID, recButton));

    addAndMakeVisible(status);
    // status.setColour(Label::textColourId, Colours::white);
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
    silenceThSlider.setTextValueSuffix(" dB");
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
    // g.setColour(juce::Colours::lightgrey);
    g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
    g.setFont(13.0f);

    // Rectangle<int> area = getLocalBounds();
    Rectangle<int> area = getLocalBounds();
    auto bottomStrip = area.removeFromBottom(18);

    Rectangle<int> leftGainArea = area.removeFromLeft(250);
    Rectangle<int> controlsArea = area.removeFromLeft((970 - 120) * 0.4);
    Rectangle<int> usableControlArea = controlsArea.reduced(10);

    auto sliderarea = leftGainArea.removeFromLeft(75);
    g.drawFittedText("Gain", sliderarea.removeFromTop(20), juce::Justification::centred, 1);
    sliderarea.removeFromLeft(10);
    sliderarea.removeFromRight(10);
    sliderarea.removeFromTop(20);
    sliderarea.removeFromBottom(20);
    gainSlider.setBounds(sliderarea);
    auto meterArea = leftGainArea.removeFromLeft(75);
    g.drawFittedText("Peak Meter", meterArea.removeFromTop(20), juce::Justification::centred, 1);
    meterArea.removeFromTop(20);
    meterArea.removeFromBottom(20);
    meterArea.removeFromLeft(10);
    meterArea.removeFromRight(10);
    meter.setBounds(meterArea);

    auto silenceArea = leftGainArea;
    silenceArea.removeFromLeft(10);
    silenceArea.removeFromRight(10);
    auto ledTextArea = silenceArea.removeFromTop(40).reduced(5);
    g.drawFittedText("Playing\nDetector", ledTextArea, juce::Justification::centred, 1);
    auto ledArea = silenceArea.removeFromTop(40).reduced(5);
    silenceLed.setBounds(ledArea);
    g.drawFittedText("Silence\nThreshold", silenceArea.removeFromTop(28), juce::Justification::centred, 1);
    silenceThSlider.setBounds(silenceArea);

    waveformDisplayComponent.setBounds(area.reduced(10));

    std::string date(__DATE__);
    std::string time(__TIME__);

    // Draw a slightly lighter rectangle under bottom strip text
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(-0.2f));
    g.fillRect(bottomStrip);
    g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));

    bottomStrip.removeFromRight(30);  // Clear resize window icon
    bottomStrip.removeFromLeft(15);
    infoButton.setBounds(bottomStrip.removeFromLeft(30));
    bottomStrip.removeFromLeft(15);
    g.drawFittedText("Domenico Stefani, Luca Turchet, Johan Pauwels", bottomStrip.removeFromLeft(bottomStrip.getWidth() * 0.5), juce::Justification::centredLeft, 1);
    g.drawFittedText("Compiled on: " + date + " at " + time, bottomStrip, juce::Justification::centredRight, 1);

    int leftHeight = usableControlArea.getHeight();

    selectSaveFolderButton.setBounds(usableControlArea.removeFromTop(70).reduced(5));
    selectModelButton.setBounds(usableControlArea.removeFromTop(60).reduced(5));
    recButton.setBounds(usableControlArea.removeFromTop(40).reduced(5));

    // g.setFont(Font(17.0f, Font::bold));
    g.drawFittedText("Status:", usableControlArea.removeFromTop(18), juce::Justification::left, 1);
    usableControlArea.removeFromTop(0.05 * leftHeight);

    status.setBounds(usableControlArea);
}

void ECEditor::resized() {}
