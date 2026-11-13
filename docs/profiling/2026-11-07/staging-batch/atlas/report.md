# atlas: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **2.908%** of the stock 580,352-cycle block budget. Midpoint run: 2.904%. Every control at its heaviest position: 2.734%. Heaviest single setting: Lo fi 50.0%.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 2.904% | 8.293% |
| Harmonic 0.0% | 2.751% | 4.228% |
| Harmonic 12.5% | 2.815% | 4.630% |
| Harmonic 25.0% | 2.744% | 4.260% |
| Harmonic 37.5% | 2.799% | 4.738% |
| Harmonic 50.0% | 2.762% | 4.628% |
| Harmonic 62.5% | 2.827% | 5.342% |
| Harmonic 75.0% | 2.765% | 5.025% |
| Harmonic 87.5% | 2.837% | 5.367% |
| Harmonic 100.0% | 2.770% | 5.054% |
| Timbre 0.0% | 2.800% | 4.714% |
| Timbre 12.5% | 2.741% | 4.245% |
| Timbre 25.0% | 2.800% | 4.689% |
| Timbre 37.5% | 2.745% | 4.306% |
| Timbre 50.0% | 2.811% | 4.706% |
| Timbre 62.5% | 2.744% | 4.320% |
| Timbre 75.0% | 2.806% | 4.581% |
| Timbre 87.5% | 2.739% | 4.215% |
| Timbre 100.0% | 2.808% | 4.608% |
| Morph 0.0% | 2.766% | 4.393% |
| Morph 12.5% | 2.806% | 4.625% |
| Morph 25.0% | 2.744% | 4.210% |
| Morph 37.5% | 2.808% | 4.730% |
| Morph 50.0% | 2.742% | 4.430% |
| Morph 62.5% | 2.802% | 4.685% |
| Morph 75.0% | 2.741% | 4.317% |
| Morph 87.5% | 2.795% | 4.701% |
| Morph 100.0% | 2.754% | 4.230% |
| Lo fi 0.0% | 2.814% | 4.706% |
| Lo fi 12.5% | 2.756% | 4.518% |
| Lo fi 25.0% | 2.811% | 4.729% |
| Lo fi 37.5% | 2.756% | 4.469% |
| Lo fi 50.0% | 2.908% | 4.837% |
| Lo fi 62.5% | 2.815% | 4.540% |
| Lo fi 75.0% | 2.873% | 4.779% |
| Lo fi 87.5% | 2.815% | 4.530% |
| Lo fi 100.0% | 2.875% | 4.773% |
| combined (as read from the reports), differing between runs: Harmonic 87.5%, Timbre 50.0%, Morph 37.5%, Lo fi 50.0%; Harmonic 87.5%, Timbre 50.0%, Morph 37.5%, Lo fi 50.0%; Harmonic 87.5%, Timbre 50.0%, Morph 12.5%, Lo fi 50.0% | 2.734% | 4.315% |

Worst observed block: **8.293%** (midpoint (stock)), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Worst observed block peak 8.293% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
