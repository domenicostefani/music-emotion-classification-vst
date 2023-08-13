from pydub import AudioSegment
from glob import glob
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

def match_target_amplitude(sound, target_dBFS):
    change_in_dBFS = target_dBFS - sound.dBFS
    return sound.apply_gain(change_in_dBFS)

if not os.path.exists('normalized'):
    os.makedirs('normalized')

wavs = sorted(glob('*.wav'))
for wid, wavf in enumerate(wavs):
    toprint = 'Normalizing File ['+str(wid)+'/'+str(len(wavs))+']: '
    print(toprint+wavf)
    sound = AudioSegment.from_file(wavf, "wav")
    normalized_sound = match_target_amplitude(sound, -20.0)
    normalized_sound.export("normalized/norm_"+wavf, format="wav")
    print('\b'*len(toprint), end='', flush=True)
    # break

# sound = AudioSegment.from_file("yourAudio.m4a", "m4a")
# normalized_sound = match_target_amplitude(sound, -20.0)
# normalized_sound.export("nomrmalizedAudio.m4a", format="mp4")