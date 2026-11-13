# atlas: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **2.908%** of the stock 580,352-cycle block budget. Midpoint run: 2.904%. Every control at its heaviest position: 2.734%. Heaviest single setting: Lo fi 50.0%.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 2.904% |
| Harmonic 0.0% | 2.751% |
| Harmonic 12.5% | 2.815% |
| Harmonic 25.0% | 2.744% |
| Harmonic 37.5% | 2.799% |
| Harmonic 50.0% | 2.762% |
| Harmonic 62.5% | 2.827% |
| Harmonic 75.0% | 2.765% |
| Harmonic 87.5% | 2.837% |
| Harmonic 100.0% | 2.770% |
| Timbre 0.0% | 2.800% |
| Timbre 12.5% | 2.741% |
| Timbre 25.0% | 2.800% |
| Timbre 37.5% | 2.745% |
| Timbre 50.0% | 2.811% |
| Timbre 62.5% | 2.744% |
| Timbre 75.0% | 2.806% |
| Timbre 87.5% | 2.739% |
| Timbre 100.0% | 2.808% |
| Morph 0.0% | 2.766% |
| Morph 12.5% | 2.806% |
| Morph 25.0% | 2.744% |
| Morph 37.5% | 2.808% |
| Morph 50.0% | 2.742% |
| Morph 62.5% | 2.802% |
| Morph 75.0% | 2.741% |
| Morph 87.5% | 2.795% |
| Morph 100.0% | 2.754% |
| Lo fi 0.0% | 2.814% |
| Lo fi 12.5% | 2.756% |
| Lo fi 25.0% | 2.811% |
| Lo fi 37.5% | 2.756% |
| Lo fi 50.0% | 2.908% |
| Lo fi 62.5% | 2.815% |
| Lo fi 75.0% | 2.873% |
| Lo fi 87.5% | 2.815% |
| Lo fi 100.0% | 2.875% |
| combined (as read from the reports), differing between runs: Harmonic 87.5%, Timbre 50.0%, Morph 37.5%, Lo fi 50.0%; Harmonic 87.5%, Timbre 50.0%, Morph 37.5%, Lo fi 50.0%; Harmonic 87.5%, Timbre 50.0%, Morph 12.5%, Lo fi 50.0% | 2.734% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
