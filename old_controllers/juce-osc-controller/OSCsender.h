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

class Sender {
private:
    juce::String ipAddress;
    int port;

public:
    juce::OSCSender innerSender;

    Sender() {
        this->ipAddress = "";
        this->port = -1;
    }

    void sendMessage(juce::String message) {
            innerSender.send(message, (float)1.0f);
    }

    template <typename T>
    void sendMessage(juce::String message, T arg1) {
            innerSender.send(message, arg1);
    }

    // void send4Bool(juce::String message, bool arg1, bool arg2, bool arg3, bool arg4) {
    //         innerSender.send(message, (int)arg1, (int)arg2, (int)arg3, (int)arg4);
    // }

};

}  // namespace cOSC
