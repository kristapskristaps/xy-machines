# sift: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **1.753%** of the stock 580,352-cycle block budget. Midpoint run: 1.740%. Every control at its heaviest position: 1.704%. Heaviest single setting: Harmonic 37.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 1.740% |
| Harmonic 0.0% | 1.714% |
| Harmonic 12.5% | 1.752% |
| Harmonic 25.0% | 1.714% |
| Harmonic 37.5% | 1.753% |
| Harmonic 50.0% | 1.714% |
| Harmonic 62.5% | 1.752% |
| Harmonic 75.0% | 1.714% |
| Harmonic 87.5% | 1.752% |
| Harmonic 100.0% | 1.714% |
| Timbre 0.0% | 1.750% |
| Timbre 12.5% | 1.711% |
| Timbre 25.0% | 1.750% |
| Timbre 37.5% | 1.712% |
| Timbre 50.0% | 1.752% |
| Timbre 62.5% | 1.714% |
| Timbre 75.0% | 1.752% |
| Timbre 87.5% | 1.714% |
| Timbre 100.0% | 1.752% |
| Morph 0.0% | 1.707% |
| Morph 12.5% | 1.747% |
| Morph 25.0% | 1.711% |
| Morph 37.5% | 1.752% |
| Morph 50.0% | 1.714% |
| Morph 62.5% | 1.753% |
| Morph 75.0% | 1.712% |
| Morph 87.5% | 1.747% |
| Morph 100.0% | 1.707% |
| Z mix 0.0% | 1.752% |
| Z mix 12.5% | 1.714% |
| Z mix 25.0% | 1.752% |
| Z mix 37.5% | 1.714% |
| Z mix 50.0% | 1.750% |
| Z mix 62.5% | 1.714% |
| Z mix 75.0% | 1.752% |
| Z mix 87.5% | 1.712% |
| Z mix 100.0% | 1.753% |
| combined (as read from the reports): Harmonic 37.5%, Timbre 50.0%, Morph 62.5%, Z mix 100.0% | 1.704% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
