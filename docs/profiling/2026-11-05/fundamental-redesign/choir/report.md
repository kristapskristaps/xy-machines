# choir: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.450%** of the stock 580,352-cycle block budget. Midpoint run: 3.354%. Every control at its heaviest position: 2.725%. Heaviest single setting: Colour 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.354% |
| Chord 0.0% | 2.835% |
| Chord 12.5% | 2.875% |
| Chord 25.0% | 2.887% |
| Chord 37.5% | 2.940% |
| Chord 50.0% | 2.904% |
| Chord 62.5% | 2.890% |
| Chord 75.0% | 2.762% |
| Chord 87.5% | 2.878% |
| Chord 100.0% | 2.887% |
| Voicing 0.0% | 2.779% |
| Voicing 12.5% | 2.873% |
| Voicing 25.0% | 2.889% |
| Voicing 37.5% | 2.928% |
| Voicing 50.0% | 2.977% |
| Voicing 62.5% | 2.840% |
| Voicing 75.0% | 2.837% |
| Voicing 87.5% | 2.806% |
| Voicing 100.0% | 2.783% |
| Colour 0.0% | 2.969% |
| Colour 12.5% | 2.995% |
| Colour 25.0% | 3.273% |
| Colour 37.5% | 2.995% |
| Colour 50.0% | 3.450% |
| Colour 62.5% | 2.995% |
| Colour 75.0% | 3.272% |
| Colour 87.5% | 2.995% |
| Colour 100.0% | 2.971% |
| Root 0.0% | 2.942% |
| Root 12.5% | 2.939% |
| Root 25.0% | 2.939% |
| Root 37.5% | 2.939% |
| Root 50.0% | 3.076% |
| Root 62.5% | 2.916% |
| Root 75.0% | 2.913% |
| Root 87.5% | 2.916% |
| Root 100.0% | 2.914% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 50.0%, Root 50.0% | 2.725% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
