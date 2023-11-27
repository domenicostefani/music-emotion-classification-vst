/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

#include <fstream>
#include <sstream>    // std::stringstream
#include <stdexcept>  // std::runtime_error
#include <string>
#include <thread>
#include <vector>

#include "initialComponent.h"

#define TERM_TXT "mate-terminal"
#define EXPLORER "nautilus"

float MainComponent::getOscInLevel() const {
    // return -60.0f;  // TODO fix
    return this->oscSaysMeter;
}
bool MainComponent::getOscIsSilent() const {
    return this->oscSaysSilent;
}

void MainComponent::showStatus(int status) {
    std::string statusStr = "Status: ";
    juce::Colour sc = juce::Colours::black;
    switch (status) {
        case 0:
            recBlinker.stopBlinking();
            statusStr += "Disconnected";
            break;
        case 1:
            statusStr += "Idle";
            recBlinker.stopBlinking();
            sc = juce::Colours::darkgreen;
            break;
        case 2:
            statusStr += "Recording";
            sc = juce::Colours::red;
            recBlinker.startBlinking();
            break;
        case 3:
            statusStr += "Classifying";
            recBlinker.stopBlinking();
            sc = juce::Colours::orange;
            break;
        default:
            statusStr += "Unknown";
            sc = juce::Colours::darkred;
            break;
    }

    statusLabel.setText(statusStr, juce::dontSendNotification);
}

void MainComponent::openSaveBanner() {
    DBG("openSaveBanner called");
    auto saveAs = [this](std::string strfilename) {
        std::cout << "Saving as " << strfilename << "\n";
        if (!oscSender1_toserver.send("/rename", juce::String(strfilename)))
            showConnectionErrorMessage("Error: could not send OSC message.");
        setRecCommandsEnabled(true);
    };

    auto trash = [this]() {
        std::cout << "Trash "
                  << "\n";
        if (!oscSender1_toserver.send("/deleteUnnamed"))
            showConnectionErrorMessage("Error: could not send OSC message.");
        setRecCommandsEnabled(true);
    };

    auto advanceRecnameCb = [this]() {
        std::cout << "Advance recname "
                  << "\n";
        this->advanceRecname();
    };

    auto advanceAndGetNext = [this]() {
        std::cout << "getNextAndAdvance "
                  << "\n";
        this->advanceRecname();
        return this->getCurrentRecname();
    };

    auto prevAndGetNext = [this]() {
        std::cout << "getNextAndAdvance "
                  << "\n";
        this->prevRecname();
        return this->getCurrentRecname();
    };

    DBG("Creating saveComponentPtr");
    saveComponentPtr = std::make_unique<SaveComponent>(getCurrentRecname(), saveAs, trash, advanceRecnameCb, advanceAndGetNext, prevAndGetNext);

    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned(saveComponentPtr.release());
    o.content->setSize(900, 180);
    o.dialogTitle = "Save Recording";
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
    // o.componentToCentreAround = this;

    // o.launchAsync();
    DBG("Launching saveComponentPtr");
    o.runModal();
}

