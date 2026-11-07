# sift: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **2.545%** of the stock 580,352-cycle block budget. Midpoint run: 2.535%. Every control at its heaviest position: 2.133%. Heaviest single setting: Body 25.0%.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 2.535% | 7.530% |
| Harmonic 0.0% | 2.141% | 5.357% |
| Harmonic 12.5% | 2.155% | 5.325% |
| Harmonic 25.0% | 2.141% | 5.278% |
| Harmonic 37.5% | 2.153% | 5.368% |
| Harmonic 50.0% | 2.141% | 5.292% |
| Harmonic 62.5% | 2.152% | 5.379% |
| Harmonic 75.0% | 2.141% | 5.356% |
| Harmonic 87.5% | 2.153% | 5.406% |
| Harmonic 100.0% | 2.141% | 5.318% |
| Timbre 0.0% | 2.152% | 5.321% |
| Timbre 12.5% | 2.141% | 5.252% |
| Timbre 25.0% | 2.153% | 5.336% |
| Timbre 37.5% | 2.141% | 5.246% |
| Timbre 50.0% | 2.153% | 5.399% |
| Timbre 62.5% | 2.141% | 5.409% |
| Timbre 75.0% | 2.153% | 5.351% |
| Timbre 87.5% | 2.142% | 5.403% |
| Timbre 100.0% | 2.156% | 5.519% |
| Morph 0.0% | 2.133% | 4.926% |
| Morph 12.5% | 2.145% | 4.994% |
| Morph 25.0% | 2.136% | 5.035% |
| Morph 37.5% | 2.152% | 5.370% |
| Morph 50.0% | 2.141% | 5.287% |
| Morph 62.5% | 2.153% | 5.373% |
| Morph 75.0% | 2.138% | 5.115% |
| Morph 87.5% | 2.147% | 5.003% |
| Morph 100.0% | 2.136% | 4.974% |
| Body 0.0% | 2.152% | 5.365% |
| Body 12.5% | 2.539% | 6.101% |
| Body 25.0% | 2.545% | 6.070% |
| Body 37.5% | 2.539% | 6.084% |
| Body 50.0% | 2.545% | 6.105% |
| Body 62.5% | 2.539% | 6.047% |
| Body 75.0% | 2.545% | 6.152% |
| Body 87.5% | 2.539% | 6.160% |
| Body 100.0% | 2.156% | 5.312% |
| combined (as read from the reports), differing between runs: Harmonic 37.5%, Timbre 100.0%, Morph 62.5%, Body 50.0%; Harmonic 12.5%, Timbre 100.0%, Morph 62.5%, Body 25.0%; Harmonic 12.5%, Timbre 50.0%, Morph 62.5%, Body 75.0% | 2.133% | 5.182% |

Worst observed block: **7.530%** (midpoint (stock)), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Worst observed block peak 7.530% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
