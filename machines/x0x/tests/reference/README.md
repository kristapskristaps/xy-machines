Frozen metallic and voice helpers from97b6a9cc011169159625186d57b5e040e31dfee7.
Only their function names are prefixed legacy_; they reuse the unchanged types,
parameters, PCM data and primitive tables. They are included by host tests only.
The existing47waveform and18metallic PCM/state hashes authenticate this legacy
renderer. New output is compared quantitatively, not substituted as new hashes.
The original machine MIT license applies.
