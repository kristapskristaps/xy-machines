# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/palme`. Original notices follow.

palme.c

/* "palme" - Ondes Martenot diffuseurs (Maurice Martenot): the "Palme" (1950), a palm-leaf soundboard with 12
 * chromatic strings that ring in sympathy with the loudspeaker, morphing into the "Metallique" gong speaker.
 * 12 Karplus-Strong strings excited by the input, tuned to the 12 semitones above the root. Each loop is one
 * 3-tap kernel = linear-interpolated fractional delay (1 - f, f) convolved with the Karplus-Strong averaging
 * filter (1 - b, b) with the decay gain folded into the taps; the filter's delay b is taken off the line
 * length so the fundamental stays in tune (FFT-verified within 1 cent for all 36 roots x 12 strings);
 * + excitation, - DC servo, saturating (= the loop clamp). Metallique glides the same 12 loops to free-plate
 * mode ratios (8 modes + 4 split twins that beat) and drives the sum (up to 2x) into a soft clip whose
 * residue is fed back into the strings while they are struck (energy blooms into the other modes).
 *   knob 0 (blue)  : tuning    root note C2 .. B4 in semitones (strings = root .. root + 11)
 *   knob 1 (green) : sustain   string decay T60 0.25 s .. 30 s
 *   knob 2 (white) : excite    input drive into the strings + brightness of the excitation
 *   knob 3 (red)   : mix       dry .. dry + strings .. strings
 * press (page 2, params[5..7], saved with the patch):
 *   blue  -> metallique  morph palme -> gong
 *   green -> damping     loop filter colour: dark (felt) .. bright (wire)
 *   white -> spread      detune spread 0 .. +-35 cents (fixed pattern per string)
 *   red   -> LATCH "etouffoir off" (not saved): the dampers lift. While input plays the loops sit at unity gain
 *            (they collect what they hear); ~0.3 s after it stops, each string's fundamental is held forever by
 *            an AGC (unity gain computed from the loop losses + PI on the fundamental's energy). Per-string
 *            loop limiter and a DC servo on each loop, so it never runs away or drifts.
 * Memory: 12 ring buffers of 1024 int16 (+ a 2-sample mirror of each ring's end in front of it) in this
 * position's scratch buffer, 25.7 KB. No periodic bulk work: a string's block is at most 2 runs (split where
 * its reads cross the ring's start; no masks per sample). Init only resets state (it runs in the audio
 * interrupt); the scratch is cleared over the first 8 blocks (strings silent meanwhile, the dry part passes).
 * CPU: string-major inner loop (one string over the 64-sample block: 3 fract16 MACs, 17 instructions per
 * sample); analysis / pitch / gain for 2 strings per block. ~371 instructions per sample in all: ~4.1 % of a
 * 400 MHz Blackfin at 1 instruction per cycle (bfin-elf-gcc -O2 objdump count); worst block 1.06 x median.
 * Copyright (C) 2026 ultralazr. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE). */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
