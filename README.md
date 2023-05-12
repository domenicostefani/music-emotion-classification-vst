# emotionally-aware-SMIs
Embedded implementation of an emotion classifier for a Smart Musical Instrument (electric guitar, acoustic guitar and piano)

## Repository organization
- Folders ```include``` and ```src``` contains the code for the feature extraction process defined in [1]. This follows the MusiCNN pipeline but in C++ instead of Python.
- Folder ```libs``` contains the dependencies and a script to compile them (```libs/compile_libraries.sh```). Currently, for aarch64 the libraries get compiled on device (RPI4), the binaries are then moved to a x86-64 laptop and only the plugin+tflite are cross-compiled.
- Folder ```utils``` contains the scripts to compile the feature extraction pipeline.
- Folder ```EmotionClassificationPlugin``` contains a JUCE plugin that records audio, extracts features and executes inference with a custom model.
- ```CMakeLists.txt```** NOT WORKING YET**: this should be fixed to compile/cross-compile the extraction pipeline with the dependencies, but it's not working right now, so we rely on the scripts in ```utils``` for the moment.

## Dependencies

<!-- - zlib - 1.2.13 ([github.com/madler/zlib](https://github.com/madler/zlib) commit ```04f42ceca40f73e2978b50e93806c2a18c1281fc```) -->
- **eigen** [3.3.4](https://gitlab.com/libeigen/eigen/-/releases/3.3.4) ([gitlab.com/libeigen/eigen](https://gitlab.com/libeigen/eigen) commit [```3dc3a0ea2d0773af4c0ffd7bbcb21c608e28fcef```](https://gitlab.com/libeigen/eigen/tree/3dc3a0ea2d0773af4c0ffd7bbcb21c608e28fcef))
- **ffmpeg** - [3.4.12](https://github.com/FFmpeg/FFmpeg/releases/tag/n3.4.12) ([github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg) commit [```872001459cf0a20c6f44105f485d125c8e22fc76```](https://github.com/FFmpeg/FFmpeg/tree/872001459cf0a20c6f44105f485d125c8e22fc76))
- **essentia** - main branch ([github.com/MTG/essentia](https://github.com/MTG/essentia) commit [```32376db9b39d8692509ac58036d0b539b7e```](https://github.com/MTG/essentia/tree/32376db9b39d8692509ac58036d0b539b7e))
- **tflite** - 2.11.0 from [domenicostefani/deep-classf-runtime-wrappers](https://github.com/domenicostefani/deep-classf-runtime-wrappers)

## References

1. Turchet, L., & Pauwels, J. (2021). Music emotion recognition: intention of composers-performers versus perception of musicians, non-musicians, and listening machines. IEEE/ACM Transactions on Audio, Speech, and Language Processing, 30, 305-316.