/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MainComponent.h"
#include "initialComponent.h"
#include "databaseComponent.h"
#include "emotional_db.h"

//==============================================================================
/**
*/

class PEditor  : public juce::AudioProcessorEditor
{
public:
    PEditor (EProcessor&);
    ~PEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    void confirmSessionInfo(juce::String playerId, juce::String dateStr);
    void confirmDatabase(std::map<int, std::vector<std::string>> dbmap);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EProcessor& audioProcessor;
    std::unique_ptr<MainComponent> mainComponentPtr;
    InitialComponent initialComponent;
    DBComponent dbComponent;

    std::vector<std::string> recording_filenames;

    // Enum Visible component with values initial or main
    enum VisibleComponent { initc, mainc, dbc } visibleComponent = VisibleComponent::initc;    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PEditor)
};
