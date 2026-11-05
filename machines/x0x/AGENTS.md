# X0X

This machine is `machines/x0x`. Follow the repository root AGENTS.md and
[development guide](../../docs/development.md). The collection root
`SDK_REPOSITORY` and `SDK_REVISION` pin its SDK; use the clean checkout
configured by `./xy setup`, without modifying or resetting other checkouts.

X0X needs the drum `parameterEdit` callback, drawing ABI 6 played-pad
snapshots and bounded sample-free drum presets. Preserve these requirements
when changing the SDK dependency. Use `./xy test x0x --native` and
`./xy preview x0x --watch` from the repository root. Its full 24-pad audition
is documented in README.md.

Keep type, style, choke group and both control pages per pad. Preserve all 47
valid type/style combinations, 7-bit controls, saved IDs and the seven kits.
Sound, display and arithmetic tests belong here. DSP uses public opxy headers,
with no platform conditionals, assembly, allocation, blocking, I/O or graphics
in audio callbacks. Keep licences, recording credits and asset provenance.

Regenerate the vector scene with `python machines/x0x/artwork/drummer.py`.
Pass the configured SDK explicitly to the preset generator and audition server.
Use the root layout-preserving build/stage workflow for authorized hardware
tests. Host checks do not establish device CPU cost; prior hardware results
belong to the measured standalone revisions until the collection build is tested.
