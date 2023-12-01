#pragma once

#include <JuceHeader.h>

// #define USE_GRILL

namespace Gui {
class LevelMeter : public juce::Component, juce::Timer {
public:
    LevelMeter(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction)) {
        setEnabled(true);
#ifdef USE_GRILL
        grill = ImageCache::getFromMemory(BinaryData::MeterGrill_png, BinaryData::MeterGrill_pngSize);
#endif
    }

    void mouseDown(const juce::MouseEvent& event) override {
        clipped = false;
    }

    void paint(juce::Graphics& g) override {
        const float MIN_DB = -60.f,
                    MAX_DB = 6.f,
                    MIN_HEIGHT = 0.f,
                    MAX_HEIGHT = static_cast<float>(getHeight());
        const float zeroPoint = juce::jmap(0.f, MIN_DB, MAX_DB, MIN_HEIGHT, MAX_HEIGHT);

        auto bounds = getLocalBounds().toFloat().reduced(3.f);
        auto levelticksArea = bounds.removeFromRight(30.f);

        // Draw ticks at MAX_DB, 0.0 and MIN_DB
        // drawText (const String &text, int x, int y, int width, int height, Justification justificationType, bool useEllipsesIfTooBig=true) const
        const float minTickOffset = 5.f, textHeight = 5.f;
        g.setColour(textColour);
        g.drawLine(levelticksArea.getX() + 1, bounds.getY(), levelticksArea.getX() + 5, bounds.getY());
        g.drawLine(levelticksArea.getX() + 1, bounds.getY() + MAX_HEIGHT - zeroPoint, levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - zeroPoint);
        g.drawLine(levelticksArea.getX() + 1, bounds.getY() + MAX_HEIGHT - minTickOffset, levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - minTickOffset);
        g.drawText(juce::String(MAX_DB) + " dB", levelticksArea.getX() + 5, bounds.getY(), levelticksArea.getWidth(), textHeight, juce::Justification::centredLeft, false);
        g.drawText(juce::String(0) + " dB", levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - zeroPoint, levelticksArea.getWidth(), textHeight, juce::Justification::centredLeft, false);
        g.drawText(juce::String(MIN_DB) + " dB", levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - minTickOffset * 3, levelticksArea.getWidth(), textHeight, juce::Justification::centredLeft, false);

        g.setColour(backColour);
        g.fillRect(bounds);

        g.setGradientFill(gradient);

        float value =  valueSupplier();
        if (this->setToClear.exchange(false)) {
            value = MIN_DB;
        }
        const auto scaLevelMeterY = juce::jmap(value, MIN_DB, MAX_DB, MIN_HEIGHT, MAX_HEIGHT);
        g.fillRect(bounds.removeFromBottom(scaLevelMeterY));


        if (value >= 0.f && !setToClear.load()) {
            clipped = true;
        }

        if (clipped) {
            g.setColour(juce::Colours::red);
            g.fillRect(bounds.removeFromTop(MAX_HEIGHT - zeroPoint));
        }
    }

    void resized() override {
        const auto bounds = getLocalBounds().toFloat();
        gradient = juce::ColourGradient{juce::Colours::green, bounds.getBottomLeft(), juce::Colours::red, bounds.getTopLeft(), false};
        gradient.addColour(0.5, juce::Colours::yellow);
    }

    void paintOverChildren(juce::Graphics& g) override {
#ifdef USE_GRILL
        g.drawImage(grill, getLocalBounds().toFloat());
#endif
    }

    void timerCallback() override {
        repaint();
    }

    void setEnabled(bool enabled) {
        if (enabled) {
            startTimerHz(24);
            textColour = juce::Colours::white;
            backColour = juce::Colours::black;
        } else {
            stopTimer();
            this->clear();
            textColour = juce::Colours::grey;
            backColour = juce::Colours::darkgrey;
        }

    }

    void clear() {
        setToClear.store(true);
    }

private:
    std::function<float()> valueSupplier;
    juce::ColourGradient gradient{};
    juce::Image grill;
    bool clipped = false;
    std::atomic<bool> setToClear{false};

    juce::Colour textColour{juce::Colours::white},
                backColour{juce::Colours::black};

};
}  // namespace Gui