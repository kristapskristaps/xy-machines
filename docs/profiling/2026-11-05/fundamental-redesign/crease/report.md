# crease: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.581%** of the stock 580,352-cycle block budget. Midpoint run: 3.581%. Every control at its heaviest position: 2.744%. Heaviest single setting: Alt mix 50.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.581% |
| Harmonic 0.0% | 2.686% |
| Harmonic 12.5% | 2.750% |
| Harmonic 25.0% | 2.779% |
| Harmonic 37.5% | 2.661% |
| Harmonic 50.0% | 2.727% |
| Harmonic 62.5% | 2.675% |
| Harmonic 75.0% | 2.780% |
| Harmonic 87.5% | 2.756% |
| Harmonic 100.0% | 2.692% |
| Timbre 0.0% | 2.686% |
| Timbre 12.5% | 2.672% |
| Timbre 25.0% | 2.650% |
| Timbre 37.5% | 2.786% |
| Timbre 50.0% | 2.658% |
| Timbre 62.5% | 2.683% |
| Timbre 75.0% | 2.748% |
| Timbre 87.5% | 2.770% |
| Timbre 100.0% | 2.736% |
| Morph 0.0% | 2.800% |
| Morph 12.5% | 2.773% |
| Morph 25.0% | 2.690% |
| Morph 37.5% | 2.663% |
| Morph 50.0% | 2.742% |
| Morph 62.5% | 2.650% |
| Morph 75.0% | 2.669% |
| Morph 87.5% | 2.731% |
| Morph 100.0% | 2.727% |
| Alt mix 0.0% | 2.718% |
| Alt mix 12.5% | 2.747% |
| Alt mix 25.0% | 2.718% |
| Alt mix 37.5% | 2.753% |
| Alt mix 50.0% | 3.386% |
| Alt mix 62.5% | 3.140% |
| Alt mix 75.0% | 3.120% |
| Alt mix 87.5% | 3.131% |
| Alt mix 100.0% | 3.120% |
| combined (as read from the reports): Harmonic 75.0%, Timbre 37.5%, Morph 0.0%, Alt mix 50.0% | 2.744% |

Recommended: at most 3.5%. **Warning: 3.581% of the block budget at the midpoint is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
