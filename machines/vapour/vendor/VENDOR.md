# Vapour DSP provenance

Clouds by Émilie Gillet, MIT: https://github.com/pichenettes/eurorack/tree/f4cfa7697095a28d292bff758beb51deb49b5a86/clouds
Pinned source commit: `f4cfa7697095a28d292bff758beb51deb49b5a86`.

The granular algorithm is adapted from dsp/grain.h and dsp/granular_sample_player.h; diffusion follows dsp/fx/diffuser.h. These are algorithm adaptations, not unmodified source copies. The MIT notice is in LICENSE and granular.h. tables.h contains generated grain-size, transposition, normalization and Hann-window data following those formulas.

Changes: 44.1 kHz stereo packed 16-bit rolling storage; fixed-point interpolation, windows, pan and gain; fixed scratch workspace; bounded sample access before the ring fills; deterministic reset and freeze/resume handling; block-wise grain rendering. Two grains retain Hermite/Hann quality and six use linear/triangle interpolation, following Clouds' mixed-quality organization. Controls expose granular mode only. No firmware, stock artwork or frozen recordings are included.

VENDOR.json hashes adapted vendor files. The enclosing fx.cpp and core.h implement the host-facing processing and storage.
