# Testing Lustre on an OP-XY

Checks to run on a device with a firmware that includes Lustre, from top to bottom, in one project:
each section leaves the project ready for the next. To run only some sections, set up what their first
line says. Host tests (`python ../open-xy/sdk.py test --machine .`) don't replace these: they can't hear
the sound or see the screen.

Report a failure with the section, the line and what you saw. A photo of the screen helps.

Record the machine commit, SDK commit, and firmware hash for each full pass.

## Setup

- Build a firmware with the machine ([building firmware](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md#3-add-your-machine))
  and install it.
- Create a new project, **plain**, without Lustre: give track 1 a stock sound of its own and save it.
  Later steps load it as a project that never had the machine.
- Create another new project, **qa**, for the pass: don't use a project you care about.
- Track 1: a stock synth, with one note on step 1 lasting a whole bar, so it sounds all the time while
  the sequencer plays. Track 15 (a send): Lustre (Shift + M1, then choose it). Turn track 1's
  send to track 15 up.
- Press play, and leave it playing unless a step says otherwise.

## Firmware basics

- [ ] The OP-XY boots, and stock engines, effects, the sequencer and project save/load work
- [ ] Lustre is in the effect list of tracks 15 and 16 (Shift + M1 on the selected send), under its name, and in
      no other track's list

## Its page and controls

- [ ] Sweep Size, Swarm, Decay, Space, Shimmer, Reverse, Drift and Tone in both directions.
      Every continuous control reads 00–99, including Size (60–360 ms internally). No abrupt cuts or stuck grains.
- [ ] Shimmer and Reverse affect newly launched grains, so changes may take a grain length to settle.
- [ ] Fast turns: the value keeps up, no flicker or slowdown
- [ ] Controls with an alternate (an encoder press, a second page): pressing switches, the page shows
      which is active, and each keeps its own value
- [ ] The page while playing and while stopped, and after leaving it and coming back: nothing stale,
      missing or drawn twice
- [ ] With track 1's send at 0 the effect is silent; raising it brings the effect in smoothly
- [ ] With only send 15 active, raising it reduces the dry source; full send is fully wet.
- [ ] Compare with the current renderer at send 0.75 and 1, using its unity wet mix.
      The return is pronounced without needing another effect or an external gain boost.
- [ ] Stop the sequencer while a note rings: the effect's tail ends naturally, with no click
- [ ] The same effect on track 16, at the same time as on track 15: each keeps its own values

## Parameter locks

- [ ] Lock each control on a step (hold the step, turn the knob): it plays on that step, and the page
      shows the lock while the step is held
- [ ] Several locks on one step, and locks on neighbouring steps: each plays as written
- [ ] Clear the locks: the control plays its own value again

## Patterns, scenes and songs

- [ ] Give the track a second pattern with different values: each pattern keeps its own
- [ ] Copy a pattern: the copy holds the same values, and changing one leaves the other alone
- [ ] Switch between the patterns while playing and while stopped: the sound and the page follow at once
- [ ] Make scenes 1 and 2 with the two patterns, and a song of scene 1 then scene 2, looping: each scene
      plays its own values from its first bar, on every pass

Then play scene 1.

## Mute

- [ ] Mute the track (hold aux and press the track's key): it's silent, with no click or tail left behind;
      unmute: it plays as before
- [ ] The same from the mixer
- [ ] Muted in scene 2 only: switching between scenes 1 and 2 mutes and unmutes it

Unmute it everywhere.

## Granular bloom and tails

- [ ] Play a short pluck or sparse arpeggio. Size short makes little fragments; Size long makes a soft bloom.
- [ ] Swarm high fills the gaps and raises the cloud level; Swarm low leaves distinct fragments.
- [ ] Shimmer 99 produces octave-up grains; Reverse 99 plays fragments backwards. Try both together.
- [ ] Drift 0 is steady; raising it adds gentle pitch movement. Tone darkens and brightens the cloud.
- [ ] Space 00 emphasizes grains; Space 99 is the diffuse reverb without a direct grain layer.
      Mono input spreads in stereo, without harsh ringing.
- [ ] Sweep send from zero to full on plucks, drums and chords: no harsh clipping or sudden jumps.
- [ ] Decay 99 with all other controls high stays bounded and fades after the input stops.
- [ ] Dial the Cathedral recipe (Decay/Space 99). A short phrase leaves a large, lush tail
      for tens of seconds; listen through at least 60 seconds for ringing or a persistent noise floor.
- [ ] While that tail rings, lower Decay and Space: it changes smoothly without clicks or a hard cut.
- [ ] Try each README recipe by ear on plucks, chords, bass and drums. Keep the same send level.
- [ ] While a long tail sounds, change to a stock effect and back. No old sound or burst leaks through.
- [ ] Rapid Size/Reverse/Shimmer automation does not chop off active grains or cause ticks.
- [ ] Run on both sends together, with several stock voices and normal project playback; no crackles.

## Block-rendering regression

- [ ] Compare Glass garden and Cathedral with the accepted revision's recordings
      at the same input and send settings: mix, intensity, grain timing and tail are unchanged.
- [ ] Size, Reverse, Shimmer and Drift high, including every control at 99:
      no boundary ticks, missing fragments or changed feedback when history wraps.
- [ ] Abrupt parameter locks, both sends, and changing away and back remain smooth.
- [ ] Immediately after selecting or resetting Lustre, short and long grains with
      Reverse/Shimmer/Drift high bloom smoothly, without old scratch or missing fragments.
- [ ] Sustain a phrase and then let the room ring out. Listen for ticks as the
      delay lines wrap, including with both sends active.
- [ ] Collect a fresh default CPU readout for this exact revision; check both
      mean and worst observed block against the 3.5% recommendation.

## Saving and loading

- [ ] Save **qa**, power off and on, load it: the machine is there with every value as you left it, on
      each pattern
- [ ] Change a value, wait a few seconds and power off without saving: the OP-XY starts normally, and
      **qa** loads as last saved
- [ ] Load **plain**: track 1 has its own sound, nothing of **qa** shows. Load **qa** again: all back
- [ ] While playing, load **plain** and then **qa** again: nothing freezes, no stray values
- [ ] Create a new project: put Lustre on a track; it starts at its defaults
- [ ] On a firmware without Lustre, open **qa**: it loads, and the rest of the project plays

## CPU

- [ ] Follow [hardware profiling](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md):
      a profiling build from the committed source, a run collected at the machine's heaviest setting, and
      the readout kept with the commit it measured

## In the builder

Once the machine's catalog entry is merged and released (the beta site from `staging`, open-xy.com from
`main`; see [submitting a machine](https://github.com/etcetc000/open-xy/blob/staging/docs/catalog.md#submit-a-machine)):

- [ ] The machine list shows Lustre as Community, with its name, maintainer and description
- [ ] Filtering by its kind (effects) lists it, and filtering by the other kinds doesn't
- [ ] Searching its name, and each of its keywords, finds it; its details panel lists its keywords, and
      clicking one lists it among that keyword's machines
- [ ] Choosing it builds a firmware that includes it, and that firmware passes
      [Firmware basics](#firmware-basics)

## Known limits

- No tempo sync, freeze or phrase looper. The network has a warm, softened top end.
- Effects have no native preset browser; dial the README recipes manually.
- A bloom starts after roughly one grain length.
