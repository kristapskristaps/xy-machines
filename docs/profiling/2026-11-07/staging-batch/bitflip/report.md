# bitflip: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 74 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 3 captures: **3.893%** of the stock 580,352-cycle block budget. Midpoint run: 3.729%. Every control at its heaviest position: 3.227%. Heaviest single setting: Model FM.

| Setting | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| midpoint (stock) | 3.729% | 7.297% |
| Model Pulse | 3.099% | 7.207% |
| Model Tri | 3.111% | 4.042% |
| Model Twin | 3.595% | 4.877% |
| Model Wave | 3.111% | 4.663% |
| Model FM | 3.893% | 4.237% |
| Model Metal | 3.857% | 3.951% |
| Model Noise | 2.966% | 3.241% |
| Model Logic | 3.070% | 3.981% |
| Color 0.0% | 3.098% | 4.070% |
| Color 12.5% | 3.128% | 4.005% |
| Color 25.0% | 3.098% | 4.041% |
| Color 37.5% | 3.128% | 3.973% |
| Color 50.0% | 3.099% | 4.099% |
| Color 62.5% | 3.127% | 4.004% |
| Color 75.0% | 3.095% | 4.021% |
| Color 87.5% | 3.128% | 4.016% |
| Color 100.0% | 3.096% | 4.022% |
| Edge 0.0% | 3.125% | 4.073% |
| Edge 12.5% | 3.575% | 7.205% |
| Edge 25.0% | 3.613% | 7.268% |
| Edge 37.5% | 3.571% | 7.253% |
| Edge 50.0% | 3.612% | 7.242% |
| Edge 62.5% | 3.574% | 7.176% |
| Edge 75.0% | 3.612% | 7.245% |
| Edge 87.5% | 3.572% | 7.219% |
| Edge 100.0% | 3.610% | 7.271% |
| Sweep 0.0% | 3.096% | 3.969% |
| Sweep 12.5% | 3.127% | 4.013% |
| Sweep 25.0% | 3.098% | 4.031% |
| Sweep 37.5% | 3.124% | 4.056% |
| Sweep 50.0% | 3.098% | 4.097% |
| Sweep 62.5% | 3.151% | 4.959% |
| Sweep 75.0% | 3.156% | 5.170% |
| Sweep 87.5% | 3.215% | 5.232% |
| Sweep 100.0% | 3.209% | 5.235% |
| Chord Off | 3.128% | 4.086% |
| Chord Oct | 3.252% | 5.309% |
| Chord Major | 3.293% | 5.396% |
| Chord Minor | 3.264% | 5.396% |
| Chord Sus | 3.294% | 5.373% |
| Chord Maj7 | 3.250% | 5.321% |
| Chord Min7 | 3.267% | 5.364% |
| Chord Power | 3.273% | 5.307% |
| Speed 8Hz | 3.113% | 4.086% |
| Speed 12Hz | 3.084% | 4.028% |
| Speed 16Hz | 3.116% | 4.063% |
| Speed 24Hz | 3.098% | 4.083% |
| Speed 32Hz | 3.127% | 4.088% |
| Speed 40Hz | 3.098% | 4.092% |
| Speed 64Hz | 3.125% | 4.085% |
| Speed 80Hz | 3.098% | 4.071% |
| Bits 2 | 3.194% | 4.140% |
| Bits 3 | 3.163% | 4.114% |
| Bits 4 | 3.195% | 4.166% |
| Bits 5 | 3.166% | 4.132% |
| Bits 6 | 3.195% | 4.154% |
| Bits 7 | 3.165% | 4.147% |
| Bits 8 | 3.194% | 4.129% |
| Bits 9 | 3.165% | 4.131% |
| Bits 10 | 3.194% | 4.134% |
| Bits 11 | 3.165% | 4.123% |
| Bits 12 | 3.194% | 4.138% |
| Bits 13 | 3.165% | 4.137% |
| Bits 14 | 3.194% | 4.128% |
| Bits 15 | 3.163% | 4.169% |
| Bits 16 | 3.125% | 4.068% |
| Clock 1.4k | 3.153% | 4.144% |
| Clock 2.8k | 3.186% | 4.155% |
| Clock 5.5k | 3.163% | 4.115% |
| Clock 8.8k | 3.204% | 4.164% |
| Clock 11.0k | 3.182% | 4.182% |
| Clock 14.7k | 3.223% | 4.189% |
| Clock 22.1k | 3.217% | 4.178% |
| Clock 44.1k | 3.125% | 4.063% |
| combined (as read from the reports), differing between runs: Model FM, Color 12.5%, Edge 75.0%, Sweep 87.5%, Chord Sus, Speed 32Hz, Bits 4, Clock 14.7k; Model FM, Color 12.5%, Edge 25.0%, Sweep 87.5%, Chord Sus, Speed 32Hz, Bits 2, Clock 14.7k; Model FM, Color 37.5%, Edge 100.0%, Sweep 87.5%, Chord Major, Speed 64Hz, Bits 4, Clock 14.7k | 3.227% | 4.132% |

Worst observed block: **7.297%** (midpoint (stock)), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Mean 3.893% of the block budget at Model FM is above the recommended 3.5%; make it lighter where you can (docs/profiling.md); Worst observed block peak 7.297% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
