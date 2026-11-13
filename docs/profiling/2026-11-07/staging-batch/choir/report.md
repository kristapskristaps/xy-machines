# choir: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.268%** of the stock 580,352-cycle block budget. Midpoint run: 3.128%. Every control at its heaviest position: 2.615%. Heaviest single setting: Root 50.0%.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 3.128% | 6.287% |
| Chord 0.0% | 2.855% | 7.976% |
| Chord 12.5% | 3.012% | 6.012% |
| Chord 25.0% | 2.969% | 5.345% |
| Chord 37.5% | 3.146% | 5.978% |
| Chord 50.0% | 3.014% | 5.263% |
| Chord 62.5% | 3.053% | 5.805% |
| Chord 75.0% | 2.716% | 5.440% |
| Chord 87.5% | 3.014% | 5.945% |
| Chord 100.0% | 2.971% | 5.342% |
| Voicing 0.0% | 2.715% | 5.548% |
| Voicing 12.5% | 2.744% | 5.577% |
| Voicing 25.0% | 2.948% | 5.805% |
| Voicing 37.5% | 3.046% | 5.933% |
| Voicing 50.0% | 3.186% | 5.946% |
| Voicing 62.5% | 2.864% | 6.154% |
| Voicing 75.0% | 2.901% | 5.963% |
| Voicing 87.5% | 2.744% | 5.898% |
| Voicing 100.0% | 2.779% | 5.876% |
| Colour 0.0% | 3.145% | 8.557% |
| Colour 12.5% | 3.113% | 7.611% |
| Colour 25.0% | 2.986% | 8.573% |
| Colour 37.5% | 3.114% | 7.700% |
| Colour 50.0% | 3.107% | 8.550% |
| Colour 62.5% | 3.114% | 7.961% |
| Colour 75.0% | 2.985% | 8.559% |
| Colour 87.5% | 3.116% | 7.753% |
| Colour 100.0% | 3.146% | 8.634% |
| Root 0.0% | 3.151% | 7.721% |
| Root 12.5% | 3.104% | 5.460% |
| Root 25.0% | 3.146% | 6.030% |
| Root 37.5% | 3.104% | 5.470% |
| Root 50.0% | 3.268% | 6.107% |
| Root 62.5% | 3.088% | 5.498% |
| Root 75.0% | 3.131% | 6.088% |
| Root 87.5% | 3.088% | 5.495% |
| Root 100.0% | 3.131% | 6.056% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 100.0%, Root 50.0% | 2.615% | 7.663% |

Worst observed block: **8.634%** (Colour 100.0%), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Worst observed block peak 8.634% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
