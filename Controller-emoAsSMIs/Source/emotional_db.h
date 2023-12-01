#pragma once
#include <JuceHeader.h>

#include <random>

class EmoDB {
public:
    EmoDB(std::map<int, std::vector<std::string>> dbmap) : dbmap(dbmap) {
        // Create map with vectors of shuffled indexes
        for (auto &pair : dbmap) {
            std::vector<size_t> indexes(pair.second.size());
            std::iota(indexes.begin(), indexes.end(), 0);
            std::shuffle(indexes.begin(), indexes.end(), std::mt19937{std::random_device{}()});
            shuffled_indexes[pair.first] = indexes;

            // Also init map with current read position in the shuffled indexes
            readPos[pair.first] = 0;

            // std::cout << "Shuffled indexes for emotion " << pair.first << ": ";
            // for (auto &index : indexes) {
            //     std::cout << index << " ";
            // }
            // std::cout << std::endl;
        }
    }
    ~EmoDB() = default;

    std::string getFilename(int emotion, int index) {
        return dbmap[emotion][shuffled_indexes[emotion][index]];
    }

    std::string getTrackFromEmo(bool aggr, bool rel, bool hap, bool sad, bool advanceIdx = true) {
        int emoCode = 0;
        if (aggr) emoCode += 0b1000;
        if (rel) emoCode += 0b0100;
        if (hap) emoCode += 0b0010;
        if (sad) emoCode += 0b0001;

        std::string res = dbmap[emoCode][shuffled_indexes[emoCode][readPos[emoCode]]];
        if (advanceIdx) {
            readPos[emoCode] = (readPos[emoCode] + 1) % dbmap[emoCode].size();
            // reshuflle indexes if we have reached the end
            if (readPos[emoCode] == 0) {
                // Althoug make sure next track is not the same as the previous one
                while (res == dbmap[emoCode][shuffled_indexes[emoCode][readPos[emoCode]]]) {
                    std::shuffle(shuffled_indexes[emoCode].begin(), shuffled_indexes[emoCode].end(), std::mt19937{std::random_device{}()});
                }
            }
        }
        return res;
    }

private:
    std::map<int, std::vector<std::string>> dbmap;
    std::map<int, std::vector<size_t>> shuffled_indexes;
    std::map<int, size_t> readPos;
};
