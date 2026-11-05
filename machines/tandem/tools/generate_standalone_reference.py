"""Regenerate standalone audio fixtures with the unchanged optimized Weave.

All host objects go to a fresh external --out directory. Only the resulting
portable audio/state hashes are written into the five machine folders.
"""
import argparse
from pathlib import Path
import subprocess

from extract_standalones import MODELS, REVISION


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    args = parser.parse_args()
    source = args.source.resolve()
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=source, text=True).strip()
    if revision != REVISION or subprocess.check_output(['git', 'status', '--porcelain'], cwd=source, text=True).strip():
        raise ValueError('Use the clean frozen source revision ' + REVISION)
    args.out.mkdir(parents=True, exist_ok=False)
    probe = subprocess.run(['git', '-C', str(args.out), 'rev-parse', '--show-toplevel'], capture_output=True)
    if probe.returncode == 0:
        raise ValueError('Keep generated host objects outside every checkout')
    root = Path(__file__).resolve().parents[2]
    template = Path(__file__).with_name('standalone_reference.cpp.in').read_text()
    for short, (model, _, _, member) in MODELS.items():
        target = root / ('weave_' + short)
        cpp = args.out / (short + '.cpp')
        cpp.write_text(template.replace('@SOURCE@', str(source / 'machines/weave/engine.cpp'))
                       .replace('@CASES@', str(target / 'tests/cases.h'))
                       .replace('@MODEL@', str(model)).replace('@MEMBER@', member))
        binary = args.out / short
        subprocess.run([args.cxx, '-std=c++11', '-O2', '-Wall', '-Werror',
                        '-I' + str(args.sdk.resolve() / 'sdk/include'), str(cpp), '-o', str(binary)], check=True)
        result = subprocess.check_output([str(binary)])
        (target / 'tests/weave_reference.h').write_bytes(result)
        print('Frozen Weave fixtures:', short, flush=True)


if __name__ == '__main__':
    main()
