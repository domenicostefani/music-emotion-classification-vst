#!/usr/bin/env bash

# This script compiles an Essentia example (streaming_spectrogram.cpp) for Linux_amd64
# This will be a base to create a CMakelists.txt file for the project
# However currently there are some issues with finding some third party libraries that are installed in the system
# 
# Domenico Stefani, 2023


#Check that script is called from  a folder called "build", otherwise exit (all paths are relative)
curdir=${PWD##*/}
if [ "$curdir" != "build" ]; then
    echo "Please run this script from the build folder"
    exit 1
fi

EXE_NAME="streaming_spectrogram.o" # Name of the output executable
echo "Compiling '$EXE_NAME'..."

# Set starting path (We already checked that we are in the build folder)
HOMEBASE='..'

# spectrogram source file, from essentia/examples , plus utility required by the example
SOURCE_FILES="$HOMEBASE/libs/essentia/src/examples/streaming_spectrogram.cpp $HOMEBASE/libs/essentia/src/examples/music_extractor/extractor_utils.cpp"
# Include dir for essentia
ESSENTIA_INCLUDES="$HOMEBASE/libs/essentia/src/"
# Include dir for essentia examples (for "credit_libav.h" and "music_extractor/extractor_utils.h")
ESSENTIA_EXAMPLES_INCLUDES="$HOMEBASE/libs/essentia/src/examples"

EIGEN_INCLUDES="/usr/include/eigen3"


# VERBOSE='-v' # Comment this line to remove verbose output

# Command to compile the example with g++, linking to the essentia and all third party libraries (This does not work in CMake currently)
CMD="g++ $VERBOSE $SOURCE_FILES\
    -I$ESSENTIA_INCLUDES -I$ESSENTIA_EXAMPLES_INCLUDES -I$EIGEN_INCLUDES\
    -L/home/cimil-01/Develop/emotionally-aware-SMIs/libs/essentia/build/src/\
    -lessentia \
    -lyaml \
    -lavformat \
    -lavutil \
    -lswresample \
    -lchromaprint \
    -lfftw3f \
    -lavcodec \
    -lavresample \
    -ltag \
    -lsamplerate \
    -o $EXE_NAME"

# Run command
sleep 1; echo "Running command: $CMD"
$CMD
echo "Done!"


sleep 1; echo -e "\nSaving simple execution command to ./run_streaming_spectrogram.sh"
echo "./streaming_spectrogram.o ~/Recordings/1-Edited/00-study2_links/acoustic_guitar_percussive_keybed_1_25_f_LucTur2_20201215.wav ssres ../utils/egtdb_profile.yaml; echo Result saved to ./ssres " > run_streaming_spectrogram.sh
chmod +x run_streaming_spectrogram.sh