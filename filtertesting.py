
# design a low pass filter that removes frequencies above 4 Hz

    
# void lpf_coefs(int16_t n, int16_t f, int16_t fs, int16_t *coefs)
# {
#     int16_t i;
#     float sum = 0;
#     for (i = 0; i < n; i++) {
#         if (i - n / 2 == 0) {
#             coefs[i] = 2 * f / fs;
#         } else {
#             coefs[i] = sin(2 * PI * f * (i - n / 2) / fs) / (PI * (i - n / 2));
#         }
#         sum += coefs[i];
#     }
#     for (i = 0; i < n; i++) {
#         coefs[i] = coefs[i] / sum;
#     }
# }

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

lpf_coefs(100, 4, 800)

# fourier transform of the filter

import numpy as np
import matplotlib.pyplot as plt

n = 100
f = 4
fs = 80

coefs = lpf_coefs(n, f, fs)

x = np.arange(0, n)
y = np.fft.fft(coefs)
psd = np.abs(y) ** 2
psd_db = 10 * np.log10(psd)

freqs = np.fft.fftfreq(n, 1/fs)
# remove negative frequencies
psd_db = psd_db[:n//2]
freqs = freqs[:n//2]


plt.plot(freqs, psd_db)
plt.show()
