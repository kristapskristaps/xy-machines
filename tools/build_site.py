"""Export a source-only machine gallery and authenticated browser auditions."""
import argparse
import hashlib
import html
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import xml.etree.ElementTree as ET
from effect_site import export_effect
from workspace import ROOT, checked_sdk, compiler, inventory, require

REPOSITORY = 'https://github.com/kristapskristaps/xy-machines'
KINDS = {'synth': 'Synthesizer', 'drum': 'Drum machine', 'effect': 'Send effect', 'sample': 'Sample engine'}
BROWSER = ('controls.mjs', 'engine.mjs', 'processor.mjs', 'display.mjs')
HARNESS = ('index.html', 'style.css', 'theme.mjs', 'knobs.mjs', 'audio-activation.mjs', 'audio-output.mjs', 'app.mjs', 'screen.mjs', 'processor.mjs')


def thumbnail(asset, metadata, words):
    """Apply the SDK's bounded drawing frame to its authored SVG, without a DOM."""
    graphics = metadata['graphics']
    art = ET.fromstring(asset)
    by_id = {node.get('id'): node for node in art.iter()}
    paths, points = graphics['paths'], graphics['points']
    offset = 0
    for path in range(paths):
        offset += 1  # retained path key
        xy = words[offset:offset + points * 2]
        offset += points * 2
        by_id[graphics['nodes'][path]].set('d', ' '.join(
            f'{"M" if i == 0 else "L"}{xy[i] / 1024} {xy[i + 1] / 1024}'
            for i in range(0, len(xy), 2)) + ' Z')
    offset = max(paths, 1) * (1 + 2 * points)
    palette = [by_id[name] for name in graphics['palette']]
    for name in graphics['nodes']:
        x, y, scale, cosine, sine, shade, paint = words[offset:offset + 7]
        offset += 7
        node = by_id[name]
        a, b = (scale * cosine // 1024) / 1024, (scale * sine // 1024) / 1024
        node.set('transform', f'matrix({a} {b} {-b} {a} {x / 1024} {y / 1024})')
        if paint:
            donor = palette[shade]
            node.set('stroke', donor.get('stroke'))
            node.set('stroke-width', donor.get('stroke-width'))
            if paint == 2 and node.get('fill', 'none') != 'none':
                node.set('fill', donor.get('fill'))
    for name in graphics['texts']:
        x, y, scale = words[offset:offset + 3]
        text = struct.pack('<6I', *(word & 0xffffffff for word in words[offset + 3:offset + 9])).split(b'\0', 1)[0].decode()
        offset += 9
        node = by_id[name]
        node.set('transform', f'matrix({scale / 1024} 0 0 {scale / 1024} {x / 1024} {y / 1024})')
        node.text = text
    svg = ET.Element('svg', xmlns='http://www.w3.org/2000/svg', width='480', height='220', viewBox='0 0 480 220')
    ET.SubElement(svg, 'rect', width='480', height='220', fill='#000000')
    svg.append(art)
    return ET.tostring(svg, encoding='unicode')


def export_player(sdk, preview, target, description):
    sys.path.insert(0, str(sdk))
    from tooling.preview import preview_bundle
    bundle = preview_bundle(preview)  # authenticate the exact compiled payloads
    for url, (_, data) in bundle.items():
        path = target / url.lstrip('/')
        path.parent.mkdir(parents=True, exist_ok=True)
        if url.endswith('/manifest.json'):
            metadata = json.loads(data)
            metadata = {key: metadata[key] for key in ('schema', 'kind', 'engine', 'description', 'definition', 'sample_rate', 'wasm_sha256', 'graphics')}
            metadata['description'] = description
            data = (json.dumps(metadata, indent=2) + '\n').encode()
        path.write_bytes(data)
    for name in BROWSER:
        path = target / 'sdk/browser' / name
        path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(sdk / 'sdk/browser' / name, path)
    for name in HARNESS:
        source = (sdk / 'harness' / name).read_text().replace('Open-XY Machine Workbench', 'XY Machines Player')
        if name == 'index.html':
            source = source.replace('="/harness/', '="./harness/').replace('href="/"', 'href="../../"')
            source = source.replace('XY Machines Player</a>', '← XY Machines</a>')
            source = source.replace('Preview Limitations', 'About this preview')
            source = source.replace('The audition applies its own simple amplitude gate after the engine.', 'Try the presets, then turn the controls to make the sound your own.')
            source = source.replace('Shared C++ sound and display. Device fonts, filters, envelopes and CPU timing still need hardware qualification.', 'This is a browser preview. Sound and display can differ on the OP-XY; see the machine guide for hardware testing status.')
            source = source.replace('Patch exports preserve every control. Rebuild the preview after changing the engine or display.', 'Export a patch to save your settings, or import one to pick up where you left off.')
            source = source.replace('</head>', '<link rel="stylesheet" href="player.css"></head>')
            path = target / 'index.html'
        else:
            source = source.replace("'/preview/", "'./preview/").replace("'/harness/", "'./harness/")
            path = target / 'harness' / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source)
    shutil.copyfile(ROOT / 'site/player.css', target / 'player.css')


