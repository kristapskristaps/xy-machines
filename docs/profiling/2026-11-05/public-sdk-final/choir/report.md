# choir: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 36 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.268%** of the stock 580,352-cycle block budget. Midpoint run: 3.128%. Every control at its heaviest position: 2.615%. Heaviest single setting: Root 50.0%.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 3.128% |
| Chord 0.0% | 2.855% |
| Chord 12.5% | 3.012% |
| Chord 25.0% | 2.969% |
| Chord 37.5% | 3.146% |
| Chord 50.0% | 3.014% |
| Chord 62.5% | 3.053% |
| Chord 75.0% | 2.716% |
| Chord 87.5% | 3.014% |
| Chord 100.0% | 2.971% |
| Voicing 0.0% | 2.715% |
| Voicing 12.5% | 2.744% |
| Voicing 25.0% | 2.948% |
| Voicing 37.5% | 3.046% |
| Voicing 50.0% | 3.186% |
| Voicing 62.5% | 2.864% |
| Voicing 75.0% | 2.901% |
| Voicing 87.5% | 2.744% |
| Voicing 100.0% | 2.779% |
| Colour 0.0% | 3.145% |
| Colour 12.5% | 3.113% |
| Colour 25.0% | 2.986% |
| Colour 37.5% | 3.114% |
| Colour 50.0% | 3.107% |
| Colour 62.5% | 3.114% |
| Colour 75.0% | 2.985% |
| Colour 87.5% | 3.116% |
| Colour 100.0% | 3.146% |
| Root 0.0% | 3.151% |
| Root 12.5% | 3.104% |
| Root 25.0% | 3.146% |
| Root 37.5% | 3.104% |
| Root 50.0% | 3.268% |
| Root 62.5% | 3.088% |
| Root 75.0% | 3.131% |
| Root 87.5% | 3.088% |
| Root 100.0% | 3.131% |
| combined (as read from the reports): Chord 37.5%, Voicing 50.0%, Colour 100.0%, Root 50.0% | 2.615% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
