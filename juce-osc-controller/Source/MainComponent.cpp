/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

#include <string>
#include <fstream>
#include <vector>
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream


float MainComponent::getOscInLevel() const {
    return -60.0f; // TODO fix
}
bool MainComponent::getOscIsSilent() const {
    return false; // TODO fix
}


void MainComponent::showStatus(int status) {
    std::string statusStr = "Status: ";

    switch (status) {
        case 0:
            statusStr += "Disconnected";
            break;
        case 1:
            statusStr += "Idle";
            break;
        case 2:
            statusStr += "Recording";
            break;
        case 3:
            statusStr += "Classifying";
            break;
        default:
            statusStr += "Unknown";
            break;
    }

    statusLabel.setText("Status: Idle",dontSendNotification);
}

void MainComponent::oscMessageReceived(const juce::OSCMessage &message) {
    std::cout << "Received message " << message.getAddressPattern().toString().toStdString() << std::endl;


    if (message.getAddressPattern() == juce::OSCAddressPattern("/pyshake")) {
        if (!this->waitForHandshakeWithServer) {
            std::cout << "Received unwanted handshake from SERVER" << std::endl;
        } else {
            std::cout << "-> Handshake with server received!" << std::endl;
            // this->setButtonsEnabled(true);
            oscSender1_toserver.send ("/startPlugin", juce::String(INSTRUMENT)); //Send handshake
            waitForHandshakeWithServer = false;
            waitForHandshakeWithPlugin = true;
            // Now keep sending the plugin handshake until we get a reply

            if (! oscSender2_toPlugin.connect (boardIpText.getText().toStdString(), OSC_SEND_PORT_2_TOPLUGIN))
                showConnectionErrorMessage ("Error: could not connect to UDP port "+std::to_string(OSC_SEND_PORT_2_TOPLUGIN)+".");
            else
                pluginHandshakePoller.startPolling(200); // Send every 200ms

        }

    } else if (message.getAddressPattern() == juce::OSCAddressPattern("/handshake")) {
        if (!this->waitForHandshakeWithPlugin) {
            std::cout << "Received unwanted handshake from PLUGIN" << std::endl;
        } else {
            std::cout << "-> Handshake with plugin received!" << std::endl;
            // this->setButtonsEnabled(true);
            waitForHandshakeWithPlugin = false;
            pluginHandshakePoller.stopPolling();
            isConnectionMade = true;
            setIPFieldsEnabled(false);
            connectBtn.setButtonText("Disconnect");
            repaint();
        }

    } 
    
    // else if (message.getAddressPattern() == juce::OSCAddressPattern("/state")) {
    //     std::cout << "Received state" << std::endl;
    //     if (message.size() == 1 && message[0].isInt32()) {
    //         std::cout << "-> state: " << message[0].getInt32() << std::endl;
    //         showStatus(message[0].getInt32()+1);
    //     }

    // } else if (message.getAddressPattern() == juce::OSCAddressPattern("/meter")) {

    // } else if (message.getAddressPattern() == juce::OSCAddressPattern("/silence")) {

    // } else if (message.getAddressPattern() == juce::OSCAddressPattern("/emotion")) {
    //     std::cout << "Received emotion result" << std::endl;
    //     if (message.size() == 1 && message[0].isInt32()) {
    //         std::cout << "-> emotion: " << message[0].getInt32() << std::endl;
    //         returnedEmotion.set(message[0].getInt32());
    //         emoAggressiveLed.repaint();
    //         emoRelaxedLed.repaint();
    //         emoHappyLed.repaint();
    //         emoSadLed.repaint();
    //     }
    // } else {
    //     std::cout << "Received unknown message" << std::endl;
    // }

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
MainComponent::MainComponent():
    meter([&]() { return this->getOscInLevel(); }),
    silenceLed([&]() { return !this->getOscIsSilent(); }),
    emoAggressiveLed([&]() { return this->returnedEmotion.getAggressive();}), 
    emoRelaxedLed(   [&]() { return this->returnedEmotion.getRelaxed();}), 
    emoHappyLed(     [&]() { return this->returnedEmotion.getHappy();}), 
    emoSadLed(       [&]() { return this->returnedEmotion.getSad();}),
    oscReceiverFromServer(RX_PORT_SERVER, [&](const juce::OSCMessage &m) { this->oscMessageReceived(m); }),
    oscReceiverFromPlugin(RX_PORT_PLUGIN, [&](const juce::OSCMessage &m) { this->oscMessageReceived(m); })
{
    setSize (600, 700);

    addAndMakeVisible(connectBtn);
    connectBtn.setButtonText("Connect");
    connectBtn.addListener(this);

    addAndMakeVisible(boardIpLabel);
    boardIpLabel.setJustificationType(Justification::centred);
    boardIpLabel.setText("Board IP",dontSendNotification);
    addAndMakeVisible(controllerIpLabel);
    controllerIpLabel.setJustificationType(Justification::centred);
    controllerIpLabel.setText("Controller IP",dontSendNotification);


    addAndMakeVisible(boardIpText);
    boardIpText.setText("elk-pi.local");
    addAndMakeVisible(controllerIpText);

    // Array< IPAddress > alladdresses = IPAddress::getAllAddresses();

    // for (auto addr: alladdresses){
    //     std::cout << ":" << addr.toString() << "\n";
    // }

    controllerIpText.setText("10.42.0.47");

    std::vector<juce::String> oscMessagesServer = {"/pyshake"};
    for (const auto& oscMessage : oscMessagesServer)
        oscReceiverFromServer.addOSCListener(oscMessage);

    std::vector<juce::String> oscMessagesPlugin = {"/handshake","/state","/meter","/silence","/emotion"};
    for (const auto& oscMessage : oscMessagesPlugin)
        oscReceiverFromPlugin.addOSCListener(oscMessage);



    addAndMakeVisible(gainSlider);
    gainSlider.setRange(0.0, 1.0);
    gainSlider.setValue(1.0);
    gainSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    gainSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);


    addAndMakeVisible(silenceSlider);
    silenceSlider.setRange(0.0, 1.0);
    silenceSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    silenceSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);


    addAndMakeVisible(gainLabel);
    gainLabel.setJustificationType(Justification::centred);
    gainLabel.setText("Gain",dontSendNotification);

    addAndMakeVisible(silenceLabel);
    silenceLabel.setJustificationType(Justification::centred);
    silenceLabel.setText("Sound Detected",dontSendNotification);

    addAndMakeVisible(silenceThresholdLabel);
    silenceThresholdLabel.setJustificationType(Justification::centred);
    silenceThresholdLabel.setText("Silence Threshold",dontSendNotification);


    addAndMakeVisible(meter);
    addAndMakeVisible(silenceLed);





    addAndMakeVisible(emoAggressiveLabel);
    emoAggressiveLabel.setText("Aggressive", dontSendNotification);
    emoAggressiveLabel.setJustificationType(Justification::centred);
    emoAggressiveLabel.setColour (juce::Label::textColourId, Colours::black);   

    addAndMakeVisible(emoRelaxedLabel);
    emoRelaxedLabel.setText("Relaxed", dontSendNotification);
    emoRelaxedLabel.setJustificationType(Justification::centred);
    emoRelaxedLabel.setColour (juce::Label::textColourId, Colours::black);

    addAndMakeVisible(emoHappyLabel);
    emoHappyLabel.setText("Happy", dontSendNotification);
    emoHappyLabel.setJustificationType(Justification::centred);
    emoHappyLabel.setColour (juce::Label::textColourId, Colours::black);

    addAndMakeVisible(emoSadLabel);
    emoSadLabel.setText("Sad", dontSendNotification);
    emoSadLabel.setJustificationType(Justification::centred);
    emoSadLabel.setColour (juce::Label::textColourId, Colours::black);



    
    addAndMakeVisible(emoAggressiveLed);
    addAndMakeVisible(emoRelaxedLed);
    addAndMakeVisible(emoHappyLed);
    addAndMakeVisible(emoSadLed);
    addAndMakeVisible(resultErrorLabel);
    

    addAndMakeVisible(playExcerptBtn);
    playExcerptBtn.setButtonText("Play Excerpt with classified emotions");






    addAndMakeVisible(startBtn);
    startBtn.setButtonText("Start");
    startBtn.addListener(this);

    addAndMakeVisible(stopBtn);
    stopBtn.setButtonText("Stop");
    stopBtn.addListener(this);

    addAndMakeVisible(statusLabel);
    // statusLabel.setText("Status: Idle",dontSendNotification);
    showStatus(0);






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
    fileCounterLabel.setText("0/0",dontSendNotification);


    addAndMakeVisible(filename);
    filename.setText("");

    std::string csvPath = "./rec_filenames.csv";
    readnames = read_names_csv(csvPath);
    switchToNextFilename();


    setButtonsEnabled(false);
}