void MainComponent::oscMessageReceived(const juce::OSCMessage &message) {
    // std::cout << "Received message " << message.getAddressPattern().toString().toStdString() << std::endl;

    if (message.getAddressPattern() == juce::OSCAddressPattern("/pyshake")) {
        if (!this->waitForHandshakeWithServer) {
            std::cout << "Received unwanted handshake from SERVER" << std::endl;
        } else {
            std::cout << "-> Handshake with server received!" << std::endl;
            oscSender1_toserver.send("/startPlugin", juce::String(INSTRUMENT));  // Send handshake
            waitForHandshakeWithServer = false;
            waitForHandshakeWithPlugin = true;
            // Now keep sending the plugin handshake until we get a reply
            if (!oscSender2_toPlugin.connect(boardIpText.getText().toStdString(), OSC_SEND_PORT_2_TOPLUGIN))
                showConnectionErrorMessage("Error: could not connect to UDP port " + std::to_string(OSC_SEND_PORT_2_TOPLUGIN) + ".");
            else
                pluginHandshakePoller.startPolling(200);  // Send every 200ms
        }

    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/handshake")) {
        if (!this->waitForHandshakeWithPlugin) {
            std::cout << "Received unwanted handshake from PLUGIN" << std::endl;
        } else {
            std::cout << "-> Handshake with plugin received!" << std::endl;
            waitForHandshakeWithPlugin = false;
            pluginHandshakePoller.stopPolling();
            isConnectionMade = true;
            // Connect also sushi
            if (!oscSender3_toSushi.connect(boardIpText.getText().toStdString(), OSC_SEND_PORT_3_TOSUSHI))
                showConnectionErrorMessage("Error: could not connect to UDP port " + std::to_string(OSC_SEND_PORT_3_TOSUSHI) + ".");

            setIPFieldsEnabled(false);
            setWorkingCommandsEnabled(true);
            connectBtn.setButtonText("Disconnect");
            repaint();

            termText.setText(juce::String(TERM_TXT) + juce::String(" --window -e \"ssh mind@" + boardIpText.getText() + " -t tmux a \""));
            termBtn.setButtonText("Open SSH");

            // mate-terminal --window -e "ssh mind@elk-pi.local -t tmux a"

            // Set date
            if (!dateSetOnce)
                setBoardDateTime();

            juce::String homefolderpath = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName();
            // Get username from home path
            juce::String username = homefolderpath.fromLastOccurrenceOf("/", false, true);

            std::cout << "homefolderpath: " << homefolderpath.toStdString() << "\n";
            std::cout << "username: " << username.toStdString() << "\n";

            juce::String copydetails = username + juce::String("@") + controllerIpText.getText() + juce::String(":") + homefolderpath + juce::String("/");

            // send copy details
            oscSender1_toserver.send("/setCopyDetails", copydetails);
            // sleep for 1 second
            juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 1000);
            oscSender1_toserver.send("/alwaysCopyRenamed");
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/silence")) {
        if (isConnectionMade) {
            if (message.size() == 1 && message[0].isInt32()) {
                // std::cout << "-> silence: " << message[0].getInt32() << std::endl;
                // silenceLed.repaint();
                oscSaysSilent = (bool)message[0].getInt32();
            }
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/meter")) {
        if (isConnectionMade) {
            if (message.size() == 1 && message[0].isFloat32()) {
                // std::cout << "-> meter: " << message[0].getFloat32() << std::endl;
                // meter.repaint();
                oscSaysMeter = message[0].getFloat32();
            }
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/state")) {
        // std::cout << "Received state" << std::endl;
        if (message.size() == 1 && message[0].isInt32()) {
            // std::cout << "-> state: " << message[0].getInt32() << std::endl;
            showStatus(message[0].getInt32() + 1);
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/emotion")) {
        // std::cout << "Received emotion result" << std::endl;
        if (message.size() == 1) {
            resultErrorLabel.setText("Error: recording too int or silent", juce::dontSendNotification);
            resultErrorLabel.setColour(juce::Label::textColourId, juce::Colours::red);

            // Send a deleteUnnamed message to the server
            oscSender1_toserver.send("/deleteUnnamed");
            setRecCommandsEnabled(true);
        }
        if (message.size() == 4 &&
            message[0].isInt32() &&
            message[1].isInt32() &&
            message[2].isInt32() &&
            message[3].isInt32()) {
            int resEmo = message[0].getInt32() * 1000 + message[1].getInt32() * 100 + message[2].getInt32() * 10 + message[3].getInt32();
            std::cout << "-> emotion: " << resEmo << std::endl;
            // if (resEmo == -1) {
            //     resultErrorLabel.setText("Error: recording too int or silent", juce::dontSendNotification);
            //     resultErrorLabel.setColour(juce::Label::textColourId, juce::Colours::red);
            // } else {
            resultErrorLabel.setText(juce::String("Received result ") + juce::String(std::to_string(resEmo)), juce::dontSendNotification);
            openSaveBanner();
            resultErrorLabel.setColour(juce::Label::textColourId, juce::Colours::darkgreen);
            returnedEmotion.set(resEmo);

            aggrTog.setToggleState(returnedEmotion.getAggressive(), juce::dontSendNotification);
            relTog.setToggleState(returnedEmotion.getRelaxed(), juce::dontSendNotification);
            hapTog.setToggleState(returnedEmotion.getHappy(), juce::dontSendNotification);
            sadTog.setToggleState(returnedEmotion.getSad(), juce::dontSendNotification);
            // }
            // emoAggressiveLed.repaint();
            // emoRelaxedLed.repaint();
            // emoHappyLed.repaint();
            // emoSadLed.repaint();
        }
    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/renamed")) {
        if (message.size() == 1 && message[0].isString()) {
            auto arg = message[0].getString();
            if (arg == juce::String("ok")) {
                renamerLed.turnOn(juce::Colours::green);
            } else if (arg == juce::String("errorTooManyFiles")) {
                renamerLed.turnOn(juce::Colours::red);
                // renamerStatus.setText();
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                       "Rename Error",
                                                       "Error in renaming last recording: There are multiple recordings that have not been renamed yet. I don't know if you want to keep the last or one of the previous ones",
                                                       "OK");
                openExplorer();

            } else if (arg == juce::String("errorNoFile")) {
                renamerLed.turnOn(juce::Colours::red);
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                       "Rename Error",
                                                       "Error in renaming: There are no recordings",
                                                       "OK");
                openExplorer();

            } else if (arg == juce::String("errorOverwrite")) {
                renamerLed.turnOn(juce::Colours::red);
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                       "Rename Error",
                                                       "Error in renaming: Name already exists!",
                                                       "OK");
                openSaveBanner();
            };
        }
    } else {
        std::cout << "Received unknown message" << std::endl;
    }

    // if (message.getAddressPattern() == juce::OSCAddressPattern("/handshake")) {
    //     std::cout << "-> Handshake!" << std::endl;
    //     if (message.size() == 1 && message[0].isString()) {
    //         std::cout << "-> ip: " << message[0].getString() << std::endl;
    //         this->oscSender.enableAndReplyToHandshake(message[0].getString(), TX_PORT);
    //         oscMeterPoller = std::make_unique<Poller>(24, [&]() { this->oscSendPollingRoutine(); });
    //         this->oscSender.sendMessage("/state", (int)STATE_IDLE);
    //     }
    // } else if (message.getAddressPattern() == juce::OSCAddressPattern("/disconnect")) {
    //     std::cout << "-> disconnect" << std::endl;
    //     oscMeterPoller.reset();
    // } else if (message.getAddressPattern() == juce::OSCAddressPattern("/gain")) {
    //     if (message.size() == 1 && message[0].isFloat32()) {
    //         std::cout << "-> gain: " << message[0].getFloat32() << std::endl;
    //     }
    // }
}

