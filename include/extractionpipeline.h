#pragma once

#include <vector>
#include <string>

#include <essentia/algorithmfactory.h>
#include <essentia/streaming/algorithms/poolstorage.h>
#include <essentia/streaming/algorithms/vectoroutput.h>
#include <essentia/scheduler/network.h>

void extractFromFile(std::string audioFilename, std::vector<std::vector<essentia::Real>> &bandsVec);
