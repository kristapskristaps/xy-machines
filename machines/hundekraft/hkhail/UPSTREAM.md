# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/hail`. Original notices follow.

LICENSE.beads

Copyright 2017 Emilie Gillet.

Author: Emilie Gillet (emilie.o.gillet@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

See http://creativecommons.org/licenses/MIT/ for more information.


hail.c

/* HAIL: granular texture FX, a fixed-point port of Mutable Instruments Beads (granular mode, digital media).
 * Original: https://github.com/pichenettes/eurorack/tree/master/beads (commit f4cfa769), dsp/processor.cc,
 * granular_processor.h, delay_granular_player.h, granular_player.h, delay_grain.h, grain.h, grain_envelope.h,
 * voice_stealing_envelope.h, trigger_generator.cc, limiter.h, fx/reverb.h. Copyright 2017 Emilie Gillet,
 * MIT licence (LICENSE.beads in this folder). Port, screen and OP-1 glue: Copyright (C) 2026 Hundekraft,
 * GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 *
 *   knob 0 (blue)  TIME     where in the recording (1.49 s) new grains start: 0 = just behind the write head
 *   knob 1 (green) SIZE     grain length 7.8 ms .. 4 s; below 1/3 the grains play backwards (as Beads)
 *   knob 2 (white) SHAPE    grain envelope: square, decay, bell, reverse ramp (as Beads)
 *   knob 3 (red)   DENSITY  left half: regular grains, fastest at the far left (130 Hz); right half: random
 *                           grains, fastest at the far right; the middle (48..52 %) starts no grains (Beads with
 *                           SEED locked and no gate patched)
 * encoder press (page 2, params[5..7], saved with the patch):
 *   blue press  -> PITCH  -24 .. +24 semitones per grain (centre = 0, small dead band)
 *   green press -> MIX    dry / wet (Beads' laws: 0.75 dry at 0, 1.5 wet at full)
 *   white press -> SPACE  0..50 %: reverb 0 .. 0.75; 50..100 %: reverb stays, feedback 0 .. 1
 *   red press   -> FREEZE (latch, not saved): nothing new is recorded, grains keep playing the buffer
 *
 * What the port leaves out or changes (OP-1: mono, 44.1 kHz, no FPU, 176 KB scratch, ~1/3 of the FX budget):
 *   - mono delay line, 16-bit, 65536 samples (Beads: 4 s stereo); no pan spread; grains read with linear
 *     instead of Hermite interpolation; up to BD_GRAINS grains (Beads: 32)
 *   - no delay mode (SIZE fully right stays granular), no wavetable synth mode, no CV / attenuverter randomness,
 *     no seed / gate input, no monophonic mode, no grain start at a fractional sample, no freeze-tail crossfade
 *   - feedback is taken from the previous 64-sample block (one block more loop delay), the 40 Hz SVF DC
 *     blocker is a one-pole high-pass (~27 Hz), the 0.999 one-pole after it is left out (it is ~identity)
 *   - Block-rate processing: the limiters run at block rate
 *     (peak per block, gain ramped; samples untouched while nothing is limited), the grain amplitude (envelope x
 *     voice stealing x gain x head fade) is computed once per block and ramped, each grain renders the block once
 *     (a trigger starts its grain with a pre-delay to its sample; a stolen grain's fade starts up to one block early),
 *     the reverb runs at quarter rate (11.025 kHz, Beads' delay lengths x0.1875 = x0.82 of Beads' times, loop
 *     low-pass set for the same cutoff), and the scheduler's pool is 12 grains, 8 while the reverb runs or rings out
 *     (Beads: 32; its 4 "safety" fades scale with the pool; a full pool gets a sqrt(16 / pool) level make-up)
 * Memory in this position's op1_fx_scratch: 128 KB recording + 32 KB reverb = 160 KB of 172 KB.

 * State per FX position (0 synth, 1 drum, 2 master; on the master the dispatcher feeds the mono sum). 


*/

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
