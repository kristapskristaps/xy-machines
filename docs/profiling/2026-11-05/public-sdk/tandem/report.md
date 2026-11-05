# tandem: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.369%** of the stock 580,352-cycle block budget. Midpoint run: 3.302%. Every control at its heaviest position: 2.660%. Heaviest single setting: Output 25.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.302% |
| Harmonic 0.0% | 2.660% |
| Harmonic 12.5% | 2.689% |
| Harmonic 25.0% | 2.670% |
| Harmonic 37.5% | 2.698% |
| Harmonic 50.0% | 2.698% |
| Harmonic 62.5% | 2.715% |
| Harmonic 75.0% | 2.748% |
| Harmonic 87.5% | 2.742% |
| Harmonic 100.0% | 2.783% |
| Timbre 0.0% | 2.696% |
| Timbre 12.5% | 2.683% |
| Timbre 25.0% | 2.696% |
| Timbre 37.5% | 2.681% |
| Timbre 50.0% | 2.696% |
| Timbre 62.5% | 2.683% |
| Timbre 75.0% | 2.696% |
| Timbre 87.5% | 2.683% |
| Timbre 100.0% | 2.699% |
| Morph 0.0% | 2.681% |
| Morph 12.5% | 2.696% |
| Morph 25.0% | 2.684% |
| Morph 37.5% | 2.695% |
| Morph 50.0% | 2.681% |
| Morph 62.5% | 2.696% |
| Morph 75.0% | 2.681% |
| Morph 87.5% | 2.696% |
| Morph 100.0% | 2.681% |
| Output 0.0% | 2.696% |
| Output 12.5% | 3.304% |
| Output 25.0% | 3.369% |
| Output 37.5% | 3.304% |
| Output 50.0% | 3.369% |
| Output 62.5% | 3.304% |
| Output 75.0% | 3.369% |
| Output 87.5% | 3.304% |
| Output 100.0% | 2.782% |
| combined (as read from the reports): Harmonic 100.0%, Timbre 100.0%, Morph 12.5%, Output 25.0% | 2.660% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
