#!/usr/bin/env bash

# This script compiles a feature extraction pipeline based on Essentia
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

EXE_NAME="standalone_extraction_pipeline-linux-amd64.o" # Name of the output executable
echo "Compiling '$EXE_NAME'..."

# Set starting path (We already checked that we are in the build folder)
HOMEBASE='..'

# spectrogram source file, from essentia/examples , plus utility required by the example
SOURCE_FILES="$HOMEBASE/src/standalone_extraction_pipeline.cpp"
# Include dir for essentia
ESSENTIA_INCLUDES="$HOMEBASE/libs/essentia/src/"
EIGEN_INCLUDES="/usr/include/eigen3"

ESSENTIA_BUILD="$HOMEBASE/libs/essentia/build-linux-x86_64/src"

# VERBOSE='-v' # Comment this line to remove verbose output

# Command to compile the example with g++, linking to the essentia and all third party libraries (This does not work in CMake currently)
CMD="g++ $VERBOSE $SOURCE_FILES\
    -I$ESSENTIA_INCLUDES  -I$EIGEN_INCLUDES\
    -L$ESSENTIA_BUILD \
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


sleep 1; echo -e "\nSaving simple execution command to ./run_standalone_extraction_pipeline.sh"
echo "./standalone_extraction_pipeline-linux-amd64.o ~/Recordings/1-Edited/00-study2_links/acoustic_guitar_percussive_keybed_1_25_f_LucTur2_20201215.wav" > run_standalone_extraction_pipeline.sh
chmod +x run_standalone_extraction_pipeline.sh