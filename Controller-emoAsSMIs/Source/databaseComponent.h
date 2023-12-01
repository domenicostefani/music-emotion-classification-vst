

/*
  Initial component to select Player identifier and confirm date
*/

#pragma once

#include <JuceHeader.h>
#include <map>

typedef std::function<void(std::map<int, std::vector<std::string>>)> DBfunc;

class DBComponent : public juce::Component {
public:
    //==============================================================================
    DBComponent(DBfunc&& okdbfun, bool loadDefaultDb = false) : confirmDbFunction(std::move(okdbfun)) {
        addAndMakeVisible(openDir);
        openDir.setButtonText("Open Database Directory");
        openDir.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible(foundFiles);
        foundFiles.setMultiLine(true);
        foundFiles.setReadOnly(true);
        foundFiles.setScrollbarsShown(true);
        foundFiles.setCaretVisible(false);

        addAndMakeVisible(okBtn);
        okBtn.setButtonText("OK");
        okBtn.onClick = [this] { okButtonClicked(); };

        juce::File tentDefaultFolder = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("emoDB");
        // try to see if subfolder emoDB exists
        if (tentDefaultFolder.exists()) {
            defaultFolder = tentDefaultFolder;
            addAndMakeVisible(openDefaultDir);

            openDefaultDir.setButtonText("Open Default Database Directory");
            openDefaultDir.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightgreen);
            openDefaultDir.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
            openDefaultDir.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);

            openDefaultDir.onClick = [this] {
                selFolder = defaultFolder;
                if (this->scanFolder())
                    okButtonClicked();
            };
        } else
            defaultFolder = juce::File::getSpecialLocation(juce::File::userHomeDirectory);

        if (tentDefaultFolder.exists() && loadDefaultDb) {
            openDefaultDir.triggerClick();
        }
    }
    ~DBComponent() = default;

    //==============================================================================
    void paint(juce::Graphics&) override {
    }

    void resized() override {
        auto area = getLocalBounds();
        area.removeFromTop(30);
        openDir.setBounds(area.removeFromTop(30).reduced(5));
        if (openDefaultDir.isVisible())
            openDefaultDir.setBounds(area.removeFromTop(30).reduced(5));
        okBtn.setBounds(area.removeFromBottom(30).reduced(5));
        foundFiles.setBounds(area.reduced(5));
    }

    void openButtonClicked() {
        chooser = std::make_unique<juce::FileChooser>("Select the dataset folder...",
                                                      defaultFolder);
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                this->selFolder = file;
                scanFolder();
            }
        });
    }

    bool scanFolder() {
        this->dbMap.clear();

        std::vector<int> emoCombinations = {
            0b1000,  // aggr
            0b0100,  // rel
            0b0010,  // hap
            0b0001,  // sad
            // Pairs
            0b1100,  // aggr_rel
            0b1010,  // aggr_hap
            0b1001,  // aggr_sad
            0b0110,  // rel_hap
            0b0101,  // rel_sad
            0b0011,  // hap_sad
            // Triplets
            0b1110,  // aggr_rel_hap
            0b1101,  // aggr_rel_sad
            0b1011,  // aggr_hap_sad
            0b0111,  // rel_hap_sad
            // Quadruplets
            0b1111  // aggr_rel_hap_sad
        };

        auto bin2emovec = [](int bin) {
            std::vector<std::string> res;
            if (bin & 0b1000) res.push_back("aggr");
            if (bin & 0b0100) res.push_back("rel");
            if (bin & 0b0010) res.push_back("hap");
            if (bin & 0b0001) res.push_back("sad");
            return res;
        };

        auto bin2emostr = [](int bin) {
            std::string res = "";
            if (bin & 0b1000) res += "aggr_";
            if (bin & 0b0100) res += "rel_";
            if (bin & 0b0010) res += "hap_";
            if (bin & 0b0001) res += "sad_";
            res = res.substr(0, res.size() - 1);  // Drop last char
            return res;
        };

        // lambda to split strings by delimiter _ and return vector of strings
        auto split = [](std::string s, char delim) {
            std::vector<std::string> res;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                res.push_back(item);
            }
            return res;
        };

        for (auto emoComb : emoCombinations) {
            // Check if folder exists

            juce::File emoCombFolder = selFolder.getChildFile(bin2emostr(emoComb));
            if (!emoCombFolder.exists()) {
                // Open juce alert window
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Warning", "Folder " + bin2emostr(emoComb) + " does not exist", "OK");
                return false;
            }
            DBG("Scanning folder " + bin2emostr(emoComb));

            // Get filenames of all wav files in folder
            juce::Array<juce::File> files;
            emoCombFolder.findChildFiles(files, juce::File::TypesOfFileToFind::findFiles, true, "*.wav");
            std::vector<std::string> filenames;
            for (auto f : files)
                filenames.push_back(f.getFullPathName().toStdString());
            DBG("Found " + std::to_string(filenames.size()) + " files");

            DBG("Appending to dbMap[" + std::to_string(emoComb) + "] (" + bin2emostr(emoComb) + ")");
            dbMap[emoComb] = filenames;
        }

        std::string printable = "";
        for (auto it : dbMap) {
            printable += bin2emostr(it.first) + " :\n";
            for (auto s : it.second) {
                s = s.substr(s.find_last_of("/\\") + 1);
                printable += "\t" + s + ",\n";
            }
            printable = printable.substr(0, printable.size() - 2);  // Drop last two chars
            printable += "\n";
        }

        foundFiles.setText(printable, juce::dontSendNotification);
        DBG(printable);
        return true;
    }

    void okButtonClicked() {
        DBG("OK button clicked");
        confirmDbFunction(dbMap);
    }

    DBfunc confirmDbFunction;

private:
    juce::TextButton openDir, okBtn, openDefaultDir;
    juce::TextEditor foundFiles;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::File selFolder, defaultFolder;
    std::map<int, std::vector<std::string>> dbMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DBComponent)
};
