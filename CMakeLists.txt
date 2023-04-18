cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME EmoAwSMI)
project(${PROJECT_NAME} C CXX)
set(LIB_NAME speccc)

# include(FetchContent)
# FetchContent_Declare(
#   essentia
#   GIT_REPOSITORY https://github.com/MTG/essentia
#   GIT_TAG        ce359e8f73ac5b746b1fc2bc54fb320237cdadda # v2.1-beta5
# )
# FetchContent_MakeAvailable(essentia)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


find_package(PkgConfig REQUIRED)


pkg_check_modules(ESSDEPS REQUIRED IMPORTED_TARGET
    # libeigen3
    libyaml
    # libfftw3
    libavcodec
    libavformat
    libavutil
    libswresample
    # libsamplerate0
    # libtag1
    libchromaprint
)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules
                      ${CMAKE_MODULE_PATH})
find_package(YamlCpp)

set(CMAKE_CXX_STANDARD 11)
add_executable(${LIB_NAME}
    src/extractor_utils.cpp
    src/streaming_spectrogram.cpp
)
target_link_libraries(${LIB_NAME}
    PkgConfig::ESSDEPS
    # "/usr/lib/x86_64-linux-gnu/libyaml.a"
    "/usr/lib/x86_64-linux-gnu/libtag.so"
    "/home/cimil-01/Develop/emotionally-aware-SMIs/libs/essentia/build/src/libessentia.a"
)

target_include_directories(${LIB_NAME} PRIVATE /home/cimil-01/Develop/emotionally-aware-SMIs/libs/essentia/src/)
target_include_directories(${LIB_NAME} PRIVATE /home/cimil-01/Develop/emotionally-aware-SMIs/libs/essentia/src/examples)

