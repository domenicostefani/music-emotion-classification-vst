# Music-emotion classification VST  for Emotionally-aware Smart Musical Instruments
Emotion classification plugin for a Smart Musical Instrument (electric guitar, acoustic guitar and piano).
The plugin can either run with a GUI on a regular computer, or run embedded in a Raspberry Pi running Elk Audio OS and be controlled remotely via an [OSC-controller](Controller-emoAsSMIs).

## Repository organization
- Folder `EmotionClassificationPlugin` contains a JUCE plugin that records audio, extracts features and executes inference with a custom model. It also contains the subfolder `python-osc-server` with the OSC server to run on a Elk Audio OS board, and `libs` with the dependencies.
- Folder `OSCcontroller` contains a Pure Data patch and accompanying abstraction to control the plugin via OSC messages.
- Folder `docs/images` contains images included in this README.

## Dependencies (EmotionClassificationPlugin/libs)

<!-- - zlib - 1.2.13 ([github.com/madler/zlib](https://github.com/madler/zlib) commit `04f42ceca40f73e2978b50e93806c2a18c1281fc`) -->
- **eigen** [3.3.4](https://gitlab.com/libeigen/eigen/-/releases/3.3.4) ([gitlab.com/libeigen/eigen](https://gitlab.com/libeigen/eigen) commit [`3dc3a0ea2d0773af4c0ffd7bbcb21c608e28fcef`](https://gitlab.com/libeigen/eigen/tree/3dc3a0ea2d0773af4c0ffd7bbcb21c608e28fcef))
- **ffmpeg** - [3.4.12](https://github.com/FFmpeg/FFmpeg/releases/tag/n3.4.12) ([github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg) commit [`872001459cf0a20c6f44105f485d125c8e22fc76`](https://github.com/FFmpeg/FFmpeg/tree/872001459cf0a20c6f44105f485d125c8e22fc76))
- **essentia** - main branch ([github.com/MTG/essentia](https://github.com/MTG/essentia) commit [`32376db9b39d8692509ac58036d0b539b7e`](https://github.com/MTG/essentia/tree/32376db9b39d8692509ac58036d0b539b7e))
- **tflite** - 2.11.0 from [domenicostefani/deep-classf-runtime-wrappers](https://github.com/domenicostefani/deep-classf-runtime-wrappers)

## Instruction to use the Emotion Classification Plugin

The plugin is made to be used both as a headless plugin in [Elk Audio Os](https://www.elk.audio/start) and standalone/VST2/3 plugin on a regular computer (currently only tested on `linux amd64` though).  
### GUI version
On a regular computer, the usage is straightforward: 
1. Open the plugin either as a standalone or in a DAW;
2. Select the folder where to save the recordings;
3. Select the TFlite model for the instrument of choice (contact the authors if not available);
4. Tune input gain: Try playing the instrument and check in the meter that the signal is coming through and the peak level is not too low or too high (clipping).
5. Tune the Playing/Silence detector: Try playing the instrument and check that the orange light turns on when you play and off when you stop playing. If it doesn't, change the threshold value until it does. If you change the input gain, repeat this step.
6. Press record, play an improvised emotional piece for more than 3 seconds, then press "Stop and classify".
7. Wait for the classifier to finish (~1 second for every 3 seconds of recording).
8. Check the results in the GUI and/or using Audacity and the label files in the recording folder.

### Elk OS Version
Since the execution on the Elk board is headless, the plugin is controlled via OSC messages from a desktop or laptop computer (we will refer to this as *controller*). A Pure Data controller application is provided in `OSCcontroller/`.

## OSC Controller

![emption-controller-off](https://github.com/domenicostefani/music-emotion-classification-vst/assets/23708296/0e691258-aaf5-41d9-8676-7cb01b08376a)


## VST on regular DAW

![emotion-plugin](https://github.com/domenicostefani/music-emotion-classification-vst/assets/23708296/e88ae362-a49d-4541-b918-acc779be2ddf)


## OSC connection notes
Here are some additional info about how the OSC controller communicates with the board and the plugin.
More details about the OSC communication procedure can be found in [docs/images/OSCcomm1.png](docs/images/OSCcomm1.png) and [docs/images/OSCcomm2.png](docs/images/OSCcomm2.png).
- Ports `6042` and `7042` are used to communicate with the Python OSC server that oversees plugin start/stop operations, file renaming and deletion (communication uses a 2-way handshake (osc /pyshake) to verify successful connection).
- Ports `8042` and `9042` are used to communicate with the plugin itself to receive loudness metering, plugin status and classification results (communication uses a 2-way handshake (osc /handshake) to verify successful plugin startup).
- Port `24024` is the default port used by the Sushi DAW in Elk Audio OS to set the parameters exposed by VST plugins. It's used to start/stop recording (setting the recstate parameter to 1 or 0), and input gain (linear, [0-1]).

___
## References

1. Turchet, L., & Pauwels, J. (2021). Music emotion recognition: intention of composers-performers versus perception of musicians, non-musicians, and listening machines. IEEE/ACM Transactions on Audio, Speech, and Language Processing, 30, 305-316.
