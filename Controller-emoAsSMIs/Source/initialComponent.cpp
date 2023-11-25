/*
  Initial component to select Player identifier and confirm date
*/

#include "initialComponent.h"


InitialComponent::InitialComponent(std::function<void(juce::String, juce::String)>&& confirmFunction): confirmDataFunction(std::move(confirmFunction)) {

    addAndMakeVisible(descLbl);
    descLbl.setText("Please enter an identifier for the musician and confirm the date of the experiment", juce::dontSendNotification);
    // descLbl.setMultiLine(true, true);
    descLbl.setJustificationType(juce::Justification::centred);
    descLbl.setFont(juce::Font(15.0f, juce::Font::bold));

    addAndMakeVisible(playerIDLbl);
    playerIDLbl.setText("Player identifier (e.g., MarRos for Mario Rossi)", juce::dontSendNotification);
    playerIDLbl.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(playerIDText);
    playerIDText.setJustification(juce::Justification::centred);
    playerIDText.setTextToShowWhenEmpty("PlaIde", juce::Colours::red);


    addAndMakeVisible(dayLbl);
    dayLbl.setText("Day", juce::dontSendNotification);
    dayLbl.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(dayText);
    dayText.setJustification(juce::Justification::right);

    addAndMakeVisible(monthLbl);
    monthLbl.setText("Month", juce::dontSendNotification);
    monthLbl.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(monthText);
    monthText.setJustification(juce::Justification::right);

    addAndMakeVisible(yearLbl);
    yearLbl.setText("Year", juce::dontSendNotification);
    yearLbl.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(yearText);
    yearText.setJustification(juce::Justification::right);

    addAndMakeVisible(okBtn);
    okBtn.setButtonText("OK");
    okBtn.onClick = [this] { confirmAll(); };

    // Get current date
    auto now = juce::Time::getCurrentTime();

    defdayText = juce::String(now.getDayOfMonth());
    defmonthText = juce::String(now.getMonth());
    defyearText = juce::String(now.getYear());
    dayText.setTextToShowWhenEmpty(defdayText, juce::Colours::lightgreen);
    monthText.setTextToShowWhenEmpty(defmonthText, juce::Colours::lightgreen);
    yearText.setTextToShowWhenEmpty(defyearText, juce::Colours::lightgreen);
}

InitialComponent::~InitialComponent() {
 
}

//==============================================================================
void InitialComponent::paint(juce::Graphics &g) {
    // g.setColour((isConnectionMade) ? juce::Colours::darkgreen : juce::Colours::red);
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void InitialComponent::resized() {
    auto area = getLocalBounds();

    auto centerarea = area.reduced(10);

    descLbl.setBounds(centerarea.removeFromTop(100));
    centerarea.removeFromTop(20);

    int pidareaheight = 50;
    auto playerIDArea = centerarea.removeFromTop(pidareaheight);

    playerIDLbl.setBounds(playerIDArea.removeFromTop(pidareaheight*.5f));
    playerIDText.setBounds(playerIDArea);

    int dateareaheight = 50;
    auto dateArea = centerarea.removeFromTop(dateareaheight);
    int tirdwidth = dateArea.getWidth() / 3;

    auto dayArea = dateArea.removeFromLeft(tirdwidth);
    dayLbl.setBounds(dayArea.removeFromTop(dateareaheight*.5f));
    dayText.setBounds(dayArea);

    auto monthArea = dateArea.removeFromLeft(tirdwidth);
    monthLbl.setBounds(monthArea.removeFromTop(dateareaheight*.5f));
    monthText.setBounds(monthArea);

    auto yearArea = dateArea.removeFromLeft(tirdwidth);
    yearLbl.setBounds(yearArea.removeFromTop(dateareaheight*.5f));
    yearText.setBounds(yearArea);


    okBtn.setBounds(centerarea.removeFromTop(100).reduced(20));
 
}


void InitialComponent::confirmAll() {
    auto playerId = playerIDText.getText();

    // If playerID empty, open alert window
    if (playerId == "") {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Warning", "Please enter a player identifier", "OK");
        return;
    }

    auto day =   dayText.getText()   == "" ? defdayText   : dayText.getText();
    auto month = monthText.getText() == "" ? defmonthText : monthText.getText();
    auto year =  yearText.getText()  == "" ? defyearText  : yearText.getText();

    // std::cout << "Player ID: '" << playerId << "'" << std::endl;
    // std::cout << "Day: '" << day << "'" << std::endl;
    // std::cout << "Month: '" << month << "'" << std::endl;
    // std::cout << "Year: '" << year << "'" << std::endl;
    // std::cout << "Confirming all" << std::endl;

    auto result = juce::AlertWindow::showOkCancelBox(
                    juce::AlertWindow::InfoIcon,
                    "Confirm?",
                    "Do you want to confirm the player ID and date ("+playerId.toStdString()+", "+day.toStdString()+"/"+month.toStdString()+"/"+year.toStdString()+" (dd/mm/yyyy)?\n\n",
                    "Yes",
                    "Cancel");
                if (result == 1) {
                    confirmDataFunction(playerId, year+month+day);
                    return;
                }
}
