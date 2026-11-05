# bitflip: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.893%** of the stock 580,352-cycle block budget. Midpoint run: 3.729%. Every control at its heaviest position: 3.227%. Heaviest single setting: Model FM.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 3.729% |
| Model Pulse | 3.099% |
| Model Tri | 3.111% |
| Model Twin | 3.595% |
| Model Wave | 3.111% |
| Model FM | 3.893% |
| Model Metal | 3.857% |
| Model Noise | 2.966% |
| Model Logic | 3.070% |
| Color 0.0% | 3.098% |
| Color 12.5% | 3.128% |
| Color 25.0% | 3.098% |
| Color 37.5% | 3.128% |
| Color 50.0% | 3.099% |
| Color 62.5% | 3.127% |
| Color 75.0% | 3.095% |
| Color 87.5% | 3.128% |
| Color 100.0% | 3.096% |
| Edge 0.0% | 3.125% |
| Edge 12.5% | 3.575% |
| Edge 25.0% | 3.613% |
| Edge 37.5% | 3.571% |
| Edge 50.0% | 3.612% |
| Edge 62.5% | 3.574% |
| Edge 75.0% | 3.612% |
| Edge 87.5% | 3.572% |
| Edge 100.0% | 3.610% |
| Sweep 0.0% | 3.096% |
| Sweep 12.5% | 3.127% |
| Sweep 25.0% | 3.098% |
| Sweep 37.5% | 3.124% |
| Sweep 50.0% | 3.098% |
| Sweep 62.5% | 3.151% |
| Sweep 75.0% | 3.156% |
| Sweep 87.5% | 3.215% |
| Sweep 100.0% | 3.209% |
| Chord Off | 3.128% |
| Chord Oct | 3.252% |
| Chord Major | 3.293% |
| Chord Minor | 3.264% |
| Chord Sus | 3.294% |
| Chord Maj7 | 3.250% |
| Chord Min7 | 3.267% |
| Chord Power | 3.273% |
| Speed 8Hz | 3.113% |
| Speed 12Hz | 3.084% |
| Speed 16Hz | 3.116% |
| Speed 24Hz | 3.098% |
| Speed 32Hz | 3.127% |
| Speed 40Hz | 3.098% |
| Speed 64Hz | 3.125% |
| Speed 80Hz | 3.098% |
| Bits 2 | 3.194% |
| Bits 3 | 3.163% |
| Bits 4 | 3.195% |
| Bits 5 | 3.166% |
| Bits 6 | 3.195% |
| Bits 7 | 3.165% |
| Bits 8 | 3.194% |
| Bits 9 | 3.165% |
| Bits 10 | 3.194% |
| Bits 11 | 3.165% |
| Bits 12 | 3.194% |
| Bits 13 | 3.165% |
| Bits 14 | 3.194% |
| Bits 15 | 3.163% |
| Bits 16 | 3.125% |
| Clock 1.4k | 3.153% |
| Clock 2.8k | 3.186% |
| Clock 5.5k | 3.163% |
| Clock 8.8k | 3.204% |
| Clock 11.0k | 3.182% |
| Clock 14.7k | 3.223% |
| Clock 22.1k | 3.217% |
| Clock 44.1k | 3.125% |
| combined (as read from the reports), differing between runs: Model FM, Color 12.5%, Edge 75.0%, Sweep 87.5%, Chord Sus, Speed 32Hz, Bits 4, Clock 14.7k; Model FM, Color 12.5%, Edge 25.0%, Sweep 87.5%, Chord Sus, Speed 32Hz, Bits 2, Clock 14.7k; Model FM, Color 37.5%, Edge 100.0%, Sweep 87.5%, Chord Major, Speed 64Hz, Bits 4, Clock 14.7k | 3.227% |

Recommended: at most 3.5%. **Warning: 3.893% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
