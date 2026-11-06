# Performance checkpoint

This extension lives separately from the SDK. Contributor C++ uses only public
`<opxy/...>` headers: no firmware addresses, assembly, OS headers or platform
conditionals. Tests belong in `tests/`.

Check with `python ../open-xy-checkpoint/sdk.py test --extension .`.
Firmware and stock content belong outside every checkout. Device operations
require a separate user instruction. Run QA.md against the exact candidate image.
