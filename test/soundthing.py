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
#np.ubyte
sinarr = np.arange(1*48000)#,np.dtype('u2'))
sinarr = np.sin(2 * np.pi * sinarr * 400/48000)
sinarr = (sinarr+1)/2 * 256 * 0.3
print(sinarr)
sinarr = sinarr.astype(np.ubyte)#np.dtype('B'))
#sinarr = np.sin(sinarr)
print(sinarr)
sinarr = sinarr.tobytes()
print("\n\n\nhi\n")
#print(sinarr)
input("stoplol")

pya = pyaudio.PyAudio()
print(wf.getframerate())
stream = pya.open(format=pya.get_format_from_width(width=1), channels=1, rate=48000, output=True)
#wf.getframerate()
#data = wf.readframes(CHUNK)
#print(data)
#print(pya.get_format_from_width(width=2))
stream.write(sinarr)
# while data != '':
#     stream.write(data)
#     data = wf.readframes(CHUNK)
stream.stop_stream()
stream.close()

pya.terminate()