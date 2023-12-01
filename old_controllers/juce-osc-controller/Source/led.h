#pragma once

#include <JuceHeader.h>

namespace Gui {
class Led : public Component, public Timer {
    juce::String text = "";

public:
    Led(std::function<bool()>&& valueFunction) : valueSupplier(std::move(valueFunction)) {
        startTimerHz(24);
    }

    Led(juce::String text, std::function<bool()>&& valueFunction) : text(text), valueSupplier(std::move(valueFunction)) {
        startTimerHz(24);
    }

    void paint(Graphics& g) override {
        g.fillAll((valueSupplier()) ? Colours::orange : Colours::grey);

        g.setColour(Colours::black);
        g.drawRect(getLocalBounds(), 1);  // draw an outline around the component

        g.setColour(Colours::white);
        g.setFont(14.0f);

        g.drawFittedText(this->text, getLocalBounds(), Justification::centred, 1);
    }
    void resized() override {
        repaint();
    }

    void timerCallback() override {
        repaint();
    }

private:
    std::function<bool()> valueSupplier;
};

class SimpleLed : public Component, public Timer {
    // juce::String text = "";
    juce::Colour colorON = Colours::orange,
                 colorOFF = Colours::grey;
    bool state = false;

public:
    SimpleLed() : valueSupplier() {
        startTimerHz(24);
    }

    void paint(Graphics& g) override {
        g.fillAll(state ? colorON : colorOFF);

        g.setColour(Colours::black);
        g.drawRect(getLocalBounds(), 1);  // draw an outline around the component

        // g.setColour(Colours::white);
        // g.setFont(14.0f);

        // g.drawFittedText(this->text, getLocalBounds(), Justification::centred, 1);
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



private:
    std::function<bool()> valueSupplier;
};

}  // namespace Gui