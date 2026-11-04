# mando: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 61 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.644%** of the stock 580,352-cycle block budget. Midpoint run: 3.015%. Every control at its heaviest position: 3.046%. Heaviest single setting: Octave +12.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.015% |
| Cutoff 0.0% | 3.299% |
| Cutoff 12.5% | 3.320% |
| Cutoff 25.0% | 3.330% |
| Cutoff 37.5% | 3.352% |
| Cutoff 50.0% | 3.336% |
| Cutoff 62.5% | 3.340% |
| Cutoff 75.0% | 3.317% |
| Cutoff 87.5% | 3.349% |
| Cutoff 100.0% | 3.330% |
| Reso 0.0% | 3.340% |
| Reso 12.5% | 3.325% |
| Reso 25.0% | 3.343% |
| Reso 37.5% | 3.326% |
| Reso 50.0% | 3.342% |
| Reso 62.5% | 3.326% |
| Reso 75.0% | 3.352% |
| Reso 87.5% | 3.337% |
| Reso 100.0% | 3.375% |
| Drive 0.0% | 3.325% |
| Drive 12.5% | 3.339% |
| Drive 25.0% | 3.337% |
| Drive 37.5% | 3.392% |
| Drive 50.0% | 3.389% |
| Drive 62.5% | 3.410% |
| Drive 75.0% | 3.409% |
| Drive 87.5% | 3.429% |
| Drive 100.0% | 3.413% |
| Waves S S | 3.340% |
| Waves S P | 3.598% |
| Waves P S | 3.642% |
| Waves P P | 3.302% |
| Detune 0.0% | 3.340% |
| Detune 12.5% | 3.433% |
| Detune 25.0% | 3.447% |
| Detune 37.5% | 3.435% |
| Detune 50.0% | 3.447% |
| Detune 62.5% | 3.435% |
| Detune 75.0% | 3.449% |
| Detune 87.5% | 3.436% |
| Detune 100.0% | 3.459% |
| Balance 0.0% | 3.093% |
| Balance 12.5% | 3.337% |
| Balance 25.0% | 3.323% |
| Balance 37.5% | 3.337% |
| Balance 50.0% | 3.325% |
| Balance 62.5% | 3.340% |
| Balance 75.0% | 3.325% |
| Balance 87.5% | 3.337% |
| Balance 100.0% | 3.085% |
| Octave -12 | 3.328% |
| Octave 0 | 3.323% |
| Octave +12 | 3.644% |
| Keytrack 0.0% | 3.323% |
| Keytrack 12.5% | 3.386% |
| Keytrack 25.0% | 3.360% |
| Keytrack 37.5% | 3.386% |
| Keytrack 50.0% | 3.362% |
| Keytrack 62.5% | 3.389% |
| Keytrack 75.0% | 3.363% |
| Keytrack 87.5% | 3.394% |
| Keytrack 100.0% | 3.371% |
| combined (as read from the reports): Cutoff 37.5%, Reso 100.0%, Drive 87.5%, Waves P S, Detune 100.0%, Balance 62.5%, Octave +12, Keytrack 87.5% | 3.046% |

Recommended: at most 3.5%. **Warning: 3.644% of the block budget at Octave +12 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `b0a3ba8ed706badd68df9ab2f6ca918106d4984e2e8ce7896a5dc0f4a0c74350`.

Compile report SHA-256: `e1715797882ea8efffce81156cbf5a76502df861cf6b613bc988711da3024199`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
