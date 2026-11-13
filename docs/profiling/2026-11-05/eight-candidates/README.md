# Eight-candidate hardware qualification — 5 November 2026

Source: `c104047` with SDK `d8db0c63d8cd2cebf1b4b8c3b84855b936628b05` ([SDK PR #422](https://github.com/etcetc000/open-xy/pull/422), not yet merged).

All eight are installed and boot normally. The original startup SVGs hung during screen creation. Integer polyline startup geometry, full-pixel strokes and unfilled open contours resolved the failure with byte-identical firmware. This isolates the failure to the original artwork; the individual parser edge case has not been isolated. DSP and runtime display callbacks were unchanged.

One complete standard SDK hardware capture returned all 406 expected cases with no protocol errors. All eight revision-bound reports were accepted. These synthetic results exclude graphics and total project CPU; X0X uses the drum callback scope, while the synths include the stock whole-voice path.

| Machine | Maximum | Target | Evidence |
| --- | ---: | --- | --- |
| Westfold | 6.511% | Above 3.5% | [Report](westfold/report.md) |
| X0X | 5.344% | Above 3.5% | [Report](x0x/report.md) |
| Bitflip | 7.839% | Above 3.5% | [Report](bitflip/report.md) |
| Tandem | 9.990% | Above 3.5% | [Report](weave_analog/report.md) |
| Atlas | 5.661% | Above 3.5% | [Report](weave_waves/report.md) |
| Choir | 3.423% | Within 3.5% | [Report](weave_chords/report.md) |
| Sift | 8.368% | Above 3.5% | [Report](weave_grain/report.md) |
| Crease | 3.684% | Above 3.5% | [Report](weave_shaper/report.md) |

The software reboot after profiling produced an observed USB-MIDI disconnect/reconnect; normal OS 1.2.1 identity returned after startup settling. The fixed eight-machine bank remains installed. Presets and projects were not transferred or deleted, and all 42 saved machine-ID reservations were preserved.

Host previews, machine sound/display tests, per-machine Blackfin compilation and source-rebuild checks passed. All collection host checks passed against the pinned SDK; the seven synths also pass public-staging host/preview/native checks. The required Windows SDK release-host qualification remains pending.

Release remains pending: seven machines exceed the CPU target, and full per-machine `QA.md` passes (controls, locks/LFOs, patterns, presets, save/reload and normal playback) have not been completed. A successful boot and benchmark do not mark those checks passed. Choir meets the measured CPU target but still needs device QA and catalog/release preparation. No new catalog publication or release tags were created.
