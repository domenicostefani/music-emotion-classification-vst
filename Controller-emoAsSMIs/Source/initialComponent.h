/*
  Initial component to select Player identifier and confirm date
*/

#pragma once

#include <JuceHeader.h>

class InitialComponent : public juce::Component {
public:
    //==============================================================================
    InitialComponent(std::function<void(juce::String, juce::String)>&& confirmFunction,
                                       std::function<void(bool)>&& skipDbSetupFunction);
    ~InitialComponent();

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    juce::TextButton okBtn;

    juce::TextEditor playerIDText, dayText, monthText, yearText;
    juce::Label playerIDLbl, dayLbl, monthLbl, yearLbl, descLbl;

    void confirmAll();

    juce::String defdayText, defmonthText, defyearText;

    std::function<void(juce::String, juce::String)> confirmDataFunction;
    std::function<void(bool)> skipDbSetupFunction;

private:
    juce::ToggleButton skipNextWindowBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InitialComponent)
};
