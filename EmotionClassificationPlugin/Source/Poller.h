
#pragma once

#include "JuceHeader.h"

class Poller : public juce::Timer {
private:
    std::function<void()> subroutine;

public:
    Poller(int hz, std::function<void()>&& subroutine) : subroutine(std::move(subroutine)) {
        startTimerHz(hz);
    }

    Poller(std::function<void()>&& subroutine) : subroutine(std::move(subroutine)) {
        startTimerHz(10);
    }

    void timerCallback() override {
        subroutine();
    }
};
