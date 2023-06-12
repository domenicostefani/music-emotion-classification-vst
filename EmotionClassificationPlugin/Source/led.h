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

    void paint (Graphics& g) override
    {
        std::cout << "valueSupplier(): " << valueSupplier() << std::endl;
        g.fillAll ((valueSupplier())?Colours::orange:Colours::grey);

        g.setColour (Colours::black);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (Colours::white);
        g.setFont (14.0f);

        g.drawFittedText (this->text, getLocalBounds(), Justification::centred, 1);
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
}  // namespace Gui