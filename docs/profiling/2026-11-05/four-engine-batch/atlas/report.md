# atlas: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **5.090%** of the stock 580,352-cycle block budget. Midpoint run: 5.084%. Every control at its heaviest position: 4.952%. Heaviest single setting: Lo fi 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 5.084% |
| Harmonic 0.0% | 4.962% |
| Harmonic 12.5% | 4.985% |
| Harmonic 25.0% | 4.958% |
| Harmonic 37.5% | 4.971% |
| Harmonic 50.0% | 4.974% |
| Harmonic 62.5% | 4.997% |
| Harmonic 75.0% | 4.976% |
| Harmonic 87.5% | 5.006% |
| Harmonic 100.0% | 4.982% |
| Timbre 0.0% | 4.968% |
| Timbre 12.5% | 4.953% |
| Timbre 25.0% | 4.968% |
| Timbre 37.5% | 4.958% |
| Timbre 50.0% | 4.982% |
| Timbre 62.5% | 4.962% |
| Timbre 75.0% | 4.973% |
| Timbre 87.5% | 4.955% |
| Timbre 100.0% | 4.977% |
| Morph 0.0% | 4.982% |
| Morph 12.5% | 4.976% |
| Morph 25.0% | 4.956% |
| Morph 37.5% | 4.976% |
| Morph 50.0% | 4.956% |
| Morph 62.5% | 4.967% |
| Morph 75.0% | 4.952% |
| Morph 87.5% | 4.967% |
| Morph 100.0% | 4.967% |
| Lo fi 0.0% | 4.984% |
| Lo fi 12.5% | 4.967% |
| Lo fi 25.0% | 4.984% |
| Lo fi 37.5% | 4.965% |
| Lo fi 50.0% | 5.090% |
| Lo fi 62.5% | 5.051% |
| Lo fi 75.0% | 5.064% |
| Lo fi 87.5% | 5.049% |
| Lo fi 100.0% | 5.063% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 50.0%, Morph 0.0%, Lo fi 50.0% | 4.952% |

Recommended: at most 3.5%. **Warning: 5.090% of the block budget at Lo fi 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
