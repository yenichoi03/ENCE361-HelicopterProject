
# design a low pass filter that removes frequencies above 4 Hz

    

# in python:
from math import sin, pi

def lpf_coefs(n, f, fs):
    coefs = []
    sum = 0
    for i in range(n):
        if i - n / 2 == 0:
            coefs.append(2 * f / fs)
        else:
            coefs.append(sin(2 * pi * f * (i - n / 2) / fs) / (pi * (i - n / 2)))
        sum += coefs[i]
    
    for i in range(n):
        coefs[i] = coefs[i] / sum
        
    return coefs

def box_coefs(n, f, fs):
    one_count = int(fs / (f * 1.2))
    
    if one_count > n:
        raise ValueError("Filter length too short for given frequency")
    
    coefs = [0] * (n - one_count)
    for i in range(one_count):
        coefs.append(1 / one_count)
            
    return coefs
    

def get_freq_response(coefs, n, fs):
    y = np.fft.fft(coefs)
    psd = np.abs(y) ** 2
    psd_db = 10 * np.log10(psd + 1e-12)

    freqs = np.fft.fftfreq(len(coefs), 1/fs)
    # remove negative frequencies
    # psd_db = psd_db[:n//2 + 1]
    # freqs = freqs[:n//2 + 1]
    
    return freqs, psd_db

# fourier transform of the filter

import numpy as np
import matplotlib.pyplot as plt

n = 70
f = 4
fs = n * 4

coefs = lpf_coefs(n, f, fs)

COEF_SCALE = 10000

print([int(i) for i in np.array(coefs) * COEF_SCALE])

box_car_coefs = box_coefs(n, f, fs)

freqs, psd_db = get_freq_response(coefs, n, fs)
freqs_box, psd_db_box = get_freq_response(box_car_coefs, n, fs)

# print attenuation at 4 Hz
print(psd_db[np.where(freqs == 4)[0][0]])

x = np.arange(n)


# plot kernel
plt.plot(x, coefs)
plt.plot(x, box_car_coefs)
plt.show()


plt.plot(freqs, psd_db, label='lpf')
# plt.show()

plt.plot(freqs_box, psd_db_box, label='box')
plt.legend()
plt.show()


# test different kernels against noisy sine wave of 4 Hz

# generate noisy sine wave
t = np.linspace(0, 1, fs)
sine = np.sin(2 * np.pi * 1 * t)
noise = np.random.normal(0, 1.5, fs)
noisy_sine = sine + noise

# apply filter
filtered_sine = np.convolve(noisy_sine, coefs, mode='same')
filtered_sine_box = np.convolve(noisy_sine, box_car_coefs, mode='same')

# plot
plt.plot(t, noisy_sine, label='noisy sine', alpha=0.5)
plt.plot(t, filtered_sine, label='filtered sine' , alpha=0.5)
plt.plot(t, filtered_sine_box, label='filtered sine box' , alpha=0.5)
plt.legend()
plt.show()