//==============================================================================
MainComponent::MainComponent(EProcessor &p,
                             std::vector<std::string> &recordingNames,
                             std::unique_ptr<EmoDB> &emoDB) : meter([&]() { return this->getOscInLevel(); }),
                                                              silenceLed([&]() { return !this->getOscIsSilent(); }),
                                                              emoAggressiveLed([&]() { return this->returnedEmotion.getAggressive(); }),
                                                              emoRelaxedLed([&]() { return this->returnedEmotion.getRelaxed(); }),
                                                              emoHappyLed([&]() { return this->returnedEmotion.getHappy(); }),
                                                              emoSadLed([&]() { return this->returnedEmotion.getSad(); }),
                                                              oscReceiverFromServer(RX_PORT_SERVER, [&](const juce::OSCMessage &m) { this->oscMessageReceived(m); }),
                                                              oscReceiverFromPlugin(RX_PORT_PLUGIN, [&](const juce::OSCMessage &m) { this->oscMessageReceived(m); }),
                                                              audioProcessor(p),
                                                              recordingNames(recordingNames),
                                                              emoDB(std::move(emoDB)) {
    setSize(600, 700);

    addAndMakeVisible(instrumentLabel);
    instrumentLabel.setJustificationType(juce::Justification::centred);
    // Set larger font
    instrumentLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    if (INSTRUMENT == "electric")
        instrumentLabel.setText("Controller for Electric Guitar", juce::dontSendNotification);
    else if (INSTRUMENT == "piano")
        instrumentLabel.setText("Controller for Piano", juce::dontSendNotification);
    else
        instrumentLabel.setText("Controller for Unknown Instrument", juce::dontSendNotification);

    addAndMakeVisible(connectBtn);
    connectBtn.setButtonText("Connect");
    connectBtn.addListener(this);

    addAndMakeVisible(termBtn);
    termBtn.setButtonText("Open Empty Term");
    termBtn.addListener(this);

    addAndMakeVisible(explorerBtn);
    explorerBtn.setButtonText("Open Explorer");
    explorerBtn.addListener(this);

    addAndMakeVisible(panicBtn);
    panicBtn.setButtonText("Panic: RESTART PLUGIN ON THE BOARD");
    panicBtn.addListener(this);
    panicBtn.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);

    addAndMakeVisible(termText);
    termText.setText(TERM_TXT);

    addAndMakeVisible(boardIpLabel);
    boardIpLabel.setJustificationType(juce::Justification::centred);
    boardIpLabel.setText("Board IP", juce::dontSendNotification);
    addAndMakeVisible(controllerIpLabel);
    controllerIpLabel.setJustificationType(juce::Justification::centred);
    controllerIpLabel.setText("Controller IP", juce::dontSendNotification);

    addAndMakeVisible(boardIpText);
    boardIpText.setText("elk-pi.local");
    addAndMakeVisible(controllerIpText);

    controllerIpText.setText("10.42.0.1");

    std::vector<juce::String> oscMessagesServer = {"/pyshake", "/renamed", "/stopped"};
    for (const auto &oscMessage : oscMessagesServer)
        oscReceiverFromServer.addOSCListener(oscMessage);

    std::vector<juce::String> oscMessagesPlugin = {"/handshake", "/state", "/meter", "/silence", "/emotion"};
    for (const auto &oscMessage : oscMessagesPlugin)
        oscReceiverFromPlugin.addOSCListener(oscMessage);

    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0.0, 1.0);
    gainSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f,
                                                                    1.0f,
                                                                    0.0001,
                                                                    0.4,
                                                                    false));
    gainSlider.setSliderSnapsToMousePosition(false);

    gainSlider.addListener(this);
    gainSlider.setValue(0.999);  // TODO: Check why it is not working
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 20);
    gainSlider.setTextValueSuffix(" dB");

    addAndMakeVisible(silenceSlider);
    // silenceSlider.setRange(0.0, 1.0);
    gainSlider.setValue(0.0);
    silenceSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    silenceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 20);
    silenceSlider.setTextValueSuffix(" dB");
    silenceSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0f,
                                                                       1.0f,
                                                                       0.0001,
                                                                       0.4,
                                                                       false));
    silenceSlider.setSliderSnapsToMousePosition(false);
    silenceSlider.addListener(this);

    auto round1 = [](float var) {
        float value = (int)(var * 100 + .5);
        return (float)value / 100;
    };

    gainSlider.textFromValueFunction = [round1](double value) {
        return juce::String(round1(juce::Decibels::gainToDecibels(value)));
    };

    gainSlider.valueFromTextFunction = [](const juce::String &text) {
        if (text.isEmpty()) return 0.0;
        if (!text.containsOnly("0123456789.-")) return 0.0;
        double value = text.getDoubleValue();
        if (value < -120.0) return 0.0;
        if (value > 0.0) return 1.0;
        return juce::Decibels::decibelsToGain(value);
    };

    silenceSlider.textFromValueFunction = [round1](double value) {
        if (value == 0.0) return juce::String("-inf");
        if (value == 1.0) return juce::String("0.0");
        return juce::String(round1((value - 1) * 120));
    };

    silenceSlider.valueFromTextFunction = [](const juce::String &text) {
        if (text.isEmpty()) return 0.0;
        if (!text.containsOnly("0123456789.-")) return 0.0;
        double value = text.getDoubleValue();
        if (value < -120.0) return 0.0;
        if (value > 0.0) return 1.0;

        return value / 120 + 1;
    };

    addAndMakeVisible(gainLabel);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setText("Gain", juce::dontSendNotification);

    addAndMakeVisible(silenceLabel);
    silenceLabel.setJustificationType(juce::Justification::centred);
    silenceLabel.setText("Sound Detected", juce::dontSendNotification);

    addAndMakeVisible(silenceThresholdLabel);
    silenceThresholdLabel.setJustificationType(juce::Justification::centred);
    silenceThresholdLabel.setText("Silence Threshold", juce::dontSendNotification);

    addAndMakeVisible(meter);
    addAndMakeVisible(silenceLed);

    addAndMakeVisible(emoAggressiveLabel);
    emoAggressiveLabel.setText("Aggressive", juce::dontSendNotification);
    emoAggressiveLabel.setJustificationType(juce::Justification::centred);
    emoAggressiveLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(emoRelaxedLabel);
    emoRelaxedLabel.setText("Relaxed", juce::dontSendNotification);
    emoRelaxedLabel.setJustificationType(juce::Justification::centred);
    emoRelaxedLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(emoHappyLabel);
    emoHappyLabel.setText("Happy", juce::dontSendNotification);
    emoHappyLabel.setJustificationType(juce::Justification::centred);
    emoHappyLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(emoSadLabel);
    emoSadLabel.setText("Sad", juce::dontSendNotification);
    emoSadLabel.setJustificationType(juce::Justification::centred);
    emoSadLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(emoAggressiveLed);
    addAndMakeVisible(emoRelaxedLed);
    addAndMakeVisible(emoHappyLed);
    addAndMakeVisible(emoSadLed);
    addAndMakeVisible(resultErrorLabel);

    resultErrorLabel.setJustificationType(juce::Justification::centred);
    resultErrorLabel.setFont(juce::Font(20.0f, juce::Font::plain));
    resultErrorLabel.setColour(juce::Label::backgroundColourId, juce::Colours::lightgrey);
    addAndMakeVisible(playExcerptBtn);
    playExcerptBtn.setButtonText("Play Excerpt with classified emotions");
    playExcerptBtn.addListener(this);

    addAndMakeVisible(aggrTog);
    aggrTog.setButtonText("Aggressive");
    addAndMakeVisible(relTog);
    relTog.setButtonText("Relaxed");
    addAndMakeVisible(hapTog);
    hapTog.setButtonText("Happy");
    addAndMakeVisible(sadTog);
    sadTog.setButtonText("Sad");

    addAndMakeVisible(ex1Btn);
    ex1Btn.setButtonText("Play/Stop 1");
    ex1Btn.addListener(this);

    addAndMakeVisible(ex2Btn);
    ex2Btn.setButtonText("Play/Stop 2");
    ex2Btn.addListener(this);

    addAndMakeVisible(recNameLabel);
    recNameLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(startBtn);
    startBtn.setButtonText("Start");
    startBtn.addListener(this);

    addAndMakeVisible(stopBtn);
    stopBtn.setButtonText("Stop");
    stopBtn.addListener(this);

    addAndMakeVisible(recBlinker);
    addAndMakeVisible(statusLabel);
    // statusLabel.setText("Status: Idle",dontSendNotification);
    showStatus(0);

    addAndMakeVisible(renamerLed);

