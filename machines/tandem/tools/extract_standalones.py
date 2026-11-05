"""Extract fixed-model siblings from the frozen, optimized Weave source.

Run from the collection root with --source pointing to the clean 957afdb
worktree. Generated machines remain independently buildable and licensed.
"""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
from pathlib import Path

REVISION = '957afdbac6f2ef9916991366e24517f1483dc0e4'
MODELS = {
    'analog': (11, 'MACRO_VA', 'macro_va', 'va'),
    'waves': (10, 'MACRO_WAVETABLE', 'macro_wavetable', 'wt'),
    'chords': (8, 'MACRO_CHORD', 'macro_chord', 'chord'),
    'grain': (7, 'MACRO_GRAIN', 'macro_grain', 'grain'),
    'shaper': (0, 'MACRO_WSH', 'macro_wsh', 'wsh'),
}


def masked(source):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), source)


def close_brace(source, start):
    code = masked(source)
    depth = 0
    for i in range(start, len(code)):
        depth += (code[i] == '{') - (code[i] == '}')
        if not depth:
            return i
    raise ValueError('Unbalanced source block')


def remove_function(source, name):
    code = masked(source)
    pattern = r'(?m)^[^\n;{}]*\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{'
    while True:
        match = re.search(pattern, code)
        if not match:
            return source
        start = match.start()
        previous = source[:start].rstrip('\n').rfind('\n') + 1
        if source[previous:start].lstrip().startswith('template'):
            start = previous
        end = close_brace(source, match.end() - 1) + 1
        source = source[:start] + source[end:]
        code = masked(source)


def one_case(source, switch, model):
    start = source.index(switch)
    opening = source.index('{', start)
    end = close_brace(source, opening)
    body = source[opening + 1:end]
    labels = list(re.finditer(r'(?m)^\s*case (MACRO_\w+):', body))
    default = re.search(r'(?m)^\s*default:', body)
    for i, label in enumerate(labels):
        if label[1] == model:
            stop = labels[i + 1].start() if i + 1 < len(labels) else (default.start() if default else len(body))
            selected = body[label.start():stop]
            tail = body[default.start():] if default else ''
            return source[:opening + 1] + '\n' + selected + tail + source[end:]
    raise ValueError('Selected model case missing')


def prune(source, model):
    source = re.sub(r'(?m)^#define MACRO_SEL [^\n]*\n[^\n]*', '#define MACRO_SEL ' + model, source, count=1)
    source = source.replace('#define MACRO_MASK 0x7fff', '#define MACRO_MASK (1u << ' + model + ')')
    source = one_case(source, 'switch (m->engine)', model)
    source = one_case(source, 'switch (Model >= 0 ? Model : m->engine)', model)
    start = source.index('        if (HAS(MACRO_FM) && m->engine == MACRO_FM)', source.index('static void macro_render_e', source.index('#define CL2')))
    end = source.index('    }\n    const uint32_t maximum_inc', start)
    source = source[:start] + source[end:]
    for function in ('particle_render', 'bd_render', 'sd_render', 'hh_render', 'swarm_render', 'fm_native_rate', 'macro_render_fm_native'):
        source = remove_function(source, function)
    if model != 'MACRO_WSH':
        source = remove_function(source, 'macro_render_device32')
    if model != 'MACRO_CHORD':
        for function in ('macro_render_chord_device32', 'chord_predict64', 'macro_chord_ready64',
                         'macro_render_chord_try64', 'macro_render_chord_device64', 'chord_q31', 'macro_render_chord_q31'):
            source = remove_function(source, function)
    return source


