# mando: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 61 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.714%** of the stock 580,352-cycle block budget. Midpoint run: 3.062%. Every control at its heaviest position: 3.069%. Heaviest single setting: Waves P S.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.062% |
| Cutoff 0.0% | 3.345% |
| Cutoff 12.5% | 3.368% |
| Cutoff 25.0% | 3.392% |
| Cutoff 37.5% | 3.421% |
| Cutoff 50.0% | 3.400% |
| Cutoff 62.5% | 3.429% |
| Cutoff 75.0% | 3.384% |
| Cutoff 87.5% | 3.418% |
| Cutoff 100.0% | 3.397% |
| Reso 0.0% | 3.406% |
| Reso 12.5% | 3.384% |
| Reso 25.0% | 3.406% |
| Reso 37.5% | 3.386% |
| Reso 50.0% | 3.407% |
| Reso 62.5% | 3.386% |
| Reso 75.0% | 3.421% |
| Reso 87.5% | 3.395% |
| Reso 100.0% | 3.444% |
| Drive 0.0% | 3.383% |
| Drive 12.5% | 3.404% |
| Drive 25.0% | 3.395% |
| Drive 37.5% | 3.447% |
| Drive 50.0% | 3.438% |
| Drive 62.5% | 3.467% |
| Drive 75.0% | 3.456% |
| Drive 87.5% | 3.485% |
| Drive 100.0% | 3.467% |
| Waves S S | 3.407% |
| Waves S P | 3.684% |
| Waves P S | 3.714% |
| Waves P P | 3.378% |
| Detune 0.0% | 3.406% |
| Detune 12.5% | 3.503% |
| Detune 25.0% | 3.526% |
| Detune 37.5% | 3.505% |
| Detune 50.0% | 3.546% |
| Detune 62.5% | 3.513% |
| Detune 75.0% | 3.529% |
| Detune 87.5% | 3.490% |
| Detune 100.0% | 3.511% |
| Balance 0.0% | 3.134% |
| Balance 12.5% | 3.400% |
| Balance 25.0% | 3.378% |
| Balance 37.5% | 3.400% |
| Balance 50.0% | 3.383% |
| Balance 62.5% | 3.404% |
| Balance 75.0% | 3.381% |
| Balance 87.5% | 3.403% |
| Balance 100.0% | 3.153% |
| Octave -12 | 3.395% |
| Octave 0 | 3.383% |
| Octave +12 | 3.635% |
| Keytrack 0.0% | 3.381% |
| Keytrack 12.5% | 3.481% |
| Keytrack 25.0% | 3.456% |
| Keytrack 37.5% | 3.459% |
| Keytrack 50.0% | 3.421% |
| Keytrack 62.5% | 3.459% |
| Keytrack 75.0% | 3.436% |
| Keytrack 87.5% | 3.481% |
| Keytrack 100.0% | 3.430% |
| combined (as read from the reports): Cutoff 62.5%, Reso 100.0%, Drive 87.5%, Waves P S, Detune 50.0%, Balance 62.5%, Octave +12, Keytrack 12.5% | 3.069% |

Recommended: at most 3.5%. **Warning: 3.714% of the block budget at Waves P S is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `9e2971b041e82946f1cd66ca2e868f1ffbfca7d3147f387d732c4939c05d8d75`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9d484556b54cbdf18ea6ca5e9325bc012ff24ab30a0cffb5272e8eb7f7957aed`. Firmware SHA-256 above identifies its resident kernel.
