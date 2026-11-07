# Auto Tune extension

The pitch tracker, stereo pitch shift and sound tests live here. Keep native
hook addresses and stock routing in the Open-XY SDK. The DSP callbacks use
preallocated memory and bounded work without OS APIs, I/O or logging.

From the collection root, run `python /path/to/open-xy/sdk.py test --extension
extensions/autotune` with an SDK that supports live input and the paired Q15
distance helper. Follow QA.md for an authorized device test. Keep firmware
and stock files outside all checkouts.
