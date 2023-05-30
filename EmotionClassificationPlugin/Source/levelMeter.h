#pragma once

#include <JuceHeader.h>

// #define USE_GRILL

namespace Gui {
class VerticalGradientMeter : public Component, public Timer {
public:
    VerticalGradientMeter(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction)) {
        startTimerHz(24);
#ifdef USE_GRILL
        grill = ImageCache::getFromMemory(BinaryData::MeterGrill_png, BinaryData::MeterGrill_pngSize);
#endif
    }

    void mouseDown(const MouseEvent& event) override {
        clipped = false;
    }

    void paint(Graphics& g) override {
        const float MIN_DB = -60.f,
                    MAX_DB = 6.f,
                    MIN_HEIGHT = 0.f,
                    MAX_HEIGHT = static_cast<float>(getHeight());
        const float zeroPoint = jmap(0.f, MIN_DB, MAX_DB, MIN_HEIGHT, MAX_HEIGHT);

        auto bounds = getLocalBounds().toFloat().reduced(3.f);
        auto levelticksArea = bounds.removeFromRight(30.f);

        // Draw ticks at MAX_DB, 0.0 and MIN_DB
        // drawText (const String &text, int x, int y, int width, int height, Justification justificationType, bool useEllipsesIfTooBig=true) const
        const float minTickOffset = 5.f, textHeight = 5.f;
        g.setColour(Colours::white);
        g.drawLine(levelticksArea.getX() + 1, bounds.getY(), levelticksArea.getX() + 5, bounds.getY());
        g.drawLine(levelticksArea.getX() + 1, bounds.getY() + MAX_HEIGHT - zeroPoint, levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - zeroPoint);
        g.drawLine(levelticksArea.getX() + 1, bounds.getY() + MAX_HEIGHT-minTickOffset, levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT-minTickOffset);
        g.drawText(String(MAX_DB) + " dB", levelticksArea.getX() + 5, bounds.getY(), levelticksArea.getWidth(), textHeight, Justification::centredLeft, false);
        g.drawText(String(0) + " dB",      levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT - zeroPoint, levelticksArea.getWidth(), textHeight, Justification::centredLeft, false);
        g.drawText(String(MIN_DB) + " dB", levelticksArea.getX() + 5, bounds.getY() + MAX_HEIGHT-minTickOffset*3, levelticksArea.getWidth(), textHeight, Justification::centredLeft, false);


        g.setColour(Colours::black);
        g.fillRect(bounds);

        g.setGradientFill(gradient);
        const auto scaledY = jmap(valueSupplier(), MIN_DB, MAX_DB, MIN_HEIGHT, MAX_HEIGHT);
        g.fillRect(bounds.removeFromBottom(scaledY));

        

        // If valueSupplier clips (> 0.0) paint permanently the part between 0 and 6 dB

        if (valueSupplier() >= 0.f) {
            clipped = true;
        }

        if (clipped) {
            g.setColour(Colours::red);
            g.fillRect(bounds.removeFromTop(MAX_HEIGHT - zeroPoint));
        }
    }

    void resized() override {
        const auto bounds = getLocalBounds().toFloat();
        gradient = ColourGradient{Colours::green, bounds.getBottomLeft(), Colours::red, bounds.getTopLeft(), false};
        gradient.addColour(0.5, Colours::yellow);
    }

    void paintOverChildren(::juce::Graphics& g) override {
#ifdef USE_GRILL
        g.drawImage(grill, getLocalBounds().toFloat());
#endif
    }

    void timerCallback() override {
        repaint();
    }

private:
    std::function<float()> valueSupplier;
    ColourGradient gradient{};
    Image grill;
    bool clipped = false;
};
}  // namespace Gui