#!/bin/bash

set -e

echo -e '#!/bin/bash\n# Cross-compilation script for ElkOS on RPI4\n# Instructions at:\n# https://elk-audio.github.io/elk-docs/html/documents/building_plugins_for_elk.html#cross-compiling-juce-plugin\n\nunset LD_LIBRARY_PATH\nsource /opt/elk/0.11.0/environment-setup-cortexa72-elk-linux\nexport CXXFLAGS="-O3 -pipe -ffast-math -feliminate-unused-debug-types -funroll-loops -Wno-poison-system-directories"\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleaseElectric CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Electric_EmotionClassificationPlugin.so\n\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleaseAcoustic CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Acoustic_EmotionClassificationPlugin.so\n\nAR=aarch64-elk-linux-ar make -j`nproc` CONFIG=ReleasePiano CFLAGS="-DJUCE_HEADLESS_PLUGIN_CLIENT=1 -Wno-psabi" TARGET_ARCH="-mcpu=cortex-a72 -mtune=cortex-a72"\nmv ./build/EmotionClassificationPlugin.so ./bin/Piano_EmotionClassificationPlugin.so' > $1/Builds/linux-aarch64/compileReleaseElkPi4.sh 
chmod +x $1/Builds/linux-aarch64/compileReleaseElkPi4.sh


echo -e '#!/bin/bash\nmake -j`nproc` CONFIG=Release' > $1/Builds/linux-amd64/compileRelease.sh 
chmod +x $1/Builds/linux-amd64/compileRelease.sh


mkdir -p $1/Builds/linux-amd64/bin/

# INPUT_MODE="mono"
INPUT_MODE="stereo"

if [ "$INPUT_MODE" == "mono" ]; then
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "mono",\n            "inputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 1\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Electric_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                }\n            ]\n        }\n    ],\n    "midi" : {\n        "cc_mappings": [\n        ]\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_electric.json
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "mono",\n            "inputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 1\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Acoustic_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                }\n            ]\n        }\n    ],\n    "midi" : {\n        "cc_mappings": [\n        ]\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_acoustic.json
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "mono",\n            "inputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_channel" : 1,\n                    "track_channel" : 1\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Piano_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                }\n            ]\n        }\n    ],\n    "midi" : {\n        "cc_mappings": [\n        ]\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_piano.json
elif [ "$INPUT_MODE" == "stereo" ]; then
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Electric_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_electric.json
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Acoustic_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_acoustic.json
    echo -e '{\n    "host_config" : {\n        "samplerate" : 48000\n    },\n    "tracks" : [\n        {\n            "name" : "main",\n            "mode" : "stereo",\n            "inputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "outputs" : [\n                {\n                    "engine_bus" : 0,\n                    "track_bus" : 0\n                }\n            ],\n            "plugins" : [\n                {\n                    "path" : "/home/mind/EmotionClassificationPlugin_Offline/Piano_EmotionClassificationPlugin.so",\n                    "name" : "emotionclassifier",\n                    "type"   : "vst2x"\n                } \n            ]\n        }\n    ],\n    "midi" : {\n    }\n}\n' > $1/Builds/linux-aarch64/bin/config_piano.json
else
    echo "error"
fi



echo -e '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_electric.json' > $1/Builds/linux-aarch64/bin/run_electric.sh
echo -e '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_acoustic.json' > $1/Builds/linux-aarch64/bin/run_acoustic.sh
echo -e '#!/bin/bash\nsushi -r -c /home/mind/EmotionClassificationPlugin_Offline/config_piano.json' > $1/Builds/linux-aarch64/bin/run_piano.sh
echo -e 'pkill -SIGINT -f sushi' > $1/Builds/linux-aarch64/bin/stop.sh


echo -e 'scp ./bin/* mind@elk-pi.local:~/EmotionClassificationPlugin_Offline/' > $1/Builds/linux-aarch64/copyover.sh 

cp $1/python-osc-server/server.py $1/Builds/linux-aarch64/bin/

echo -e '[Unit]\nDescription=Emotionally Aware SmartMusicalInstruments OSC SERVER\n\n[Service]\nType=forking\nUser=mind\nWorkingDirectory=/home/mind/EmotionClassificationPlugin_Offline/\nExecStart=/bin/bash /home/mind/EmotionClassificationPlugin_Offline/start_server_in_tmux.sh\nExecStop=/usr/bin/tmux kill-session -t emoAwSMIs\n\n[Install]\nWantedBy=multi-user.target\n' > $1/Builds/linux-aarch64/bin/emo_aw_smis.service

echo -e '# Start the OSC server in tmux. Source manually profile.sh to get LD_LIBRARY_PATH (needed for sushi)\nsource /etc/profile.d/elk_profile.sh\n/usr/bin/tmux new -d -s emoAwSMIs "/usr/bin/python3 /home/mind/EmotionClassificationPlugin_Offline/server.py"\n' > $1/Builds/linux-aarch64/bin/start_server_in_tmux.sh