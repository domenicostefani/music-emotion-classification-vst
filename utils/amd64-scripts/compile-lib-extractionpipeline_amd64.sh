#!/usr/bin/env bash

# This script compiles a feature extraction pipeline based on Essentia
# This will be a base to create a CMakelists.txt file for the project
# However currently there are some issues with finding some third party libraries that are installed in the system
# 
# Domenico Stefani, 2023

set -e

CXX="g++"

curdir=${PWD##*/}
#Check that script is called from  a folder that has "build" in its name, otherwise exit (all paths are relative)
if [[ $curdir != *"build"* ]]; then
    echo "Please run this script from a 'build' folder"
    exit 1
fi

EXE_NAME="LIBTESTextraction_pipeline-linux-amd64" # Name of the output executable

# Set starting path (We already checked that we are in the build folder)
HOMEBASE='..'

# spectrogram source file, from essentia/examples , plus utility required by the example
SOURCE_FILES="$HOMEBASE/src/extractionpipeline.cpp"
# Include dir for essentia
ESSENTIA_INCLUDES="$HOMEBASE/libs/essentia/src/"
EIGEN_INCLUDES="/usr/include/eigen3"

CURRENT_INCLUDE="$HOMEBASE/include/"

ESSENTIA_BUILD="$HOMEBASE/libs/essentia/build-linux-x86_64/src"

# VERBOSE='-v' # Comment this line to remove verbose output
mkdir -p ./partials
# Command to compile the example with g++, linking to the essentia and all third party libraries (This does not work in CMake currently)
CMD="$CXX $VERBOSE $SOURCE_FILES \
    -c \
    -fPIC \
    -o./partials/libextractionpipeline.o \
    -I$ESSENTIA_INCLUDES  -I$EIGEN_INCLUDES\
    -I$CURRENT_INCLUDE \
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
    -lsamplerate"


# Run command
echo "Running command: $CMD"
$CMD

ar rvs ./libextractionpipeline.a ./partials/libextractionpipeline.o

# VERBOSE='-v' # Comment this line to remove verbose output

$CXX $VERBOSE $HOMEBASE/src/libmain.cpp \
    -I$ESSENTIA_INCLUDES  -I$EIGEN_INCLUDES\
    -I$CURRENT_INCLUDE \
    -L$ESSENTIA_BUILD \
    -L./ \
    -lextractionpipeline \
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
    -o $EXE_NAME \
     -Wl,--no-as-needed -ldl


SCRIPTNAME="run_LIBTESTextraction_pipeline_amd64"
sleep 1; echo -e "\nSaving simple execution command to ./$SCRIPTNAME.sh"
echo "./$EXE_NAME ~/Recordings/1-Edited/00-study2_links/acoustic_guitar_percussive_keybed_1_25_f_LucTur2_20201215.wav" > ./$SCRIPTNAME.sh
chmod +x ./$SCRIPTNAME.sh