#ifdef OLD_RENAMER
    addAndMakeVisible(renameBtn);
    renameBtn.setButtonText("Rename&Copy");
    renameBtn.addListener(this);

    addAndMakeVisible(nextFilenameBtn);
    nextFilenameBtn.setButtonText(">");
    nextFilenameBtn.addListener(this);

    addAndMakeVisible(prevFilenameBtn);
    prevFilenameBtn.setButtonText("<");
    prevFilenameBtn.addListener(this);

    addAndMakeVisible(fileCounterLabel);
    fileCounterLabel.setText("0/0", juce::dontSendNotification);

    addAndMakeVisible(filename);
    filename.setText("");
    switchToNextFilename();
#endif

    setWorkingCommandsEnabled(false);

    // double sr = getSampleRate();
    // emoDB.prepareToPlay();

#ifdef TEST_BTN
    TESTBTN.setButtonText("SAVE TEST");              // TODO:remove
    TESTBTN.onClick = [this] { openSaveBanner(); };  // TODO:remove
    addAndMakeVisible(TESTBTN);                      // TODO:remove
#endif
    resetRecname();
}

MainComponent::~MainComponent() {
    // emoDB.releaseResources();
    oscSender1_toserver.disconnect();
    oscSender2_toPlugin.disconnect();
    oscSender3_toSushi.disconnect();
}

//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
    g.setColour((isConnectionMade) ? juce::Colours::darkgreen : juce::Colours::red);
    g.fillRect(wholearea);

    g.setColour(juce::Colour(0xFF323e44));
    // FF323e44
    g.fillRect(basearea);

    g.setColour(juce::Colours::lightgrey);
    g.fillRect(origEmoArea);

    // g.setColour(juce::Colours::lightgrey);
    // g.fillRect(resultStatusArea);

    // g.setColour(juce::Colours::limegreen);
    // g.fillRect(originalDatabaseArea);
}

