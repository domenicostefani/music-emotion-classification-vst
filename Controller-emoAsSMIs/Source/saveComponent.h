#pragma once

#include <JuceHeader.h>

typedef std::function<void(std::string)> SvCallback;
typedef std::function<void()> SvCallbackRoutine;
typedef std::function<juce::String()> SvCallbackrstr;

class SaveComponent : public juce::Component
// , juce::Timer {
{
public:
    //==============================================================================
    SaveComponent(juce::String saveName, SvCallback&& saveAsp, SvCallbackRoutine&& trashp, SvCallbackRoutine&& advanceRecname, SvCallbackrstr&& nextName, SvCallbackrstr&& prevName)
        : saveAs(std::move(saveAsp)),
          trash(std::move(trashp)),
          advanceRecname(std::move(advanceRecname)),
          defSaveName(saveName),
          nextName(std::move(nextName)),
          prevName(std::move(prevName)){
        // startTimer(24);

        DBG("SaveComponent::SaveComponent() called");
        initGui();

    }
    ~SaveComponent() = default;

    void initGui()  {
        if (defSaveName == "") {
            defSaveName = "test.wav";
        }

        DBG("SaveComponent::SaveComponent() called with saveName = " + defSaveName);
        addAndMakeVisible(saveAsPrepname);
        saveAsPrepname.setButtonText("Save as\n" + defSaveName);
        saveAsPrepname.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgreen);
        saveAsPrepname.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
        saveAsPrepname.onClick = [&] {
            this->saveAs(this->defSaveName.toStdString());
            advanceRecname();
            this->close();
        };
        
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

        addAndMakeVisible(tinyNext);
        tinyNext.setButtonText(">");
        tinyNext.onClick = [&] {
            auto newname = nextName();
            this->defSaveName = newname;
            this->initGui();
            this->repaint();
        };

        addAndMakeVisible(tinyPrev);
        tinyPrev.setButtonText("<");
        tinyPrev.onClick = [&] {
            auto newname = prevName();
            this->defSaveName = newname;
            this->initGui();
            this->repaint();
        };
    }

    //==============================================================================
    void paint(juce::Graphics& g) override {
        // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.fillAll(juce::Colour(0xff445258));
    }

    void resized() override {
        auto area = getLocalBounds();
        auto redarea = area.reduced(20);
        auto tinyarea = redarea.removeFromBottom(20);
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


        int tinyBtnWidth = 30;
        int aa = (thirdwidth - tinyBtnWidth)/2;
        tinyarea.removeFromLeft(aa);
        tinyPrev.setBounds(tinyarea.removeFromLeft(tinyBtnWidth));
        tinyNext.setBounds(tinyarea.removeFromLeft(tinyBtnWidth));
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
    SvCallbackrstr nextName, prevName;

    juce::TextButton tinyNext, tinyPrev;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaveComponent)
};
