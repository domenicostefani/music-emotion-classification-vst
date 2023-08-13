import os
from glob import glob

DB_PATH = '/home/cimil-01/tensorflow_datasets/downloads/extracted/ZIP.acoustic-guitar-emotion-dataset-v0.6.0.zip/acoustic-guitar'

# Change to script dir
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# for all folders in DB_PATH
for folder in sorted(glob(DB_PATH + '/*/')):
    # for all wavs in folder
    for wav in sorted(glob(folder + '*.wav')):
        # print link
        # print(wav)
        # break
        os.symlink(wav, os.path.basename(wav))