void MainComponent::resized() {
    const int SLIDER_WIDTH = 60;
    const int METER_WIDTH = 60;
    const int LED_WIDTH = 20;

    int vSeparator = 20;
    wholearea = getLocalBounds();
    basearea = wholearea.reduced(8);

    auto area = basearea.reduced(10);
#ifdef TEST_BTN
    TESTBTN.setBounds(area.removeFromTop(30));  // TODO: remove
#endif

    instrumentLabel.setBounds(area.removeFromTop(40));
    juce::Rectangle<int> top = area.removeFromTop(50);
    area.removeFromTop(vSeparator);

    int vIntervals = 5;
    int verticalInterval = (area.getHeight() - vSeparator * (vIntervals - 1)) / vIntervals;
    auto termArea = area.removeFromTop(70);

    juce::Rectangle<int> volumesArea = area.removeFromTop(verticalInterval * 2);
    area.removeFromTop(vSeparator);
    juce::Rectangle<int> playStopStatusArea = area.removeFromTop(verticalInterval * 0.6 + 20);
    area.removeFromTop(vSeparator);
    juce::Rectangle<int> emoArea = area.removeFromTop(verticalInterval * 1.2);
    repaint();

    area.removeFromTop(vSeparator);
    juce::Rectangle<int> databaseArea = area.removeFromTop(60);
    originalDatabaseArea = databaseArea;
    area.removeFromTop(vSeparator);
    juce::Rectangle<int> renameArea = area;

    int hSeparator = 15, blocks = 3;
    int horBlock = (area.getWidth() - (hSeparator * (blocks - 1))) / blocks;

    auto labelStrip = top.removeFromTop(top.getHeight() / 2);
    boardIpLabel.setBounds(labelStrip.removeFromLeft(horBlock));
    labelStrip.removeFromLeft(hSeparator);
    controllerIpLabel.setBounds(labelStrip.removeFromLeft(horBlock));

    auto connectInputStrip = top;

    boardIpText.setBounds(connectInputStrip.removeFromLeft(horBlock));
    connectInputStrip.removeFromLeft(hSeparator);
    controllerIpText.setBounds(connectInputStrip.removeFromLeft(horBlock));
    connectInputStrip.removeFromLeft(hSeparator);
    connectBtn.setBounds(connectInputStrip);

    auto exparea = termArea.removeFromBottom(30);
    termArea.removeFromTop(5);
    termArea.removeFromBottom(5);
    termText.setBounds(termArea.removeFromLeft(horBlock * 2 + hSeparator));
    termArea.removeFromLeft(hSeparator);

    termBtn.setBounds(termArea);

    exparea.removeFromLeft(horBlock + hSeparator);
    panicBtn.setBounds(exparea.removeFromLeft(horBlock));
    exparea.removeFromLeft(hSeparator);
    explorerBtn.setBounds(exparea);

    auto gainControlArea = volumesArea.removeFromLeft(horBlock);
    gainLabel.setBounds(gainControlArea.removeFromTop(20));
    // int toremove = (horBlock - SLIDER_WIDTH) / 2;
    // gainControlArea.removeFromLeft(toremove);
    // gainControlArea.removeFromRight(toremove);
    gainSlider.setBounds(gainControlArea);

    auto levelMeterArea = volumesArea.removeFromLeft(horBlock);
    int meterToremove = (horBlock - METER_WIDTH) / 2;
    levelMeterArea.removeFromLeft(meterToremove);
    levelMeterArea.removeFromRight(meterToremove);
    meter.setBounds(levelMeterArea);

    auto silenceControlArea = volumesArea;
    silenceLabel.setBounds(silenceControlArea.removeFromTop(20));
    int toremoveLedsides = (horBlock - LED_WIDTH) / 2;
    auto ledArea = silenceControlArea.removeFromTop(20);
    ledArea.removeFromLeft(toremoveLedsides);
    ledArea.removeFromRight(toremoveLedsides);
    silenceLed.setBounds(ledArea);
    // silenceControlArea.removeFromLeft(toremove);
    // silenceControlArea.removeFromRight(toremove);
    silenceThresholdLabel.setBounds(silenceControlArea.removeFromTop(20));
    silenceSlider.setBounds(silenceControlArea);

    // playStopStatusArea

    auto cromFromTopBottom = [](auto area, int top, int bottom) {
        area.removeFromTop(top);
        area.removeFromBottom(bottom);
        return area;
    };

    recNameLabel.setBounds(playStopStatusArea.removeFromTop(20));
    startBtn.setBounds(playStopStatusArea.removeFromLeft(horBlock * 1.3).reduced(5));
    auto stopArea = cromFromTopBottom(playStopStatusArea.removeFromLeft(horBlock * 0.7), 10, 10);
    stopBtn.setBounds(stopArea.reduced(5));

    auto statusArea = cromFromTopBottom(playStopStatusArea, 10, 10);
    recBlinker.setBounds(statusArea);
    statusLabel.setBounds(statusArea);

    // Emotion area

    resultStatusArea = emoArea.removeFromBottom(25);
    resultErrorLabel.setBounds(resultStatusArea);
    origEmoArea = emoArea;

    int hfourth = (emoArea.getWidth() - (hSeparator * (4 - 1))) / 4;
    int emoLblHeight = 15;
    auto aggemoArea = emoArea.removeFromLeft(hfourth).reduced(10);
    emoAggressiveLabel.setBounds(aggemoArea.removeFromTop(emoLblHeight));
    emoAggressiveLed.setBounds(aggemoArea.reduced(7));
    emoArea.removeFromLeft(hSeparator);

    auto relemoArea = emoArea.removeFromLeft(hfourth).reduced(10);
    emoRelaxedLabel.setBounds(relemoArea.removeFromTop(emoLblHeight));
    emoRelaxedLed.setBounds(relemoArea.reduced(7));
    emoArea.removeFromLeft(hSeparator);

    auto hapemoArea = emoArea.removeFromLeft(hfourth).reduced(10);
    emoHappyLabel.setBounds(hapemoArea.removeFromTop(emoLblHeight));
    emoHappyLed.setBounds(hapemoArea.reduced(7));
    emoArea.removeFromLeft(hSeparator);

    auto sademoArea = emoArea.reduced(10);
    emoSadLabel.setBounds(sademoArea.removeFromTop(emoLblHeight));
    emoSadLed.setBounds(sademoArea.reduced(7));

    // database area

    int thirdNospace = area.getWidth() / 3;

    auto cropFromLeftRight = [](auto area, int left, int right) {
        area.removeFromLeft(left);
        area.removeFromRight(right);
        return area;
    };

    databaseArea.removeFromTop(5);
    databaseArea.removeFromBottom(5);

    auto togglearea = databaseArea.removeFromTop(30);
    auto fourth = togglearea.getWidth() / 4;
    aggrTog.setBounds(togglearea.removeFromLeft(fourth));
    relTog.setBounds(togglearea.removeFromLeft(fourth));
    hapTog.setBounds(togglearea.removeFromLeft(fourth));
    sadTog.setBounds(togglearea);

    playExcerptBtn.setBounds(cropFromLeftRight(databaseArea.removeFromLeft(thirdNospace * 2), 5, 5));
    ex1Btn.setBounds(cropFromLeftRight(databaseArea.removeFromLeft(thirdNospace / 2), 5, 5));
    ex2Btn.setBounds(cropFromLeftRight(databaseArea.removeFromLeft(thirdNospace / 2), 5, 5));

    // rename area
    auto renameAreaTop = renameArea.removeFromTop(renameArea.getHeight() / 2);
    auto renameAreaBottom = renameArea;
#ifdef OLD_RENAMER
    nextFilenameBtn.setBounds(renameAreaTop.removeFromRight(thirdNospace / 2));
    fileCounterLabel.setBounds(renameAreaTop.removeFromRight(thirdNospace));
    prevFilenameBtn.setBounds(renameAreaTop.removeFromRight(thirdNospace / 2));
#endif

    // auto statusStrip = renameAreaBottom.removeFromBottom(25);
    // renamerStatus.setBounds(statusStrip.removeFromLeft(area.getWidth()*0.75f));

    renamerLed.setBounds(renameAreaBottom.removeFromLeft(20));
#ifdef OLD_RENAMER
    renameBtn.setBounds(renameAreaBottom.removeFromLeft(thirdNospace - 20));
    filename.setBounds(renameAreaBottom.removeFromLeft(thirdNospace * 2));
#endif
}