MainComponent::~MainComponent()
{
    oscSender1_toserver.disconnect();
    oscSender2_toPlugin.disconnect();
    recstateSender.disconnect();
    renamingSender.disconnect();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.setColour ((isConnectionMade)?Colours::darkgreen : Colours::red);
    g.fillRect (wholearea);
    
    g.setColour (juce::Colours::grey);
    g.fillRect (basearea);

    g.setColour (juce::Colours::lightgrey);
    g.fillRect (origEmoArea);
    
    g.setColour (juce::Colours::grey);
    g.fillRect (resultStatusArea);
}

void MainComponent::resized()
{
    const int SLIDER_WIDTH = 60;
    const int METER_WIDTH = 60;
    const int LED_WIDTH = 20;

    int vSeparator = 20;
    wholearea = getLocalBounds();
    basearea = wholearea.reduced(15);
    
    auto area = basearea.reduced(10);
    Rectangle<int> top = area.removeFromTop(50);
    area.removeFromTop(vSeparator);

    int vIntervals = 5;
    int verticalInterval = (area.getHeight()- vSeparator*(vIntervals-1))/vIntervals;
    Rectangle<int> volumesArea = area.removeFromTop(verticalInterval*2.1);
    area.removeFromTop(vSeparator);
    Rectangle<int> playStopStatusArea = area.removeFromTop(verticalInterval*0.5);
    area.removeFromTop(vSeparator);
    Rectangle<int> emoArea = area.removeFromTop(verticalInterval*1.4);
    repaint();

    area.removeFromTop(vSeparator);
    Rectangle<int> databaseArea = area.removeFromTop(verticalInterval*0.5);
    area.removeFromTop(vSeparator);
    Rectangle<int> renameArea = area;

    int hSeparator = 15, blocks = 3;
    int horBlock = (area.getWidth()-(hSeparator*(blocks-1)))/ blocks;

    auto labelStrip = top.removeFromTop(top.getHeight()/2);
    boardIpLabel.setBounds(labelStrip.removeFromLeft(horBlock));
    labelStrip.removeFromLeft(hSeparator);
    controllerIpLabel.setBounds(labelStrip.removeFromLeft(horBlock));

    
    auto connectInputStrip = top;

    boardIpText.setBounds(connectInputStrip.removeFromLeft(horBlock));
    connectInputStrip.removeFromLeft(hSeparator);
    controllerIpText.setBounds(connectInputStrip.removeFromLeft(horBlock));
    connectInputStrip.removeFromLeft(hSeparator);
    connectBtn.setBounds(connectInputStrip);



    auto gainControlArea = volumesArea.removeFromLeft(horBlock);
    gainLabel.setBounds(gainControlArea.removeFromTop(20));
    int toremove = (horBlock-SLIDER_WIDTH)/2;
    gainControlArea.removeFromLeft(toremove); gainControlArea.removeFromRight(toremove);
    gainSlider.setBounds(gainControlArea);

    auto levelMeterArea = volumesArea.removeFromLeft(horBlock);
    int meterToremove = (horBlock-METER_WIDTH)/2;
    levelMeterArea.removeFromLeft(meterToremove); levelMeterArea.removeFromRight(meterToremove);
    meter.setBounds(levelMeterArea);

    auto silenceControlArea = volumesArea;
    silenceLabel.setBounds(silenceControlArea.removeFromTop(20));
    int toremoveLedsides = (horBlock-LED_WIDTH)/2;
    auto ledArea = silenceControlArea.removeFromTop(20);
    ledArea.removeFromLeft(toremoveLedsides);
    ledArea.removeFromRight(toremoveLedsides);
    silenceLed.setBounds(ledArea);
    silenceControlArea.removeFromLeft(toremove); silenceControlArea.removeFromRight(toremove);
    silenceThresholdLabel.setBounds(silenceControlArea.removeFromTop(20));
    silenceSlider.setBounds(silenceControlArea);

    // playStopStatusArea

    startBtn.setBounds(playStopStatusArea.removeFromLeft(horBlock).reduced(5));
    stopBtn.setBounds(playStopStatusArea.removeFromLeft(horBlock).reduced(5));
    statusLabel.setBounds(playStopStatusArea);



    // Emotion area

    resultStatusArea=emoArea.removeFromBottom(15);
    resultErrorLabel.setBounds(resultStatusArea);
    origEmoArea = emoArea;

    int hfourth = (emoArea.getWidth()-(hSeparator*(4-1)))/ 4;
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

    playExcerptBtn.setBounds(databaseArea.reduced(10));


    // rename area


    auto renameAreaTop = renameArea.removeFromTop(renameArea.getHeight()/2);
    auto renameAreaBottom = renameArea;

    
    int thirdNospace = area.getWidth()/3;
    nextFilenameBtn.setBounds(renameAreaTop.removeFromRight(thirdNospace/2));
    fileCounterLabel.setBounds(renameAreaTop.removeFromRight(thirdNospace));
    prevFilenameBtn.setBounds(renameAreaTop.removeFromRight(thirdNospace/2));
    renameBtn.setBounds(renameAreaBottom.removeFromLeft(thirdNospace));
    filename.setBounds(renameAreaBottom.removeFromLeft(thirdNospace*2));

}

