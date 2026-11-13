# atlas: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **2.922%** of the stock 580,352-cycle block budget. Midpoint run: 2.863%. Every control at its heaviest position: 2.838%. Heaviest single setting: Lo fi 62.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.863% |
| Harmonic 0.0% | 2.853% |
| Harmonic 12.5% | 2.776% |
| Harmonic 25.0% | 2.849% |
| Harmonic 37.5% | 2.759% |
| Harmonic 50.0% | 2.866% |
| Harmonic 62.5% | 2.785% |
| Harmonic 75.0% | 2.866% |
| Harmonic 87.5% | 2.791% |
| Harmonic 100.0% | 2.873% |
| Timbre 0.0% | 2.757% |
| Timbre 12.5% | 2.846% |
| Timbre 25.0% | 2.759% |
| Timbre 37.5% | 2.850% |
| Timbre 50.0% | 2.771% |
| Timbre 62.5% | 2.849% |
| Timbre 75.0% | 2.766% |
| Timbre 87.5% | 2.843% |
| Timbre 100.0% | 2.768% |
| Morph 0.0% | 2.870% |
| Morph 12.5% | 2.768% |
| Morph 25.0% | 2.847% |
| Morph 37.5% | 2.770% |
| Morph 50.0% | 2.846% |
| Morph 62.5% | 2.762% |
| Morph 75.0% | 2.844% |
| Morph 87.5% | 2.756% |
| Morph 100.0% | 2.858% |
| Lo fi 0.0% | 2.771% |
| Lo fi 12.5% | 2.860% |
| Lo fi 25.0% | 2.771% |
| Lo fi 37.5% | 2.860% |
| Lo fi 50.0% | 2.869% |
| Lo fi 62.5% | 2.922% |
| Lo fi 75.0% | 2.835% |
| Lo fi 87.5% | 2.922% |
| Lo fi 100.0% | 2.835% |
| combined (as read from the reports): Harmonic 100.0%, Timbre 37.5%, Morph 0.0%, Lo fi 62.5% | 2.838% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
