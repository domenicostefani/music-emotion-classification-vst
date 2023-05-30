set -e

echo '#!/bin/bash\n# Cross-compilation script for ElkOS on RPI4\n# Instructions at:\n# https://elk-audio.github.io/elk-docs/html/documents/building_plugins_for_elk.html#cross-compiling-juce-plugin\n\nunset LD_LIBRARY_PATH\nsource /opt/elk/0.11.0/environment-setup-cortexa72-elk-linux\nexport CXXFLAGS="-O3 -pipe -ffast-math -feliminate-unused-debug-types -funroll-loops -Wno-poison-system-directories"\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleaseElectric CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Electric_EmotionClassificationPlugin.so\n\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleaseAcoustic CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Acoustic_EmotionClassificationPlugin.so\n\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleasePiano CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Piano_EmotionClassificationPlugin.so' > $1/Builds/linux-aarch64/compileReleaseElkPi4.sh 
chmod +x $1/Builds/linux-aarch64/compileReleaseElkPi4.sh


echo '#!/bin/bash\nmake -j`nproc` CONFIG=Release' > $1/Builds/linux-amd64/compileRelease.sh 
chmod +x $1/Builds/linux-amd64/compileRelease.sh


mkdir -p $1/Builds/linux-amd64/bin/
echo '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Electric_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_electric.json
echo '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Acoustic_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_acoustic.json
echo '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Piano_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_piano.json


echo '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_electric.json' > $1/Builds/linux-aarch64/bin/run_electric.sh
echo '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_acoustic.json' > $1/Builds/linux-aarch64/bin/run_acoustic.sh
echo '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_piano.json' > $1/Builds/linux-aarch64/bin/run_piano.sh
echo 'pkill -SIGINT -f sushi' > $1/Builds/linux-aarch64/bin/stop.sh


echo 'scp ./bin/* mind@elk-pi.local:~/EmotionClassificationPlugin_Offline/' > $1/Builds/linux-aarch64/copyover.sh 

cp $1/python-osc-server/server.py $1/Builds/linux-aarch64/bin/