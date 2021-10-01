import pyaudio
import wave
import numpy as np 
#print("lmao2")

#input("idklol")

import array
CHUNK = 1024
bytestream = array.array('B')
wf = wave.open("piano2.wav", 'rb')
#bytestream.fromstring(audio_file.read())
#print(bytestream)

input("stoplol")

pya = pyaudio.PyAudio()
print(wf.getframerate())
stream = pya.open(format=pya.get_format_from_width(width=2), channels=1, rate=wf.getframerate(), output=True)
data = wf.readframes(CHUNK)

while data != '':
    stream.write(data)
    data = wf.readframes(CHUNK)
stream.stop_stream()
stream.close()

pya.terminate()