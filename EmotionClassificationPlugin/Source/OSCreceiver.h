/**
 * @file OSCsender.h
 * @author your name (you@domain.com)
 * @brief This contans all the code needed for an handshaked UDP OSC communication from a plugin to a controller
 * @version 0.1
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <JuceHeader.h>

namespace cOSC {

class Receiver : public juce::OSCReceiver, public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>{
private:
    int port;
    std::function<void(const juce::OSCMessage&)> callback;

public:
    Receiver(int port, std::function<void(const juce::OSCMessage& message)>&& oscMessageReceived) : 
            callback(std::move(oscMessageReceived)) {
        this->port = port;
        connect(port);
    }

    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        callback(message);
    }

    void addOSCListener(juce::String oscMessage) {
        addListener (this, oscMessage);
    }

};

}  // namespace cOSC