void MainComponent::buttonClicked (Button *button)
{
    if(button == &startBtn)
    {
        std::cout << "start\n";
        if (! recstateSender.send ("/parameter/audiorecordervst/recstate", (float) 1.0f))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    }
    else if(button == &stopBtn)
    {
        std::cout << "stop\n";
        if (! recstateSender.send ("/parameter/audiorecordervst/recstate", (float) 0.0f))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    }
    else if(button == &connectBtn)
    {
        if (isConnectionMade){
            // TODO
            setIPFieldsEnabled(true);
            isConnectionMade = false;

            oscSender2_toPlugin.send ("/disconnect"); //Send
            oscSender1_toserver.send ("/disconnect"); //Send

            oscSender1_toserver.disconnect();
            oscSender2_toPlugin.disconnect();

            connectBtn.setButtonText("Connect");
            repaint();
        } else {
            std::string controllerIpAddr = controllerIpText.getText().toStdString();
            std::string ipaddr = boardIpText.getText().toStdString();
            // int port2 = OSC_SEND_PORT_2_TOPLUGIN;
            // int port3 = OSC_SEND_PORT_3_TOSUSHI;

            std::cout << "Connecting to "<< ipaddr << " on port " << OSC_SEND_PORT_1_TOSERVER << "\n";
            

            // std::cout << "Connecting to "<< ipaddr << " on ports " << port1 << ", " << port2 << ", and " << port3 << "\n";
            // specify here where to send OSC messages to: host URL and UDP port number
            if (! oscSender1_toserver.connect (ipaddr, OSC_SEND_PORT_1_TOSERVER))
                showConnectionErrorMessage ("Error: could not connect to UDP port "+std::to_string(OSC_SEND_PORT_1_TOSERVER)+".");
            else
                oscSender1_toserver.send ("/pyshake", juce::String(ipaddr), juce::String(controllerIpAddr)); //Send handshake
            //     this->sender1Connected = true;

            this->waitForHandshakeWithServer = true;
        }

    }
    else if(button == &renameBtn)
    {
        std::string strfilename = filename.getText().toStdString();
        switchToNextFilename();
        std::cout << "rename file to "<< strfilename <<"\n";

        if (! renamingSender.send ("/rename", juce::String(strfilename)))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    }
    else if(button == &nextFilenameBtn)
    {
        switchToNextFilename();
    }
    else if(button == &prevFilenameBtn)
    {
        switchToPrevFilename();
    }
}