def trim_unused(target, cxx, sdk):
    """Remove unreachable static DSP, then headers no remaining code needs."""
    cpp = target / 'vendor/macro.cpp'
    source = cpp.read_text()
    code = re.sub(r'__attribute__\s*\(\([^)]*\)\)', lambda m: ' ' * len(m[0]), masked(source))
    functions = {}
    for match in re.finditer(r'(?m)^[ \t]*static [^\n;{}]*?\b(\w+)\s*\([^;{}]*\)\s*\{', code):
        end = close_brace(source, match.end() - 1)
        functions.setdefault(match[1], set()).update(re.findall(r'\b\w+\b', code[match.end():end]))
    roots = set()
    for name in ('macro_init', 'macro_trig', 'macro_engine_of', 'macro_inc_of_note', 'macro_render',
                 'macro_render_device32', 'macro_render_chord_device32', 'macro_chord_ready64',
                 'macro_render_chord_try64', 'macro_render_chord_device64', 'macro_render_chord_q31'):
        match = re.search(r'(?m)^[^\n;{}]*\b' + name + r'\s*\([^;{}]*\)\s*\{', code)
        if match:
            end = close_brace(source, match.end() - 1)
            roots.update(re.findall(r'\b\w+\b', code[match.end():end]))
    for macro in re.finditer(r'(?m)^#define [^\n]*(?:\\\n[^\n]*)*', code):
        roots.update(re.findall(r'\b\w+\b', macro[0]))
    reached = set()
    pending = roots & functions.keys()
    while pending:
        name = pending.pop()
        reached.add(name)
        pending.update((functions[name] & functions.keys()) - reached)
    for name in functions.keys() - reached:
        source = remove_function(source, name)
    cpp.write_text(source)
    command = [cxx, '-std=c++11', '-O2', '-Wall', '-Werror', '-ferror-limit=0',
               '-I' + str(sdk / 'sdk/include'), '-fsyntax-only', str(target / 'engine.cpp')]
    for _ in range(24):
        result = subprocess.run(command, capture_output=True, text=True)
        if not result.returncode:
            break
        before = cpp.read_text()
        after = before
        for name in set(re.findall(r"unused function '([^']+)'", result.stderr)):
            after = remove_function(after, name)
        for name in set(re.findall(r"unused (?:const )?variable '([^']+)'", result.stderr)):
            match = re.search(r'(?m)^static const [^\n]*\b' + re.escape(name) + r'\s*(?:\[[^\]]*\])*\s*=\s*\{', after)
            if match:
                end = close_brace(after, match.end() - 1) + 1
                end = after.index(';', end) + 1
                after = after[:match.start()] + after[end:]
        if before == after:
            raise ValueError(result.stderr)
        cpp.write_text(after)
    else:
        raise ValueError('Static DSP pruning did not converge')
    for header in sorted((target / 'vendor').glob('macro_*.h')):
        # macro.h declares MACRO_SINE extern, so syntax-only analysis cannot
        # detect a missing definition after removing the general table file.
        if header.name == 'macro_tables.h':
            continue
        source = cpp.read_text()
        line = '#include "' + header.name + '"'
        if line not in source:
            continue
        cpp.write_text(source.replace(line, ''))
        if subprocess.run(command, capture_output=True).returncode:
            cpp.write_text(source)
        else:
            header.unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--cxx', default='clang++')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    source_root = args.source.resolve()
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=source_root, text=True).strip()
    if revision != REVISION or subprocess.check_output(['git', 'status', '--porcelain'], cwd=source_root, text=True).strip():
        raise ValueError('Use the clean frozen Weave revision ' + REVISION)
    original = source_root / 'machines/weave'
    for short, (number, model, state_type, member) in MODELS.items():
        mid = 'weave_' + short
        target = root / mid
        vendor = target / 'vendor'
        shutil.copytree(original / 'vendor', vendor, dirs_exist_ok=True)
        cpp = prune((vendor / 'macro.cpp').read_text(), model)
        header = (vendor / 'macro.h').read_text()
        header = re.sub(r'    union \{[\s\S]*?    \} e;', '    union { struct ' + state_type + ' ' + member + '; } e;', header, count=1)
        for path in vendor.iterdir():
            if path.suffix in ('.h', '.cpp'):
                content = cpp if path.name == 'macro.cpp' else header if path.name == 'macro.h' else path.read_text()
                path.write_text(content.replace('weave_dsp', mid + '_dsp'))
        shutil.copyfile(original / 'LICENSE', target / 'LICENSE')
        if number not in (0, 8):
            (target / 'resample.h').write_text((original / 'resample.h').read_text().replace('namespace weave {', 'namespace ' + mid + ' {'))
        core = '''#pragma once
#include <opxy/audio.hpp>
#include "vendor/macro.h"
namespace ID {
struct Voice {
    ID_dsp::macro_voice oscillator;
    opxy::u32 pending;
    opxy::i16 cache[32];
CONVERTER
};
struct Shared {};
void initialize(Voice&,Shared&,opxy::u32);
void start(Voice&,Shared&,opxy::u32);
void destroy(Voice&,Shared&,opxy::u32);
void render(Voice&,Shared&,opxy::u32,opxy::Output,opxy::Parameters,opxy::Pitch);
}
'''.replace('CONVERTER', '' if number in (0, 8) else '    opxy::i16 history[48];\n    opxy::u32 cache_read,history_write,resample_phase;').replace('ID', mid)
        (target / 'core.h').write_text(core)
        engine = (original / 'engine.cpp').read_text()
        engine = engine.replace('namespace weave {', 'namespace ' + mid + ' {').replace('weave_dsp', mid + '_dsp')
        engine = remove_function(engine, 'model')
        engine = engine.replace('const unsigned selected = model(parameters[0]);', 'const unsigned selected = ' + str(number) + ';')
        engine = engine.replace('voice.pending || selected != voice.model', 'voice.pending != 0')
        engine = re.sub(r'    const bool clock_changed = [^\n]+\n', '', engine)
        engine = re.sub(r'    voice.model = selected;\n', '', engine)
        engine = engine.replace('controls[0] = static_cast<opxy::u8>(selected * 8u);', 'controls[0] = 0;')
        engine = engine.replace('normalized(parameters[i]) >> 8', 'normalized(parameters[i-1]) >> 8')
        engine = engine.replace('normalized(parameters[4]) >> 8', 'normalized(parameters[3]) >> 8')
        engine = re.sub(r'    controls\[5\].*\n    controls\[6\].*\n', '', engine)
        engine = re.sub(r'        // Zero keeps[\s\S]*?if \(seed\) voice.oscillator.rng = [^\n]+\n', '', engine)
        engine = engine.replace('    voice.model = 0;\n', '')
        if number not in (0, 8):
            engine = remove_function(engine, 'render_chord_native')
            start = engine.index('    // Redesigned periodic models')
            stop = engine.index('    // Keep the source DSP', start)
            engine = engine[:start] + engine[stop:]
            engine = engine.replace('    const unsigned selected = ' + str(number) + ';\n', '').replace('    const bool native_clock = selected == 0 || selected == 1 || selected == 8;\n', '')
        else:
            engine = engine.replace('#include "resample.h"\n', '')
            engine = remove_function(engine, 'advance_source')
            engine = re.sub(r'    voice.cache_read=32;voice.history_write=voice.resample_phase=0;\n    for\(auto& sample:voice.history\)sample=0;\n', '', engine)
            engine = engine.replace('        voice.cache_read=32;\n', '')
            start = engine.index('    // Redesigned periodic models')
            engine = engine[:start] + ('    render_chord_native(voice,controls,pitch,output);\n}\n}\n' if number == 8 else '''    for(unsigned offset=0;offset<opxy::block_frames;offset+=32) {
        ID_dsp::macro_render_device32(&voice.oscillator,controls,pitch[offset],voice.cache);
        for(unsigned i=0;i<32;++i)output[offset+i]=static_cast<opxy::i32>(voice.cache[i])*16384;
    }
}
}
'''.replace('ID', mid))
            engine = engine.replace('    const unsigned selected = ' + str(number) + ';\n', '').replace('    const bool native_clock = selected == 0 || selected == 1 || selected == 8;\n', '')
            if number == 0:
                engine = remove_function(engine, 'render_chord_native')
        (target / 'engine.cpp').write_text(engine)
        trim_unused(target, args.cxx, args.sdk)
        (vendor / 'VENDOR.md').write_text('''# DSP provenance

Plaits by Émilie Gillet (MIT), via the attributed DMDaudio integer port.
See LICENSE and the copyright notices retained in macro.cpp.

This fixed-model extraction uses XY Machines revision ''' + REVISION + '''.
It fixes the model dispatch, retains only that model in the voice union,
isolates public symbols in its own namespace, and removes unused entry points.
Controls map directly to Weave Harmonic, Timbre, Morph and Output. DSP formulas,
gains, physical clocks and the optimized Chords path are retained. Other models
are not selectable. Host regression fixtures bind audio to the frozen Weave
revision; hardware CPU for this new machine remains unmeasured.
''')
        provenance = dict(source_revision=REVISION, source_model=number, adaptation=True,
                          upstream='https://github.com/pichenettes/eurorack',
                          files={f.name: hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(vendor.iterdir()) if f.name != 'VENDOR.json'})
        (vendor / 'VENDOR.json').write_text(json.dumps(provenance, indent=2) + '\n')
        print(mid + ': selected state and reachable DSP extracted', flush=True)
    print('Extracted five fixed-model DSP adapters; artwork and regression fixtures are separate.')


if __name__ == '__main__':
    main()
