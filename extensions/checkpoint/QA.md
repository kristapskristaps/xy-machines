# Device checks

Record SDK/extension commits, candidate OS SHA-256 and layout. Test a disposable
project containing synth, sample, auxiliary and SDK-machine tracks.

The owner reported "works great!" for the core store/recall/recovery workflow
on 2026-11-06, after confirming the modifier is Metronome. The installed image
contained the same 26 Hundekraft/Halo machines, source hashes and machine IDs as
the prior build; only the firmware file was transferred, with backup and readback.

- SDK: `4c35df7f9b238de3424d27d126ec739bb89938fc`.
- Extension: `5860a71` (subsequent changes correct the modifier's source name and documentation).
- OS SHA-256: `facf403a9a43de8ee2280ea4cbe70d28d6466c672e78bbf60c659ddbd5f872f2`.

This user acceptance does not establish the detailed cases below or recall timing.
The final source must still pass these checks on its exact image.

- [ ] Metronome tap, Metronome + M2, Shift + Metronome and browser/naming controls
  retain stock behavior. Check both orders of releasing every chord.
- [ ] Metronome + plus stores once; holding plus does not repeatedly store.
- [ ] Edit notes, locks, components, length/scale, sounds, levels, pan, sends
  and mutes. Recall restores every captured value.
- [ ] Change samples and trims after storing; recall restores them. Exercise
  sample-memory pressure and retained assignments.
- [ ] Exercise different active and sound-link source patterns. Check the
  documented effects on other scenes sharing them.
- [ ] Exercise SDK drums, sample engines and control machines with saved state.
- [ ] Playback/external sync retain position. Listen for stalls, missing samples,
  stuck notes and discontinuities. Record timing on the exact image.
- [ ] Metronome + Record recovers the pre-recall state once, including SDK state.
  A redundant recall preserves this useful recovery state.
- [ ] No checkpoint, wrong scene, recording and changed pattern counts refuse
  without partial edits. Failed stores preserve the old checkpoint.
- [ ] New/load/reload project clears both states, including same-project reload.
- [ ] Stock undo and Retrospective capture still work after recall/recovery.
- [ ] Save/autosave does not move the checkpoint. Restart clears it.
