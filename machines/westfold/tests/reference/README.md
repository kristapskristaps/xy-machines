Frozen exact Westfold DSP from97b6a9cc011169159625186d57b5e040e31dfee7,
namespace-renamed to westfold_reference only. Tables alias the unchanged
production raw table data; original golden literals independently enforce it.
The current intentional blend approximation is checked in quality.cpp and
tools/fidelity.py. Do not replace original goldens to call it bit-identical.
