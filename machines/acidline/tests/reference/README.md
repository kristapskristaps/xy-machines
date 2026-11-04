Frozen Acidline source from commit 97b6a9cc011169159625186d57b5e040e31dfee7,
with only its C++ namespace renamed to acidline_legacy. The original MIT license
at ../../LICENSE applies. This source is included only by host sound-comparison
tests and is not linked into the machine's audio callback.

The original 73 hashes in ../render_reference.h still authenticate this legacy
renderer. Tests compare the new voice's level, normalized edge energy, pitch
and contour timing against that frozen renderer instead of replacing hashes
with the new output. The 1x alias fixture explicitly measures the lost 2x/FIR
rejection.
