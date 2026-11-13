# choir: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.284%** of the stock 580,352-cycle block budget. Midpoint run: 3.136%. Every control at its heaviest position: 2.620%. Heaviest single setting: Root 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.136% |
| Chord 0.0% | 2.788% |
| Chord 12.5% | 3.040% |
| Chord 25.0% | 2.956% |
| Chord 37.5% | 3.177% |
| Chord 50.0% | 2.998% |
| Chord 62.5% | 3.079% |
| Chord 75.0% | 2.713% |
| Chord 87.5% | 3.041% |
| Chord 100.0% | 2.956% |
| Voicing 0.0% | 2.745% |
| Voicing 12.5% | 2.753% |
| Voicing 25.0% | 3.015% |
| Voicing 37.5% | 3.030% |
| Voicing 50.0% | 3.218% |
| Voicing 62.5% | 2.802% |
| Voicing 75.0% | 2.872% |
| Voicing 87.5% | 2.695% |
| Voicing 100.0% | 2.776% |
| Colour 0.0% | 3.098% |
| Colour 12.5% | 3.162% |
| Colour 25.0% | 2.911% |
| Colour 37.5% | 3.160% |
| Colour 50.0% | 3.069% |
| Colour 62.5% | 3.160% |
| Colour 75.0% | 2.911% |
| Colour 87.5% | 3.160% |
| Colour 100.0% | 3.098% |
| Root 0.0% | 3.180% |
| Root 12.5% | 3.093% |
| Root 25.0% | 3.175% |
| Root 37.5% | 3.093% |
| Root 50.0% | 3.284% |
| Root 62.5% | 3.018% |
| Root 75.0% | 3.148% |
| Root 87.5% | 3.018% |
| Root 100.0% | 3.148% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 12.5%, Root 50.0% | 2.620% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
