# westfold: hardware CPU measurement

Machine commit: `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 72 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **9.133%** of the stock 580,352-cycle block budget. Midpoint run: 8.731%. Every control at its heaviest position: 4.123%. Heaviest single setting: FM Ratio 75.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 8.731% |
| Overtone 0.0% | 6.746% |
| Overtone 12.5% | 8.881% |
| Overtone 25.0% | 8.870% |
| Overtone 37.5% | 8.882% |
| Overtone 50.0% | 8.870% |
| Overtone 62.5% | 8.881% |
| Overtone 75.0% | 8.870% |
| Overtone 87.5% | 8.878% |
| Overtone 100.0% | 8.869% |
| Multiply 0.0% | 8.435% |
| Multiply 12.5% | 8.391% |
| Multiply 25.0% | 8.725% |
| Multiply 37.5% | 8.856% |
| Multiply 50.0% | 8.893% |
| Multiply 62.5% | 8.849% |
| Multiply 75.0% | 8.580% |
| Multiply 87.5% | 8.562% |
| Multiply 100.0% | 8.894% |
| Balance 0.0% | 4.248% |
| Balance 12.5% | 8.884% |
| Balance 25.0% | 8.870% |
| Balance 37.5% | 8.882% |
| Balance 50.0% | 8.869% |
| Balance 62.5% | 8.882% |
| Balance 75.0% | 8.869% |
| Balance 87.5% | 8.882% |
| Balance 100.0% | 8.872% |
| Decay 0.0% | 8.864% |
| Decay 12.5% | 8.846% |
| Decay 25.0% | 8.870% |
| Decay 37.5% | 8.852% |
| Decay 50.0% | 8.872% |
| Decay 62.5% | 8.869% |
| Decay 75.0% | 8.884% |
| Decay 87.5% | 8.901% |
| Decay 100.0% | 8.899% |
| FM Ratio 0.0% | 8.780% |
| FM Ratio 12.5% | 8.876% |
| FM Ratio 25.0% | 8.931% |
| FM Ratio 37.5% | 9.001% |
| FM Ratio 50.0% | 9.094% |
| FM Ratio 62.5% | 9.125% |
| FM Ratio 75.0% | 9.133% |
| FM Ratio 87.5% | 9.126% |
| FM Ratio 100.0% | 9.126% |
| FM Depth 0.0% | 8.881% |
| FM Depth 12.5% | 8.873% |
| FM Depth 25.0% | 8.772% |
| FM Depth 37.5% | 8.774% |
| FM Depth 50.0% | 8.791% |
| FM Depth 62.5% | 8.760% |
| FM Depth 75.0% | 8.602% |
| FM Depth 87.5% | 8.544% |
| FM Depth 100.0% | 8.597% |
| Slope Hz 0.0% | 8.789% |
| Slope Hz 12.5% | 8.856% |
| Slope Hz 25.0% | 8.869% |
| Slope Hz 37.5% | 8.884% |
| Slope Hz 50.0% | 8.881% |
| Slope Hz 62.5% | 8.893% |
| Slope Hz 75.0% | 8.920% |
| Slope Hz 87.5% | 9.017% |
| Slope Hz 100.0% | 9.120% |
| Motion 0.0% | 8.894% |
| Motion 12.5% | 8.867% |
| Motion 25.0% | 8.898% |
| Motion 37.5% | 8.794% |
| Motion 50.0% | 8.685% |
| Motion 62.5% | 8.608% |
| Motion 75.0% | 8.611% |
| Motion 87.5% | 8.586% |
| Motion 100.0% | 8.602% |
| combined (as read from the reports): Overtone 37.5%, Multiply 100.0%, Balance 12.5%, Decay 87.5%, FM Ratio 75.0%, FM Depth 0.0%, Slope Hz 100.0%, Motion 25.0% | 4.123% |

Recommended: at most 3.5%. **Warning: 9.133% of the block budget at FM Ratio 75.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `1c5f96cf1aeb65402e7347026e33c9878052d39c904035d3b46c3cda59a5e862`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `7a454a4b044dcf31753eb414d3c32c01013d0b0a60cae6784803116db5400aef`. Firmware SHA-256 above identifies its resident kernel.
