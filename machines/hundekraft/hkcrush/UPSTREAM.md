# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/crush`. Original notices follow.

crush.c

/* Copyright (C) 2026 Hundekraft. Maiko #250 port authorized by owner; README.md. */

crush_dsp.c

/* Maiko #250 Crush DSP, reconstructed from the permission-authorized custom
 * region (init 0x01e4fd3c/render 0x01e4fd60). No TE code/data is imported.
 * Explicit scalar fixed-point operations replace Blackfin saturating intrinsics;
 * block ramps and per-channel sample holds retain the original algorithm.
 * Copyright (C) 2026 Hundekraft. GPL-3.0-or-later with OP-1 firmware permission. */

crush_screen.c

/* Maiko #250 original custom static art and recovered procedural plot geometry.
 * Re-rendered through kennel's raster; no TE font data is bundled. README.md. */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
