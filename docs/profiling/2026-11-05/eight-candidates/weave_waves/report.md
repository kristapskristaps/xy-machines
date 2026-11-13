# weave_waves: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **5.661%** of the stock 580,352-cycle block budget. Midpoint run: 5.652%. Every control at its heaviest position: 5.423%. Heaviest single setting: Lo fi 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 5.652% |
| Harmonic 0.0% | 5.435% |
| Harmonic 12.5% | 5.414% |
| Harmonic 25.0% | 5.429% |
| Harmonic 37.5% | 5.402% |
| Harmonic 50.0% | 5.443% |
| Harmonic 62.5% | 5.420% |
| Harmonic 75.0% | 5.443% |
| Harmonic 87.5% | 5.429% |
| Harmonic 100.0% | 5.451% |
| Timbre 0.0% | 5.397% |
| Timbre 12.5% | 5.428% |
| Timbre 25.0% | 5.399% |
| Timbre 37.5% | 5.431% |
| Timbre 50.0% | 5.409% |
| Timbre 62.5% | 5.434% |
| Timbre 75.0% | 5.403% |
| Timbre 87.5% | 5.429% |
| Timbre 100.0% | 5.406% |
| Morph 0.0% | 5.454% |
| Morph 12.5% | 5.406% |
| Morph 25.0% | 5.432% |
| Morph 37.5% | 5.405% |
| Morph 50.0% | 5.429% |
| Morph 62.5% | 5.399% |
| Morph 75.0% | 5.423% |
| Morph 87.5% | 5.400% |
| Morph 100.0% | 5.438% |
| Lo fi 0.0% | 5.414% |
| Lo fi 12.5% | 5.438% |
| Lo fi 25.0% | 5.412% |
| Lo fi 37.5% | 5.441% |
| Lo fi 50.0% | 5.661% |
| Lo fi 62.5% | 5.449% |
| Lo fi 75.0% | 5.425% |
| Lo fi 87.5% | 5.449% |
| Lo fi 100.0% | 5.425% |
| combined (as read from the reports): Harmonic 100.0%, Timbre 62.5%, Morph 0.0%, Lo fi 50.0% | 5.423% |

Recommended: at most 3.5%. **Warning: 5.661% of the block budget at Lo fi 50.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
