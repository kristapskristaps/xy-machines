# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/arc`. Original notices follow.

arc.c

/* arc: Duddell's singing arc (William Duddell, London 1899-1900). A carbon arc lamp shunted by an LC circuit
 * oscillates through the arc's negative resistance; Duddell played it from a keyboard. Each voice is a
 * Van der Pol oscillator  x'' - mu (1 - x^2) x' + w^2 x = 0  (a "lamp") plus a second, coupled lamp.
 *   page 1  blue  = current mu: clean sine -> reedy buzz -> screaming relaxation oscillation
 *           green = carbon: asymmetric electrode (even harmonics, a DC bias that the DC blocker removes)
 *           white = hiss: plasma noise injected into the oscillator (phase noise: pitch wanders at high values)
 *           red   = strike: pitch sag on attack (depth and 50 ms..1 s recovery) + an ignition noise burst; while a
 *                   key is held the lamp also sputters: random re-strikes (dip, crackle, short dropout), 0..6 / s
 *   page 2  blue  = 2nd lamp tuning ratio 1/2..2 with detents at 1/2 2/3 3/4 1 5/4 4/3 3/2 5/3 2 (word 5)
 *   (press) green = coupling: 0 = 2nd lamp off; weak = beats / locking, strong = chaos (word 6)
 *           white = brightness: key-tracked output tilt, dark .. bright (word 7; unset = neutral)
 *
 * DSP (integers only). The oscillator runs in its own time tau = w t, so the waveform does not change across the
 * keyboard: v' = eps (1 - x^2) v - x, x' = v, semi-implicit (symplectic) Euler with step h = G * theta per sample
 * (theta = 2 pi f / fs). The limit cycle period grows with eps (2 pi at eps = 0, ~12 at eps = 7) and the
 * discretisation shifts it again at high notes, so G comes from a 2-D table (eps knob x pitch, 1/8 octaves)
 * that gen_tables.py measured by running this very code (arc_calib.c): notes stay in tune at every setting.
 * The table is stored compressed (lossless, saves 3 KB of code space) and decoded into RAM once: by the screen's init
 * (GUI thread, normally before any note), else row by row on first use in render (bounded, once per boot).
 * Stability: x is clamped to +-3, v saturates at +-16; h*eps <= 0.3 (at high notes the lamp gets cleaner instead
 * of blowing up, which also keeps aliasing down) and the per-step damping factor is clamped at -0.75.
 * Cost: all products are 16 x 16 bit (one MAC on the Blackfin): x, v enter them as Q12 / Q11 views, h as a Q15
 * mantissa with one per-block shift per lamp. Pass 1 runs the lamps and writes the raw arc current into out[],
 * pass 2 shapes it (carbon, DC blocker, tilt, gain), so neither loop runs out of registers.
 * Voice state: 2 x (x, v) + rng + DC + tilt + strike env + gate = 32 bytes.
 * Copyright (C) 2026 ultralazr. GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

arc_calib.c

/* arc tuning calibration (host only, run by gen_tables.py; not part of the firmware build).
 * Runs the real engine (arc.c compiled with -DARC_CALIB -DOP1_HOST) with a fixed table value G*2pi and records
 * x1 per sample, measures the limit cycle period from interpolated upward zero crossings and iterates G until
 * the period equals 2^32 / inc samples. One line per table node: "ki pj gp cents rms" (rms of x1, 1.0 = 1). */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
