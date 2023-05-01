#!/usr/bin/env bash

set -e

unset LD_LIBRARY_PATH
# source /opt/elk/1.0/environment-setup-aarch64-elk-linux
source /opt/elk/0.11.0/environment-setup-cortexa72-elk-linux
CXXFLAGS="-O3 -pipe -ffast-math -feliminate-unused-debug-types -funroll-loops"
# AR=aarch64-elk-linux-ar make -j`nproc` CONFIG=Release CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"


#Check that script is called from  a folder called "build", otherwise exit (all paths are relative)
curdir=${PWD##*/}
if [ "$curdir" != "build" ]; then
    echo "Please run this script from the build folder"
    exit 1
fi

EXE_NAME="standalone_extraction_pipeline-linux-aarch64.o" # Name of the output executable
echo "Compiling '$EXE_NAME'..."

# Set starting path (We already checked that we are in the build folder)
HOMEBASE='..'

# spectrogram source file, from essentia/examples , plus utility required by the example
SOURCE_FILES="$HOMEBASE/src/standalone_extraction_pipeline.cpp"
# Include dir for essentia
ESSENTIA_INCLUDES="$HOMEBASE/libs/essentia/src/"
EIGEN_INCLUDES="/usr/include/eigen3"

# EIGEN_INCLUDES="/home/cimil-01/Develop/emotionally-aware-SMIs/libs/eigen/build-linux-x86_64"

ESSENTIA_BUILD="$HOMEBASE/libs/essentia/build-aarch64/src"


FFMPEG="$HOMEBASE/libs/FFmpeg-build-linux-aarch64"

VERBOSE='-v' # Comment this line to remove verbose output

# Command to compile the example with g++, linking to the essentia and all third party libraries (This does not work in CMake currently)
CMD="$CXX $VERBOSE $SOURCE_FILES\
    $CXXFLAGS \
    -I$ESSENTIA_INCLUDES  -I$EIGEN_INCLUDES\
    -L$ESSENTIA_BUILD \
    -L$FFMPEG \
    -lavdevice \
    -lessentia \
    -lavformat \
    -lswresample \
    -lfftw3f \
    -lavcodec \
    -lsamplerate \
    -lavutil \
    -latomic \
    -lgcc \
    -pthread\
    -o $EXE_NAME"


# Run command
sleep 1; echo "Running command: $CMD"
$CMD
echo "Done!"


sleep 1; echo -e "\nSaving simple execution command to ./run_standalone_extraction_pipeline.sh"
echo "./standalone_extraction_pipeline.o ~/Recordings/1-Edited/00-study2_links/acoustic_guitar_percussive_keybed_1_25_f_LucTur2_20201215.wav" > run_standalone_extraction_pipeline.sh
chmod +x run_standalone_extraction_pipeline.sh