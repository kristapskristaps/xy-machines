# sift: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **8.214%** of the stock 580,352-cycle block budget. Midpoint run: 7.997%. Every control at its heaviest position: 5.249%. Heaviest single setting: Z mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 7.997% |
| Harmonic 0.0% | 6.012% |
| Harmonic 12.5% | 6.023% |
| Harmonic 25.0% | 6.003% |
| Harmonic 37.5% | 6.035% |
| Harmonic 50.0% | 5.998% |
| Harmonic 62.5% | 6.012% |
| Harmonic 75.0% | 5.997% |
| Harmonic 87.5% | 6.014% |
| Harmonic 100.0% | 5.994% |
| Timbre 0.0% | 5.956% |
| Timbre 12.5% | 5.949% |
| Timbre 25.0% | 5.960% |
| Timbre 37.5% | 5.927% |
| Timbre 50.0% | 6.027% |
| Timbre 62.5% | 6.047% |
| Timbre 75.0% | 6.065% |
| Timbre 87.5% | 6.052% |
| Timbre 100.0% | 6.072% |
| Morph 0.0% | 5.335% |
| Morph 12.5% | 5.455% |
| Morph 25.0% | 5.739% |
| Morph 37.5% | 6.032% |
| Morph 50.0% | 6.021% |
| Morph 62.5% | 6.033% |
| Morph 75.0% | 5.748% |
| Morph 87.5% | 5.425% |
| Morph 100.0% | 5.283% |
| Z mix 0.0% | 6.033% |
| Z mix 12.5% | 6.017% |
| Z mix 25.0% | 6.035% |
| Z mix 37.5% | 6.015% |
| Z mix 50.0% | 8.214% |
| Z mix 62.5% | 5.147% |
| Z mix 75.0% | 5.164% |
| Z mix 87.5% | 5.145% |
| Z mix 100.0% | 5.167% |
| combined (as read from the reports): Harmonic 37.5%, Timbre 100.0%, Morph 62.5%, Z mix 50.0% | 5.249% |

Recommended: at most 3.5%. **Warning: 8.214% of the block budget at Z mix 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
