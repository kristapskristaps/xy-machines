# tandem: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **2.727%** of the stock 580,352-cycle block budget. Midpoint run: 2.580%. Every control at its heaviest position: 2.554%. Heaviest single setting: Harmonic 87.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.580% |
| Harmonic 0.0% | 2.553% |
| Harmonic 12.5% | 2.602% |
| Harmonic 25.0% | 2.557% |
| Harmonic 37.5% | 2.614% |
| Harmonic 50.0% | 2.570% |
| Harmonic 62.5% | 2.664% |
| Harmonic 75.0% | 2.589% |
| Harmonic 87.5% | 2.727% |
| Harmonic 100.0% | 2.612% |
| Timbre 0.0% | 2.617% |
| Timbre 12.5% | 2.562% |
| Timbre 25.0% | 2.618% |
| Timbre 37.5% | 2.562% |
| Timbre 50.0% | 2.620% |
| Timbre 62.5% | 2.568% |
| Timbre 75.0% | 2.614% |
| Timbre 87.5% | 2.562% |
| Timbre 100.0% | 2.626% |
| Morph 0.0% | 2.562% |
| Morph 12.5% | 2.614% |
| Morph 25.0% | 2.562% |
| Morph 37.5% | 2.618% |
| Morph 50.0% | 2.562% |
| Morph 62.5% | 2.623% |
| Morph 75.0% | 2.562% |
| Morph 87.5% | 2.615% |
| Morph 100.0% | 2.565% |
| Output 0.0% | 2.620% |
| Output 12.5% | 2.562% |
| Output 25.0% | 2.618% |
| Output 37.5% | 2.562% |
| Output 50.0% | 2.618% |
| Output 62.5% | 2.562% |
| Output 75.0% | 2.614% |
| Output 87.5% | 2.570% |
| Output 100.0% | 2.617% |
| combined (as read from the reports): Harmonic 87.5%, Timbre 100.0%, Morph 62.5%, Output 0.0% | 2.554% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