void MainComponent::sliderValueChanged(juce::Slider *slider) {
    if (slider == &gainSlider) {
        oscSender3_toSushi.send("/parameter/emotionclassifier/gain", (float)gainSlider.getValue());
    } else if (slider == &silenceSlider) {
        oscSender3_toSushi.send("/parameter/emotionclassifier/silenceThresh", (float)silenceSlider.getValue());
    }
}

void MainComponent::setBoardDateTime() {
    if (!isConnectionMade)
        return;
    juce::Time now = juce::Time::getCurrentTime();
    auto year = now.getYear();
    auto month = now.getMonth() + 1;
    auto day = now.getDayOfMonth();

    auto hour = now.getHours();
    auto minute = now.getMinutes();
    auto second = now.getSeconds();

    oscSender1_toserver.send("/setDate", year, month, day, hour, minute, second);
    buttonClicked(&termBtn);
    dateSetOnce = true;
}

void MainComponent::buttonClicked(juce::Button *button) {
    if (button == &startBtn) {
        std::cout << "start\n";
        oscSender3_toSushi.send("/parameter/emotionclassifier/recstate", (float)1.0f);
        returnedEmotion.set(0);
        resultErrorLabel.setText("", juce::dontSendNotification);
        // resultErrorLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    } else if (button == &stopBtn) {
        std::cout << "stop\n";
        oscSender3_toSushi.send("/parameter/emotionclassifier/recstate", (float)0.0f);
        setRecCommandsEnabled(false);
    } else if (button == &connectBtn) {
        if (isConnectionMade) {
            setIPFieldsEnabled(true);
            setWorkingCommandsEnabled(false);
            isConnectionMade = false;

            oscSender1_toserver.send("/stopPlugin");  // Send

            oscSender2_toPlugin.send("/disconnect");  // Send
            oscSender1_toserver.send("/disconnect");  // Send

            oscSender1_toserver.disconnect();
            oscSender2_toPlugin.disconnect();
            oscSender3_toSushi.disconnect();

            connectBtn.setButtonText("Connect");

            termText.setText(juce::String(TERM_TXT));
            termBtn.setButtonText("Open Empty Term");

            repaint();
        } else {
            std::string controllerIpAddr = controllerIpText.getText().toStdString();
            std::string ipaddr = boardIpText.getText().toStdString();

            // Check that controller IP is one of the IPs of this computer and show warning if not so
            juce::Array<juce::IPAddress> alladdresses = juce::IPAddress::getAllAddresses();
            bool isIPofThisComputer = false;
            for (auto addr : alladdresses)
                if (addr.toString() == juce::String(controllerIpAddr))
                    isIPofThisComputer = true;
            if (!isIPofThisComputer) {
                auto result = juce::AlertWindow::showOkCancelBox(
                    juce::AlertWindow::InfoIcon,
                    "Warning",
                    "The controller IP address is not one of the addresses of this computer. This may cause problems.",
                    "Abort",
                    "Continue anyway");
                if (result == 1) {
                    controllerIpText.setText("");
                    return;
                }
            }

            std::cout << "Connecting to " << ipaddr << " on port " << OSC_SEND_PORT_1_TOSERVER << "\n";

            // std::cout << "Connecting to "<< ipaddr << " on ports " << port1 << ", " << port2 << ", and " << port3 << "\n";
            // specify here where to send OSC messages to: host URL and UDP port number
            if (!oscSender1_toserver.connect(ipaddr, OSC_SEND_PORT_1_TOSERVER))
                showConnectionErrorMessage("Error: could not connect to UDP port " + std::to_string(OSC_SEND_PORT_1_TOSERVER) + ".");
            else
                oscSender1_toserver.send("/pyshake", juce::String(ipaddr), juce::String(controllerIpAddr));  // Send handshake
            //     this->sender1Connected = true;

            this->waitForHandshakeWithServer = true;
        }

    } else if (button == &termBtn) {
        // auto res = system(termText.getText().toStdString().c_str());  // -- bash -c 'ssh -t
        // do the same but from separate thread
        std::thread t([this]() {
            auto res = system(termText.getText().toStdString().c_str());
            std::cout << "Result of term: " << res << "\n";
        });
        t.detach();

    } else if (button == &playExcerptBtn) {
        auto aggr = aggrTog.getToggleState();
        auto rel = relTog.getToggleState();
        auto hap = hapTog.getToggleState();
        auto sad = sadTog.getToggleState();

        if (!aggr && !rel && !hap && !sad) {
            std::cout << "No emotions selected\n";
            return;
        }

        dbTrack1 = emoDB->getTrackFromEmo(aggr, rel, hap, sad);
        dbTrack2 = emoDB->getTrackFromEmo(aggr, rel, hap, sad);

        std::cout << "play 2 excerpts: " << dbTrack1 << " and " << dbTrack2 << "\n";
        std::cout << "With emotions " << (aggr ? "aggr" : "") << (rel ? "rel" : "") << (hap ? "hap" : "") << (sad ? "sad" : "") << "\n";
        // audioProcessor.wavPlayer.play(dbTrack1.c_str());
        // while (audioProcessor.wavPlayer.isPlaying());
        // audioProcessor.wavPlayer.play(dbTrack2.c_str());

        std::vector<std::string> tracks = {dbTrack1, dbTrack2};
        audioProcessor.wavPlayer.play(tracks);

    } else if (button == &explorerBtn) {
        openExplorer();
    } else if (button == &panicBtn) {
        // use system to run ssh and send the command /home/mind/restart_emotion_service.sh
        std::string command = "ssh -t root@" + boardIpText.getText().toStdString() + " /home/mind/restart_emotion_service.sh";
        std::cout << "Running command: " << command << "\n";
        auto res = system(command.c_str());

    } else if (button == &ex1Btn) {
        // std::cout << "play excerpt 1\n";
        if (dbTrack1 == "")
            return;
        audioProcessor.wavPlayer.clearPlaylist();
        if (audioProcessor.wavPlayer.isPlaying() && audioProcessor.wavPlayer.getNowPlaying() == dbTrack1)
            audioProcessor.wavPlayer.stop();
        else
            audioProcessor.wavPlayer.play(dbTrack1.c_str());
    } else if (button == &ex2Btn) {
        // std::cout << "play excerpt 2\n";
        if (dbTrack2 == "")
            return;
        audioProcessor.wavPlayer.clearPlaylist();
        if (audioProcessor.wavPlayer.isPlaying() && audioProcessor.wavPlayer.getNowPlaying() == dbTrack2)
            audioProcessor.wavPlayer.stop();
        else
            audioProcessor.wavPlayer.play(dbTrack2.c_str());
    }
#ifdef OLD_RENAMER
    else if (button == &renameBtn) {
        renamerLed.reset();
        std::string strfilename = filename.getText().toStdString();
        switchToNextFilename();
        std::cout << "rename file to " << strfilename << "\n";

        if (!oscSender1_toserver.send("/rename", juce::String(strfilename)))
            showConnectionErrorMessage("Error: could not send OSC message.");
        setRecCommandsEnabled(true);
    } else if (button == &nextFilenameBtn) {
        switchToNextFilename();
    } else if (button == &prevFilenameBtn) {
        switchToPrevFilename();
    }
#endif
    else {
        std::cout << "Unknown button clicked\n";
    }
}

