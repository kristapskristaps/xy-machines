# tandem: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.430%** of the stock 580,352-cycle block budget. Midpoint run: 3.319%. Every control at its heaviest position: 2.797%. Heaviest single setting: Output 12.5%.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 3.319% |
| Harmonic 0.0% | 2.795% |
| Harmonic 12.5% | 2.747% |
| Harmonic 25.0% | 2.808% |
| Harmonic 37.5% | 2.754% |
| Harmonic 50.0% | 2.829% |
| Harmonic 62.5% | 2.782% |
| Harmonic 75.0% | 2.873% |
| Harmonic 87.5% | 2.817% |
| Harmonic 100.0% | 2.899% |
| Timbre 0.0% | 2.757% |
| Timbre 12.5% | 2.817% |
| Timbre 25.0% | 2.756% |
| Timbre 37.5% | 2.818% |
| Timbre 50.0% | 2.757% |
| Timbre 62.5% | 2.817% |
| Timbre 75.0% | 2.756% |
| Timbre 87.5% | 2.817% |
| Timbre 100.0% | 2.756% |
| Morph 0.0% | 2.817% |
| Morph 12.5% | 2.756% |
| Morph 25.0% | 2.818% |
| Morph 37.5% | 2.756% |
| Morph 50.0% | 2.817% |
| Morph 62.5% | 2.756% |
| Morph 75.0% | 2.817% |
| Morph 87.5% | 2.756% |
| Morph 100.0% | 2.817% |
| Output 0.0% | 2.756% |
| Output 12.5% | 3.430% |
| Output 25.0% | 3.380% |
| Output 37.5% | 3.429% |
| Output 50.0% | 3.380% |
| Output 62.5% | 3.429% |
| Output 75.0% | 3.378% |
| Output 87.5% | 3.430% |
| Output 100.0% | 2.792% |
| combined (as read from the reports), differing between runs: Harmonic 100.0%, Timbre 12.5%, Morph 0.0%, Output 12.5%; Harmonic 100.0%, Timbre 37.5%, Morph 0.0%, Output 12.5%; Harmonic 100.0%, Timbre 12.5%, Morph 25.0%, Output 87.5% | 2.797% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
