# sift: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **2.615%** of the stock 580,352-cycle block budget. Midpoint run: 2.589%. Every control at its heaviest position: 2.161%. Heaviest single setting: Body 25.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.589% |
| Harmonic 0.0% | 2.168% |
| Harmonic 12.5% | 2.225% |
| Harmonic 25.0% | 2.167% |
| Harmonic 37.5% | 2.225% |
| Harmonic 50.0% | 2.167% |
| Harmonic 62.5% | 2.223% |
| Harmonic 75.0% | 2.167% |
| Harmonic 87.5% | 2.225% |
| Harmonic 100.0% | 2.167% |
| Timbre 0.0% | 2.225% |
| Timbre 12.5% | 2.168% |
| Timbre 25.0% | 2.225% |
| Timbre 37.5% | 2.168% |
| Timbre 50.0% | 2.225% |
| Timbre 62.5% | 2.167% |
| Timbre 75.0% | 2.225% |
| Timbre 87.5% | 2.168% |
| Timbre 100.0% | 2.226% |
| Morph 0.0% | 2.161% |
| Morph 12.5% | 2.217% |
| Morph 25.0% | 2.162% |
| Morph 37.5% | 2.225% |
| Morph 50.0% | 2.168% |
| Morph 62.5% | 2.225% |
| Morph 75.0% | 2.164% |
| Morph 87.5% | 2.219% |
| Morph 100.0% | 2.161% |
| Body 0.0% | 2.225% |
| Body 12.5% | 2.567% |
| Body 25.0% | 2.615% |
| Body 37.5% | 2.568% |
| Body 50.0% | 2.615% |
| Body 62.5% | 2.567% |
| Body 75.0% | 2.615% |
| Body 87.5% | 2.567% |
| Body 100.0% | 2.249% |
| combined (as read from the reports): Harmonic 12.5%, Timbre 100.0%, Morph 37.5%, Body 25.0% | 2.161% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