// std::vector<std::string> MainComponent::read_names_csv(std::string filename) {
//     // Create a vector of <string, int vector> pairs to store the result
//     std::vector<std::string> result;

//     // Create an input filestream
//     std::ifstream myFile(filename);

//     // Make sure the file is open
//     if (!myFile.is_open()) throw std::runtime_error("You should provide a file with the recording names at: " + filename);

//     // Helper vars
//     std::string line;
//     std::string val;

//     // Read the column names
//     if (!myFile.good()) {
//         throw std::runtime_error("File not good");
//     }

//     // Read data, line by line
//     while (std::getline(myFile, line)) {
//         // Create a stringstream of the current line
//         std::stringstream ss(line);
//         // Extract each integer
//         while (ss >> val) {
//             // Add the current integer to the 'colIdx' column's values vector
//             result.push_back(val);

//             // If the next token is a comma, ignore it and move on
//             if (ss.peek() == ',') ss.ignore();
//         }
//     }

//     // Close file
//     myFile.close();

//     return result;
// }

// void MainComponent::switchToNextFilename() {
//     if ((!recordingNames.empty()) && ((filenameCounter + 1) < recordingNames.size()))
//         filenameCounter++;
// #ifdef OLD_RENAMER
//     fileCounterLabel.setText(std::to_string(filenameCounter + 1) + "/" + std::to_string(recordingNames.size()), juce::dontSendNotification);
//     filename.setText(recordingNames.at(filenameCounter));
// #endif
//     setPreviewName(recordingNames.at(filenameCounter));
// }

