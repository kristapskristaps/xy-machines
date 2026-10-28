# Braids folding transfer

`braids_fold.h` contains only `ws_tri_fold` from Emilie Gillet’s MIT-licensed Braids resources.

Source: https://github.com/etcetc000/open-xy-core-machines/blob/4bc104f14856500fecc5eee72be51eb952b67942/machines/macroosc/vendor/braids/resources.cc

Original upstream: pichenettes/eurorack at `08460a69a7e1f7a81c5a2abcc7189c9a6b7208d4`.

The table is renamed and wrapped in a namespace; values are unchanged. `resources/generate_tables.py` projects the transfer into pitch-limited periodic waves, removes DC and even partials from the odd branch, and normalizes its level. All other synthesis and graphics are Westfold code.
