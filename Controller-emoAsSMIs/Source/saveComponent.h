#pragma once

#include <JuceHeader.h>

typedef std::function<void(std::string)> SvCallback;
typedef std::function<void()> SvCallbackRoutine;

class SaveComponent : public juce::Component
// , juce::Timer {
{
public:
    //==============================================================================
    SaveComponent(juce::String saveName, SvCallback&& saveAsp, SvCallbackRoutine&& trashp, SvCallbackRoutine&& advanceRecname)
        : saveAs(std::move(saveAsp)),
          trash(std::move(trashp)),
          advanceRecname(std::move(advanceRecname)),
          defSaveName(saveName) {
        // startTimer(24);

        DBG("SaveComponent::SaveComponent() called");
        if (defSaveName != "") {
            DBG("SaveComponent::SaveComponent() called with saveName = " + saveName);
            addAndMakeVisible(saveAsPrepname);
            saveAsPrepname.setButtonText("Save as\n" + saveName);
            saveAsPrepname.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgreen);
            saveAsPrepname.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
            saveAsPrepname.onClick = [&] {
                this->saveAs(this->defSaveName.toStdString());
                advanceRecname();
                this->close();
            };
        } else {
            defSaveName = "test.wav";
        }

        addAndMakeVisible(SaveAsCustom);
        SaveAsCustom.setButtonText("Save as Custom Name");
        SaveAsCustom.onClick = [&] {
            juce::String newname = customName.getText();
            if (newname.isEmpty()) {
                newname = defSaveName;
                advanceRecname();
            }
            if (!newname.endsWith(".wav"))
                newname += ".wav";
            this->saveAs(newname.toStdString());
            close();
        };

        addAndMakeVisible(customName);
        customName.setMultiLine(false);
        customName.setReturnKeyStartsNewLine(false);
        customName.setTextToShowWhenEmpty(defSaveName, juce::Colours::grey);

        addAndMakeVisible(moveToTrash);
        moveToTrash.setButtonText("Move to Trash");
        moveToTrash.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);
        moveToTrash.onClick = [&] {
            trash();
            close();
        };
    }
    ~SaveComponent() = default;

    //==============================================================================
    void paint(juce::Graphics& g) override {
        // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.fillAll(juce::Colour(0xff445258));
    }

    void resized() override {
        auto area = getLocalBounds();
        auto redarea = area.reduced(20);
        int sep = 20;
        int thirdwidth = (redarea.getWidth() - 2 * sep) / 3;
        int halfheight = (redarea.getHeight() - sep) / 2;

        auto a1 = redarea.removeFromLeft(thirdwidth);
        redarea.removeFromLeft(sep);
        auto a2 = redarea.removeFromLeft(thirdwidth);
        redarea.removeFromLeft(sep);
        auto a3 = redarea;

        saveAsPrepname.setBounds(a1);

        SaveAsCustom.setBounds(a2.removeFromTop(halfheight));
        a2.removeFromTop(sep);
        customName.setBounds(a2);

        moveToTrash.setBounds(a3);
    }

    // void timerCallback() override {
    //     // if (juce::DialogWindow* dw = this->findParentComponentOfClass<juce::DialogWindow>())
    //     //     dw->setTitleBarButtonsRequired(0);
    //     repaint();
    // }

    void close() {
        if (juce::DialogWindow* dw = this->findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);

        // if (buttonThatWasClicked == closeButton)
        //     {
        //     contentComponent->findParentComponentOfClass ((DialogWindow*) 0)->exitModalState(0);
        //     }
    }

private:
    juce::TextButton saveAsPrepname, SaveAsCustom, moveToTrash;
    juce::TextEditor customName;
    juce::String defSaveName;

    SvCallback saveAs;
    SvCallbackRoutine trash, advanceRecname;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaveComponent)
};
