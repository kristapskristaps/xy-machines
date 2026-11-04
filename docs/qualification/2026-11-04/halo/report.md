# halo: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-effect-callback-v1`. Scope: machine render callback only.

One capture: **3.210%** of the stock 580,352-cycle block budget.

Recommended: at most 3.5%. Within it.

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or a settings sweep. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `9e2971b041e82946f1cd66ca2e868f1ffbfca7d3147f387d732c4939c05d8d75`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9d484556b54cbdf18ea6ca5e9325bc012ff24ab30a0cffb5272e8eb7f7957aed`. Firmware SHA-256 above identifies its resident kernel.
