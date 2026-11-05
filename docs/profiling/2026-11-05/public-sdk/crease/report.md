# crease: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.529%** of the stock 580,352-cycle block budget. Midpoint run: 3.426%. Every control at its heaviest position: 2.699%. Heaviest single setting: Alt mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.426% |
| Harmonic 0.0% | 2.719% |
| Harmonic 12.5% | 2.695% |
| Harmonic 25.0% | 2.722% |
| Harmonic 37.5% | 2.695% |
| Harmonic 50.0% | 2.765% |
| Harmonic 62.5% | 2.695% |
| Harmonic 75.0% | 2.766% |
| Harmonic 87.5% | 2.750% |
| Harmonic 100.0% | 2.724% |
| Timbre 0.0% | 2.693% |
| Timbre 12.5% | 2.650% |
| Timbre 25.0% | 2.760% |
| Timbre 37.5% | 2.722% |
| Timbre 50.0% | 2.695% |
| Timbre 62.5% | 2.707% |
| Timbre 75.0% | 2.687% |
| Timbre 87.5% | 2.759% |
| Timbre 100.0% | 2.661% |
| Morph 0.0% | 2.728% |
| Morph 12.5% | 2.718% |
| Morph 25.0% | 2.715% |
| Morph 37.5% | 2.696% |
| Morph 50.0% | 2.777% |
| Morph 62.5% | 2.692% |
| Morph 75.0% | 2.707% |
| Morph 87.5% | 2.702% |
| Morph 100.0% | 2.699% |
| Alt mix 0.0% | 2.771% |
| Alt mix 12.5% | 2.780% |
| Alt mix 25.0% | 2.771% |
| Alt mix 37.5% | 2.782% |
| Alt mix 50.0% | 3.529% |
| Alt mix 62.5% | 3.201% |
| Alt mix 75.0% | 3.206% |
| Alt mix 87.5% | 3.201% |
| Alt mix 100.0% | 3.206% |
| combined (as read from the reports): Harmonic 75.0%, Timbre 25.0%, Morph 50.0%, Alt mix 50.0% | 2.699% |

Recommended: at most 3.5%. **Warning: 3.529% of the block budget at Alt mix 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
