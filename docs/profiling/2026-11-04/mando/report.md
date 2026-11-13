# mando: hardware CPU measurement

Machine commit: `49ccadf53bc4ddff5ed009d86139269349aef1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 61 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, each the median of 2 captures: **3.450%** of the stock 580,352-cycle block budget. Midpoint run: 2.856%. Every control at its heaviest position: 2.817%. Heaviest single setting: Waves S P.

| Setting | Median |
| --- | ---: |
| midpoint (stock) | 2.856% |
| Cutoff 0.0% | 3.094% |
| Cutoff 12.5% | 3.069% |
| Cutoff 25.0% | 3.132% |
| Cutoff 37.5% | 3.110% |
| Cutoff 50.0% | 3.136% |
| Cutoff 62.5% | 3.110% |
| Cutoff 75.0% | 3.138% |
| Cutoff 87.5% | 3.119% |
| Cutoff 100.0% | 3.146% |
| Reso 0.0% | 3.121% |
| Reso 12.5% | 3.146% |
| Reso 25.0% | 3.124% |
| Reso 37.5% | 3.146% |
| Reso 50.0% | 3.122% |
| Reso 62.5% | 3.145% |
| Reso 75.0% | 3.135% |
| Reso 87.5% | 3.160% |
| Reso 100.0% | 3.156% |
| Drive 0.0% | 3.146% |
| Drive 12.5% | 3.120% |
| Drive 25.0% | 3.163% |
| Drive 37.5% | 3.165% |
| Drive 50.0% | 3.203% |
| Drive 62.5% | 3.189% |
| Drive 75.0% | 3.223% |
| Drive 87.5% | 3.203% |
| Drive 100.0% | 3.233% |
| Waves S S | 3.120% |
| Waves S P | 3.450% |
| Waves P S | 3.433% |
| Waves P P | 3.147% |
| Detune 0.0% | 3.117% |
| Detune 12.5% | 3.205% |
| Detune 25.0% | 3.192% |
| Detune 37.5% | 3.216% |
| Detune 50.0% | 3.194% |
| Detune 62.5% | 3.205% |
| Detune 75.0% | 3.189% |
| Detune 87.5% | 3.207% |
| Detune 100.0% | 3.180% |
| Balance 0.0% | 2.955% |
| Balance 12.5% | 3.119% |
| Balance 25.0% | 3.142% |
| Balance 37.5% | 3.116% |
| Balance 50.0% | 3.143% |
| Balance 62.5% | 3.119% |
| Balance 75.0% | 3.144% |
| Balance 87.5% | 3.117% |
| Balance 100.0% | 2.959% |
| Octave -12 | 3.159% |
| Octave 0 | 3.145% |
| Octave +12 | 3.400% |
| Keytrack 0.0% | 3.143% |
| Keytrack 12.5% | 3.164% |
| Keytrack 25.0% | 3.186% |
| Keytrack 37.5% | 3.153% |
| Keytrack 50.0% | 3.178% |
| Keytrack 62.5% | 3.155% |
| Keytrack 75.0% | 3.178% |
| Keytrack 87.5% | 3.163% |
| Keytrack 100.0% | 3.186% |
| combined (as read from the reports), differing between runs: Cutoff 100.0%, Reso 87.5%, Drive 100.0%, Waves S P, Detune 37.5%, Balance 50.0%, Octave +12, Keytrack 25.0%; Cutoff 100.0%, Reso 87.5%, Drive 100.0%, Waves S P, Detune 37.5%, Balance 75.0%, Octave +12, Keytrack 25.0% | 2.817% |

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `c3b66a514a450bdab33a893e4f4870104b8b63c1a7e1ea6d2056aa0bcf21ca19`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `800f721707300d42c4e48c2ab960d4483078a51264847b4e44b8361031a29730`. Firmware SHA-256 above identifies its resident kernel.
