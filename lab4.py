# draw a 1Hz square wave with amplitude between 0 and 5, from time 0 to 10 seconds and with a sampling rate of 10000Hz. Use matplotlib

import numpy as np
import matplotlib.pyplot as plt

# Create a time vector from 0 to 10 seconds with a sampling rate of 10000Hz
t = np.linspace(0, 10, 10000)

# Create a square wave with a frequency of 1Hz and amplitude between 0 and 5
f = 1
A = 2.5
y = A * np.sign(np.cos(2 * np.pi * f * t))

# move the square wave up by 2.5
y = y + 2.5

# Plot the square wave
plt.plot(t, y)
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.title('1Hz Square Wave')
plt.savefig('square_wave.png')


# compute and post-porcess the FFT of the square wave. Plot it
Y = np.fft.fft(y)
n = len(Y)
power = np.abs(Y) ** 2
freq = np.fft.fftfreq(n, 1/10000)
plt.figure()
plt.plot(freq, power)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Power')
plt.title('FFT of 1Hz Square Wave')
plt.savefig('fft_square_wave.png')