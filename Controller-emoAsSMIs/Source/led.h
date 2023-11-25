#pragma once

#include <JuceHeader.h>

namespace Gui {
class Led : public juce::Component, public juce::Timer {
    juce::String text = "";

public:
    Led(std::function<bool()>&& valueFunction) : valueSupplier(std::move(valueFunction)) {
        setEnabled(true);
    }

    Led(juce::String text, std::function<bool()>&& valueFunction) : text(text), valueSupplier(std::move(valueFunction)) {
        setEnabled(true);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll((valueSupplier()) ? juce::Colours::orange : juce::Colours::grey);

        g.setColour(borderColour);
        g.drawRect(getLocalBounds(), 1);  // draw an outline around the component

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);

        g.drawFittedText(this->text, getLocalBounds(), juce::Justification::centred, 1);
    }
    void resized() override {
        repaint();
    }

    void timerCallback() override {
        repaint();
    }

    void setEnabled(bool enabled) {
        if (enabled) {
            startTimerHz(24);
            borderColour = juce::Colours::black;
        } else {
            stopTimer();
            borderColour = juce::Colours::darkgrey;
        }
    }

private:
    std::function<bool()> valueSupplier;

    juce::Colour borderColour{juce::Colours::black};
};

class SimpleLed : public juce::Component, public juce::Timer {
    // juce::String text = "";
    juce::Colour colorON = juce::Colours::orange,
                 colorOFF = juce::Colours::grey;
    bool state = false;

public:
    SimpleLed() : valueSupplier() {
        setEnabled(true);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(state ? colorON : colorOFF);

        g.setColour(borderColour);
        g.drawRect(getLocalBounds(), 1);
    }
    void resized() override {
        repaint();
    }

    void timerCallback() override {
        repaint();
    }

    void set(bool state) {
        this->state = state;
    }

    void setColor (juce::Colour color) {
        this->colorON = color;
    }

    void reset(){
        state = false;
        repaint();
    }

    void turnOn(juce::Colour color){
        setColor(color);
        set(true);
    }

    void setEnabled(bool enabled) {
        if (enabled) {
            startTimerHz(24);
            borderColour = juce::Colours::black;
        } else {
            stopTimer();
            borderColour = juce::Colours::darkgrey;
        }
    }

private:
    std::function<bool()> valueSupplier;
    juce::Colour borderColour{juce::Colours::black};
};

}  // namespace Gui