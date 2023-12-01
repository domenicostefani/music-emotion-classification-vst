/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PEditor::PEditor (EProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      initialComponent([&](juce::String a, juce::String b) {this->confirmSessionInfo(a,b);}, [&](bool a) {this->skipDBwindow(a);})
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 800);
    // addAndMakeVisible(mainComponent);
    addAndMakeVisible(initialComponent);

    if (!((INSTRUMENT == "electric") || (INSTRUMENT == "piano"))) {
        std::cout << "ERROR: INSTRUMENT must be either 'electric' or 'piano'" << std::endl;
        exit(1);
    }
    
}

PEditor::~PEditor()
{
}

//==============================================================================
void PEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void PEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();

    // if (visibleComponent == VisibleComponent::initc) {
    //     initialComponent.setBounds(area);
    // } else {
    //     mainComponentPtr->setBounds(area);
    // }

    // switch (visibleComponent) {
    //     case VisibleComponent::initc:
    //         initialComponent.setBounds(area);
    //         break;
    //     case VisibleComponent::mainc:
    //         mainComponentPtr->setBounds(area);
    //         break;
    //     case VisibleComponent::dbc:
    //         dbComponentPtr->setBounds(area);
    //         break;
    //     default:
    //         std::cout << "ERROR: unknown visibleComponent" << std::endl;
    //         exit(1);
    // }
    // redo with ifs
    if (visibleComponent == VisibleComponent::initc) {
        initialComponent.setBounds(area);
    } else if (visibleComponent == VisibleComponent::mainc) {
        mainComponentPtr->setBounds(area);
    } else if (visibleComponent == VisibleComponent::dbc) {
        std::cout << "Setting bounds for dbComponent" << std::endl;
        if (dbComponentPtr)
            dbComponentPtr->setBounds(area);
    } else {
        std::cout << "ERROR: unknown visibleComponent" << std::endl;
        exit(1);
    }


    // mainComponent.setBounds(area);
    // initialComponent.setBounds(area);
}



void PEditor::confirmSessionInfo(juce::String playerId, juce::String dateStr) {
    std::string playerIdstd = playerId.toStdString();
    std::string dateStrstd = dateStr.toStdString();
    std::string instrumentstr = INSTRUMENT;
    if ((instrumentstr == "electric")|| (instrumentstr == "acoustic")) {
        instrumentstr = instrumentstr + "guitar";
    }


    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expA_01_unprompted_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expA_02_unprompted_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expA_03_unprompted_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expA_04_unprompted_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_01_prompted_happy_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_02_prompted_aggressive_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_03_prompted_sad_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_04_prompted_relaxed_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_05_prompted_aggressive_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_06_prompted_sad_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_07_prompted_relaxed_"+dateStrstd+".wav");
    this->recording_filenames.push_back(instrumentstr+"_"+playerIdstd+"_expB_08_prompted_happy_"+dateStrstd+".wav");

    // // Print all filenames
    // for (auto &filename : recording_filenames) {
    //     std::cout << filename << std::endl;
    // }

    std::cout << "Instantiating DBComponent" << std::endl << std::flush;
    std::cout << "loadDefaultDb: " << loadDefaultDb << std::endl << std::flush;
    dbComponentPtr = std::make_unique<DBComponent>([&](std::map<int, std::vector<std::string>> a) {this->confirmDatabase(a);},loadDefaultDb);

    // dbComponent([&](std::map<int, std::vector<std::string>> a) {this->confirmDatabase(a);},loadDefaultDb)
    addAndMakeVisible(*dbComponentPtr);
    initialComponent.setVisible(false);
    this->visibleComponent = VisibleComponent::dbc;
    this->repaint();
    this->resized();
}

void PEditor::skipDBwindow(bool doskip) {
    this->loadDefaultDb = doskip;
}





void PEditor::confirmDatabase(std::map<int, std::vector<std::string>> dbmap) {
    auto emoDBptr = std::make_unique<EmoDB>(dbmap);
    mainComponentPtr = std::make_unique<MainComponent>(audioProcessor, recording_filenames, emoDBptr);
    addAndMakeVisible(*mainComponentPtr);
    dbComponentPtr->setVisible(false);
    this->visibleComponent = VisibleComponent::mainc;
    this->repaint();
    this->resized();
}