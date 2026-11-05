# Crease optimization

The selected change forces the wave-shaper renderer inline, exposing its fixed
block size and model to the Blackfin compiler. The 45 frozen PCM/state scenes
and the display test pass unchanged. Native compilation passes.

| Paired image | Baseline | Candidate |
| --- | ---: | ---: |
| First trial, selected | 3.628595% | 3.515679% |
| Additional helper inlining, discarded | 3.799496% | 3.524834% |

Each row comes from a complete hardware control sweep containing both baseline
and candidate. The images also contain an unchanged Choir reference. Timing
changes between image layouts; the second trial did not improve the absolute
result, so the smaller first version is retained. The selected version is
0.015679 percentage points above the recommended 3.5%. Its midpoint and
Alt mix at midpoint are the heaviest measured setting.

The generated report binds the selected machine files at commit
`06af6c4`. All 43 authored factory presets across the seven-machine collection
were installed and readback-verified. Full preset and project QA remains
separate from this CPU measurement. Firmware, captures and backups remain
private outside the repository. No publication has been performed.
