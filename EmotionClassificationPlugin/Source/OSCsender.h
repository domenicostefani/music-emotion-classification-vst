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

class HandshakedSender {
private:
    const juce::String HANDSHAKE_MESSAGE = "/handshake";
    bool enabled = false;  // Enable only when a handshake request is received
    juce::String ipAddress;
    int port;

public:
    juce::OSCSender innerSender;
    
    HandshakedSender() {
        this->enabled = false;
        this->ipAddress = "";
        this->port = -1;
    }

    void enableAndReplyToHandshake(juce::String ipAddress, int port) {
        if (innerSender.connect(ipAddress, port)) {
            enabled = true;
            this->ipAddress = ipAddress;
            this->port = port;
            sendMessage(HANDSHAKE_MESSAGE);
            std::cout << "Handshake sent to " << ipAddress << ":" << port << std::endl;
        }
    }

    void sendMessage(juce::String message) {
        if (enabled) {
            innerSender.send(message, (float)1.0f);
        }
    }

    template <typename T>
    void sendMessage(juce::String message, T arg1) {
        if (enabled) {
            innerSender.send(message, arg1);
        }
    }

    void send4Bool(juce::String message, bool arg1, bool arg2, bool arg3, bool arg4) {
        if (enabled) {
            innerSender.send(message, (int)arg1, (int)arg2, (int)arg3, (int)arg4);
        }
    }
    
    void disable () {
        enabled = false;
    }
};

}  // namespace cOSC