// void MainComponent::switchToPrevFilename() {
//     if ((!recordingNames.empty()) && ((filenameCounter - 1) >= 0))
//         filenameCounter--;
// #ifdef OLD_RENAMER
//     fileCounterLabel.setText(std::to_string(filenameCounter + 1) + "/" + std::to_string(recordingNames.size()), juce::dontSendNotification);
//     filename.setText(recordingNames.at(filenameCounter));
// #endif
//     setPreviewName(recordingNames.at(filenameCounter));
// }

std::string MainComponent::getCurrentRecname() {
    if (recordingNames.empty() || (filenameCounter >= recordingNames.size()))
        return "";
    return recordingNames.at(filenameCounter);
}

void MainComponent::advanceRecname() {
    if ((!recordingNames.empty()) && ((filenameCounter + 1) < recordingNames.size()))
        filenameCounter++;
    setPreviewName(recordingNames.at(filenameCounter));
}

void MainComponent::prevRecname() {
    if ((!recordingNames.empty()) && ((filenameCounter - 1) >= 0))
        filenameCounter--;
    setPreviewName(recordingNames.at(filenameCounter));
}

void MainComponent::resetRecname() {
    filenameCounter = 0;
    setPreviewName(recordingNames.at(filenameCounter));
}

void MainComponent::openExplorer() {
    // juce::String expCommand = juce::String(EXPLORER) + juce::String(" sftp://mind@") + boardIpText.getText() +
    //                           juce::String("/udata/emoAwSMIs-recordings/unnamed/ &");

    // auto res = system(expCommand.toStdString().c_str());

    // Run command in new thread
    std::thread t([this]() {
        juce::String expCommand = juce::String(EXPLORER) + juce::String(" sftp://mind@") + boardIpText.getText() +
                                  juce::String("/udata/emoAwSMIs-recordings/unnamed/ &");

        auto res = system(expCommand.toStdString().c_str());
    });
    t.detach();
}

void MainComponent::setWorkingCommandsEnabled(bool doEnable) {
    // playExcerptBtn.setEnabled(doEnable); TODO: lock after testing
    // ex1Btn.setEnabled(doEnable);
    // ex2Btn.setEnabled(doEnable);

    setRecCommandsEnabled(doEnable);

#ifdef OLD_RENAMER
    renameBtn.setEnabled(doEnable);
#endif
    gainSlider.setEnabled(doEnable);
    silenceSlider.setEnabled(doEnable);
    meter.setEnabled(doEnable);

    gainLabel.setEnabled(doEnable);
    silenceLabel.setEnabled(doEnable);
    silenceThresholdLabel.setEnabled(doEnable);
    silenceLed.setEnabled(doEnable);
    recNameLabel.setEnabled(doEnable);
    statusLabel.setEnabled(doEnable);

    emoAggressiveLed.setEnabled(doEnable);
    emoAggressiveLabel.setEnabled(doEnable);
    emoRelaxedLed.setEnabled(doEnable);
    emoRelaxedLabel.setEnabled(doEnable);
    emoHappyLed.setEnabled(doEnable);
    emoHappyLabel.setEnabled(doEnable);
    emoSadLed.setEnabled(doEnable);
    emoSadLabel.setEnabled(doEnable);
}

void MainComponent::setIPFieldsEnabled(bool doEnable) {
    boardIpText.setEnabled(doEnable);
    controllerIpText.setEnabled(doEnable);
}

void MainComponent::setRecCommandsEnabled(bool doEnable) {
    startBtn.setEnabled(doEnable);
    startBtn.setColour(juce::TextButton::buttonColourId, doEnable ? juce::Colours::darkred : juce::Colours::grey);
    stopBtn.setEnabled(doEnable);
}

void MainComponent::setEnableExcerptButtons(bool doEnable) {
    // playExcerptBtn.setEnabled(!doEnable);
    ex1Btn.setEnabled(doEnable);
    ex2Btn.setEnabled(doEnable);
}