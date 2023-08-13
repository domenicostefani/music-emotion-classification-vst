import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))
os.system('for file in *.wav; do sox "$file" "../norm_$file" norm -0.1; done')