# acidline: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 66 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.983%** of the stock 580,352-cycle block budget. Midpoint run: 3.720%. Every control at its heaviest position: 2.815%. Heaviest single setting: Shape Square.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.720% |
| Flow 0.0% | 3.732% |
| Flow 12.5% | 3.708% |
| Flow 25.0% | 3.679% |
| Flow 37.5% | 3.651% |
| Flow 50.0% | 3.668% |
| Flow 62.5% | 3.642% |
| Flow 75.0% | 3.688% |
| Flow 87.5% | 3.638% |
| Flow 100.0% | 3.651% |
| Bite 0.0% | 3.647% |
| Bite 12.5% | 3.684% |
| Bite 25.0% | 3.655% |
| Bite 37.5% | 3.688% |
| Bite 50.0% | 3.662% |
| Bite 62.5% | 3.680% |
| Bite 75.0% | 3.661% |
| Bite 87.5% | 3.677% |
| Bite 100.0% | 3.671% |
| Burn 0.0% | 2.805% |
| Burn 12.5% | 3.659% |
| Burn 25.0% | 3.674% |
| Burn 37.5% | 3.656% |
| Burn 50.0% | 3.673% |
| Burn 62.5% | 3.668% |
| Burn 75.0% | 3.694% |
| Burn 87.5% | 3.699% |
| Burn 100.0% | 3.711% |
| Sweep 0.0% | 3.662% |
| Sweep 12.5% | 3.680% |
| Sweep 25.0% | 3.664% |
| Sweep 37.5% | 3.682% |
| Sweep 50.0% | 3.662% |
| Sweep 62.5% | 3.682% |
| Sweep 75.0% | 3.662% |
| Sweep 87.5% | 3.679% |
| Sweep 100.0% | 3.661% |
| Length 0.0% | 3.688% |
| Length 12.5% | 3.677% |
| Length 25.0% | 3.680% |
| Length 37.5% | 3.662% |
| Length 50.0% | 3.680% |
| Length 62.5% | 3.661% |
| Length 75.0% | 3.687% |
| Length 87.5% | 3.664% |
| Length 100.0% | 3.680% |
| Shape Saw | 3.664% |
| Shape Triangle | 3.673% |
| Shape Square | 3.983% |
| Body 0.0% | 3.677% |
| Body 12.5% | 3.563% |
| Body 25.0% | 3.583% |
| Body 37.5% | 3.563% |
| Body 50.0% | 3.581% |
| Body 62.5% | 3.563% |
| Body 75.0% | 3.583% |
| Body 87.5% | 3.563% |
| Body 100.0% | 3.583% |
| Shade 0.0% | 3.662% |
| Shade 12.5% | 3.673% |
| Shade 25.0% | 3.655% |
| Shade 37.5% | 3.685% |
| Shade 50.0% | 3.656% |
| Shade 62.5% | 3.670% |
| Shade 75.0% | 3.655% |
| Shade 87.5% | 3.687% |
| Shade 100.0% | 3.667% |
| combined (as read from the reports): Flow 0.0%, Bite 37.5%, Burn 100.0%, Sweep 37.5%, Length 0.0%, Shape Square, Body 0.0%, Shade 87.5% | 2.815% |

Recommended: at most 3.5%. **Warning: 3.983% of the block budget at Shape Square is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `9e2971b041e82946f1cd66ca2e868f1ffbfca7d3147f387d732c4939c05d8d75`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9d484556b54cbdf18ea6ca5e9325bc012ff24ab30a0cffb5272e8eb7f7957aed`. Firmware SHA-256 above identifies its resident kernel.
