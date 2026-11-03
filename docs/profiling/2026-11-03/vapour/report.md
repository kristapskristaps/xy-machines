# vapour: hardware CPU measurement

Machine commit: `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367`. Benchmark: `opxy-effect-callback-v1`. Scope: machine render callback only.

One capture: **4.559%** of the stock 580,352-cycle block budget.

Recommended: at most 3.5%. **Warning: 4.559% of the block budget is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or a settings sweep. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `e9a672835f954189e99b4b9146501991b3a470c29a8dcce6751df33b502b8bce`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9aa09831641a09f8765d82d636c50f6b8338106af90f03f987987178cc4f3f88`. Firmware SHA-256 above identifies its resident kernel.
