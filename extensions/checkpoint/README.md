# Performance checkpoint

A temporary scene checkpoint for OP-XY OS 1.2.1, using the checkpoint-capable
Open-XY SDK. Enable this extension when building firmware. It requires the
checkpoint contract proposed in [Open-XY SDK PR #436](https://github.com/etcetc000/open-xy/pull/436).

| Hold Metronome, then press | Action |
| --- | --- |
| plus | Store or replace the checkpoint |
| minus | Recall the checkpoint |
| Record | Recover the state before the last recall |

Key repeats do nothing. An ordinary Metronome tap replays through stock on release.
Other Metronome combinations replay through stock. Browser, naming and sampling
screens keep their controls. Shift + Record remains stock's single-track undo.

The checkpoint holds the scene's active instrument and auxiliary patterns,
linked sound-source patterns, scene mutes/flags, and SDK control machines'
saved data. Notes, locks, components, length, scale, sounds, sample assignments,
and per-pattern mix parameters travel together. Samples must already be resident
when storing; the SDK retains their cache references without copying PCM.

One checkpoint is bound to its project and scene. Track/page changes preserve
it. Visiting another scene preserves it, but recall there is refused; return to
the original scene. Changed pattern counts require a new checkpoint. Loading
or creating a project clears both states. Failed stores preserve the checkpoint.

Recall refreshes each track through stock's pattern-paste path at the current
clock position. Held voices may be released as during stock pattern paste.
A redundant recall preserves the useful recovery state. Recovery works once per
reload and does not toggle between states. Finish recording before using any
checkpoint gesture.

Shared patterns and linked sound sources also affect other scenes that use
them. Tempo, song arrangement, global mixer/master settings, sample files and
transient DSP/voice state are outside this checkpoint. Saves/autosaves do not
move it; they still save the working project normally. The checkpoint itself
disappears on power-off.

The SDK reserves about 1 MiB for two compressed states and up to 64 KiB of SDK
machine data per state. Fewer machines may fit in a build. Each state's pattern
data has a 384 KiB compressed limit; exceeding either capacity refuses the action
without replacing the checkpoint or changing live patterns.

```
python ../open-xy/sdk.py test --extension .
python ../open-xy/sdk.py build --machine <machine-folder> --extension . --stock-os <external-stock>/os.ldr --out <external-build-folder>
```

The owner accepted the core store/recall/recovery workflow on hardware.
[QA.md](QA.md) records the tested image and remaining checks.
