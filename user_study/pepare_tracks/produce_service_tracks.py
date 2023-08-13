#!/usr/bin/env python3

## This scrpt take the steelstring/classical guitar tracks from the dataset used in [1] and 
#  uses the user study data to sort them into the following emotional categories:
#     (single emotion)
#  0. Aggressive
#  1. Relaxed
#  2. Happy
#  3. Sad
#     (Ambiguous two emotions)
#  4. Aggressive/Relaxed - Amb.
#  5. Aggressive/Happy - Amb.
#  6. Aggressive/Sad - Amb.
#  7. Relaxed/Happy - Amb.
#  8. Relaxed/Sad - Amb.
#  9. Happy/Sad - Amb.
#     (Ambiguous three emotions)
#  10. Aggressive/Ralaxed/Happy - Amb.
#  11. Aggressive/Ralaxed/Sad - Amb.
#  12. Aggressive/Happy/Sad - Amb.
#  13. Relaxed/Happy/Sad - Amb.

import os
from glob import glob
os.chdir(os.path.dirname(os.path.abspath(__file__))) # Change directory to the one where this file is located

# Tensorflow-db-folder
DATABASE_FOLDER = "/home/cimil-01/tensorflow_datasets"
DATASET_NAME = "acoustic_guitar_emotion_recognition/"

# User-study-data-path
USER_STUDY_DATA_PATH = "./acoustic_guitar_db_userstudy.csv"


searchquery = os.path.join(DATABASE_FOLDER, 'downloads', 'extracted', '*'+DATASET_NAME.replace('_','-').strip('/')+'*')
print(searchquery)
vals = glob(searchquery)
print(vals)