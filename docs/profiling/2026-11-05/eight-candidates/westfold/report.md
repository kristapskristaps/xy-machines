# westfold: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 72 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **6.511%** of the stock 580,352-cycle block budget. Midpoint run: 6.229%. Every control at its heaviest position: 6.264%. Heaviest single setting: FM Ratio 75.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 6.229% |
| Overtone 0.0% | 6.294% |
| Overtone 12.5% | 6.288% |
| Overtone 25.0% | 6.296% |
| Overtone 37.5% | 6.288% |
| Overtone 50.0% | 6.297% |
| Overtone 62.5% | 6.290% |
| Overtone 75.0% | 6.288% |
| Overtone 87.5% | 6.282% |
| Overtone 100.0% | 6.290% |
| Multiply 0.0% | 6.311% |
| Multiply 12.5% | 6.317% |
| Multiply 25.0% | 6.302% |
| Multiply 37.5% | 6.299% |
| Multiply 50.0% | 6.290% |
| Multiply 62.5% | 6.304% |
| Multiply 75.0% | 6.305% |
| Multiply 87.5% | 6.307% |
| Multiply 100.0% | 6.293% |
| Balance 0.0% | 6.282% |
| Balance 12.5% | 6.290% |
| Balance 25.0% | 6.299% |
| Balance 37.5% | 6.290% |
| Balance 50.0% | 6.296% |
| Balance 62.5% | 6.290% |
| Balance 75.0% | 6.296% |
| Balance 87.5% | 6.290% |
| Balance 100.0% | 6.297% |
| Decay 0.0% | 6.276% |
| Decay 12.5% | 6.285% |
| Decay 25.0% | 6.284% |
| Decay 37.5% | 6.296% |
| Decay 50.0% | 6.285% |
| Decay 62.5% | 6.300% |
| Decay 75.0% | 6.307% |
| Decay 87.5% | 6.313% |
| Decay 100.0% | 6.300% |
| FM Ratio 0.0% | 6.305% |
| FM Ratio 12.5% | 6.351% |
| FM Ratio 25.0% | 6.384% |
| FM Ratio 37.5% | 6.413% |
| FM Ratio 50.0% | 6.479% |
| FM Ratio 62.5% | 6.503% |
| FM Ratio 75.0% | 6.511% |
| FM Ratio 87.5% | 6.487% |
| FM Ratio 100.0% | 6.497% |
| FM Depth 0.0% | 6.287% |
| FM Depth 12.5% | 6.296% |
| FM Depth 25.0% | 6.290% |
| FM Depth 37.5% | 6.297% |
| FM Depth 50.0% | 6.290% |
| FM Depth 62.5% | 6.302% |
| FM Depth 75.0% | 6.300% |
| FM Depth 87.5% | 6.319% |
| FM Depth 100.0% | 6.326% |
| Slope Hz 0.0% | 6.268% |
| Slope Hz 12.5% | 6.278% |
| Slope Hz 25.0% | 6.304% |
| Slope Hz 37.5% | 6.287% |
| Slope Hz 50.0% | 6.299% |
| Slope Hz 62.5% | 6.290% |
| Slope Hz 75.0% | 6.300% |
| Slope Hz 87.5% | 6.304% |
| Slope Hz 100.0% | 6.348% |
| Motion 0.0% | 6.299% |
| Motion 12.5% | 6.293% |
| Motion 25.0% | 6.288% |
| Motion 37.5% | 6.307% |
| Motion 50.0% | 6.302% |
| Motion 62.5% | 6.313% |
| Motion 75.0% | 6.307% |
| Motion 87.5% | 6.316% |
| Motion 100.0% | 6.308% |
| combined (as read from the reports): Overtone 50.0%, Multiply 12.5%, Balance 25.0%, Decay 87.5%, FM Ratio 75.0%, FM Depth 100.0%, Slope Hz 100.0%, Motion 87.5% | 6.264% |

Recommended: at most 3.5%. **Warning: 6.511% of the block budget at FM Ratio 75.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
