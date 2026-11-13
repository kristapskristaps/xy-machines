# atlas: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **2.901%** of the stock 580,352-cycle block budget. Midpoint run: 2.884%. Every control at its heaviest position: 2.725%. Heaviest single setting: Lo fi 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.884% |
| Harmonic 0.0% | 2.737% |
| Harmonic 12.5% | 2.806% |
| Harmonic 25.0% | 2.728% |
| Harmonic 37.5% | 2.794% |
| Harmonic 50.0% | 2.745% |
| Harmonic 62.5% | 2.820% |
| Harmonic 75.0% | 2.748% |
| Harmonic 87.5% | 2.827% |
| Harmonic 100.0% | 2.756% |
| Timbre 0.0% | 2.789% |
| Timbre 12.5% | 2.727% |
| Timbre 25.0% | 2.791% |
| Timbre 37.5% | 2.730% |
| Timbre 50.0% | 2.803% |
| Timbre 62.5% | 2.734% |
| Timbre 75.0% | 2.795% |
| Timbre 87.5% | 2.730% |
| Timbre 100.0% | 2.800% |
| Morph 0.0% | 2.756% |
| Morph 12.5% | 2.797% |
| Morph 25.0% | 2.731% |
| Morph 37.5% | 2.799% |
| Morph 50.0% | 2.728% |
| Morph 62.5% | 2.791% |
| Morph 75.0% | 2.725% |
| Morph 87.5% | 2.791% |
| Morph 100.0% | 2.741% |
| Lo fi 0.0% | 2.805% |
| Lo fi 12.5% | 2.739% |
| Lo fi 25.0% | 2.805% |
| Lo fi 37.5% | 2.741% |
| Lo fi 50.0% | 2.901% |
| Lo fi 62.5% | 2.800% |
| Lo fi 75.0% | 2.864% |
| Lo fi 87.5% | 2.800% |
| Lo fi 100.0% | 2.864% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 50.0%, Morph 37.5%, Lo fi 50.0% | 2.725% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