std::vector<std::string> MainComponent::read_names_csv(std::string filename){

    // Create a vector of <string, int vector> pairs to store the result
    std::vector<std::string> result;

    // Create an input filestream
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("You should provide a file with the recording names at: "+filename);

    // Helper vars
    std::string line;
    std::string val;

    // Read the column names
    if(! myFile.good())
    {
        throw std::runtime_error("File not good");
    }

    // Read data, line by line
    while(std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);
        // Extract each integer
        while(ss >> val){

            // Add the current integer to the 'colIdx' column's values vector
            result.push_back(val);

            // If the next token is a comma, ignore it and move on
            if(ss.peek() == ',') ss.ignore();
        }
    }

    // Close file
    myFile.close();

    return result;
}

void MainComponent::switchToNextFilename()
{
    if((!readnames.empty()) && ((filenameCounter+1) < readnames.size()))
        filenameCounter++;
    fileCounterLabel.setText(std::to_string(filenameCounter+1)+"/"+std::to_string(readnames.size()),dontSendNotification);
    filename.setText(readnames.at(filenameCounter));
}

void MainComponent::switchToPrevFilename()
{
    if((!readnames.empty()) && ((filenameCounter-1) >= 0))
        filenameCounter--;
    fileCounterLabel.setText(std::to_string(filenameCounter+1)+"/"+std::to_string(readnames.size()),dontSendNotification);
    filename.setText(readnames.at(filenameCounter));
}

void MainComponent::setButtonsEnabled(bool enable){
    playExcerptBtn.setEnabled(enable);
    startBtn.setEnabled(enable);
    stopBtn.setEnabled(enable);
    renameBtn.setEnabled(enable);
}

void MainComponent::setIPFieldsEnabled(bool enable){
    boardIpText.setEnabled(enable);
    controllerIpText.setEnabled(enable);
}