def build(sdk, out, cxx):
    require(not out.exists(), 'Choose a fresh output directory.')
    require(out.is_relative_to(ROOT / 'local') or not out.is_relative_to(ROOT), 'Generated site belongs in ignored local/ or outside the checkout.')
    out.mkdir(parents=True)
    for name in ('style.css', 'app.mjs'):
        shutil.copyfile(ROOT / 'site' / name, out / name)
    revision = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip()
    machines = inventory()
    descriptions = json.loads((ROOT / 'site/descriptions.json').read_text())
    require(set(descriptions) == set(machines), 'Public descriptions must match the current inventory.')
    cards, records = [], []
    for ident, item in sorted(machines.items(), key=lambda pair: pair[1]['definition']['name'].casefold()):
        definition, metadata, folder = item['definition'], item['metadata'], item['path']
        name, kind = definition['name'], metadata['kind']
        require(kind in KINDS and re.fullmatch('[a-z][a-z0-9_]{0,14}', ident), 'Invalid public machine identity')
        description = descriptions[ident]
        require(isinstance(description, str) and description.strip(), 'Missing public machine description')
        relative = folder.relative_to(ROOT).as_posix()
        guide = f'{REPOSITORY}/blob/{revision}/{relative}/README.md'
        playable = kind in ('synth', 'drum', 'effect')
        record = dict(id=ident, name=name, kind=kind, description=description, guide=guide, playable=playable)
        if kind == 'effect':
            target = out / 'play' / ident
            export_effect(sdk, item, target, out.parent / (out.name + '-effect-build'), cxx, description, guide)
            record.update(player=f'play/{ident}/')
        if kind in ('synth', 'drum'):
            preview = out.parent / (out.name + '-previews') / ident
            subprocess.run([sys.executable, str(sdk / 'sdk.py'), 'preview', '--machine', str(folder), '--cxx', cxx, '--out', str(preview)], check=True)
            target = out / 'play' / ident
            export_player(sdk, preview, target, description)
            manifest = json.loads((preview / 'manifest.json').read_text())
            words = json.loads(subprocess.check_output(['node', str(ROOT / 'site/render_screen.mjs'), str(sdk), str(preview / 'engine.wasm'), str(preview / 'manifest.json')], text=True))
            thumb = out / 'screens' / (ident + '.svg')
            thumb.parent.mkdir(exist_ok=True)
            thumb.write_text(thumbnail((preview / 'panel.svg').read_text(), manifest, words))
            record.update(player=f'play/{ident}/', screen=f'screens/{ident}.svg')
        if playable:
            licenses = target / 'licenses'
            licenses.mkdir()
            shutil.copyfile(sdk / 'LICENSE', licenses / 'SDK-LICENSE.txt')
            for path in folder.rglob('*'):
                if path.is_file() and (path.name.upper().startswith(('LICENSE', 'NOTICE')) or path.name in ('VENDOR.md', 'VENDOR.json')):
                    destination = licenses / path.relative_to(folder)
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(path, destination)
        escape = html.escape
        data = f'data-name="{escape(name, quote=True)}" data-guide="{escape(guide, quote=True)}"'
        image = f'<a class="machine-screen" href="{record["player"]}" data-play {data} aria-label="Play {escape(name, quote=True)}"><img src="{record["screen"]}" alt="{escape(name, quote=True)} instrument screen" width="480" height="220" loading="lazy"></a>' if 'screen' in record else ''
        play = f'<a href="{record["player"]}" data-play {data}>{'Try effect' if kind == 'effect' else 'Play machine'} ↗</a>' if playable else ''
        presets = len(definition.get('presets', {}))
        detail = f' · {presets} {"preset" if presets == 1 else "presets"}' if presets else ''
        search = f'{name} {ident} {description}'.casefold()
        cards.append(f'<article class="machine-card {"" if "screen" in record else "documentation"}" data-kind="{kind}" data-search="{escape(search, quote=True)}">{image}<p class="machine-meta">{KINDS[kind]}{detail}</p><h3>{escape(name)}</h3><p class="machine-description">{escape(description)}</p><div class="machine-links">{play}<a href="{escape(guide, quote=True)}">Read guide ↗</a></div></article>')
        records.append(record)
    featured = next(record for record in records if record['id'] == 'acidline')
    feature = f'<div class="featured"><a class="featured-screen" href="{featured["player"]}" data-play data-name="Acidline" data-guide="{html.escape(featured["guide"], quote=True)}" aria-label="Play Acidline"><img src="{featured["screen"]}" alt="Acidline’s monochrome liquid display" width="480" height="220" fetchpriority="high"></a><div class="featured-caption"><div><h2>Acidline</h2><p>Resonant bass, liquid movement.<br>Take the controls for a spin.</p></div><a class="play-link" href="{featured["player"]}" data-play data-name="Acidline" data-guide="{html.escape(featured["guide"], quote=True)}">Play Acidline <span aria-hidden="true">↗</span></a></div></div>'
    page = (ROOT / 'site/index.html').read_text().replace('{{CARDS}}', '\n'.join(cards)).replace('{{COUNT}}', str(len(records))).replace('{{FEATURED}}', feature)
    (out / 'index.html').write_text(page)
    (out / '.nojekyll').write_text('')
    (out / 'machines.json').write_text(json.dumps(dict(source=f'{REPOSITORY}/tree/{revision}', sdk=(ROOT / 'SDK_REVISION').read_text().strip(), machines=records), indent=2) + '\n')
    files = {path.relative_to(out).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest() for path in out.rglob('*') if path.is_file()}
    require(not any(Path(name).suffix.lower() in ('.ldr', '.tfw', '.zfw', '.elf', '.o', '.bin', '.zip') for name in files), 'Private/native output in public site')
    (out / 'site-files.json').write_text(json.dumps(files, indent=2) + '\n')
    print(f'PASS: {len(records)} catalog entries, {sum(record["playable"] for record in records)} hashed auditions. Static site: {out}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cxx', default=compiler())
    args = parser.parse_args()
    build(checked_sdk(args.sdk), args.out.resolve(), args.cxx)


if __name__ == '__main__':
    main()
