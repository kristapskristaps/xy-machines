# X0X authored kits

These seven native OP-XY kit presets preserve all 24 pad records: 808 Classic
(`default.json`), 606 Dry, 707 Studio, 909 Club, Night Wire, Warehouse and Soft
Machines. They are separate from the public SDK builder's X0X init preset.

The public builder accepts an empty-region instrument preset and supplies
X0X's default pad mapping through its drum API. It cannot bundle these full
packed-pad presets. This pack retains them without making the engine depend
on SDK PR #422. The browser audition still offers all seven.

The folder `content/presets/x0x/` is laid out for the SDK's existing device
transfer command. It contains no firmware, samples or stock presets. Install
it separately after the matching X0X engine, with backups/readback and explicit
device authorization. It replaces same-named presets, including the builder's
init; do not overwrite a user's edited sounds without arranging that first.
The regular qualification transfer excludes presets and leaves existing kits
untouched. Public-SDK hardware import/save/reload QA remains pending.

Regenerate with `python machines/x0x/tools/generate_presets.py --sdk <pinned-sdk>`
from the collection root. Host integration tests decode every pad through the
public SDK codec and compare its values with the seven browser kits.
