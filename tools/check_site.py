"""Check a generated Pages artifact, including subpath URLs and public payloads."""
import argparse
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit
from workspace import require
from check_private_files import content_problem
from serve_site import snapshot


class References(HTMLParser):
    def __init__(self):
        super().__init__()
        self.references = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        for key in ('href', 'src'):
            if attrs.get(key):
                self.references.append(attrs[key])


def check(root):
    files = snapshot(root)
    catalog = json.loads((root / 'machines.json').read_text())
    records = catalog['machines']
    require(len({record['id'] for record in records}) == len(records), 'Duplicate gallery entry')
    for name, (_, data) in files.items():
        require(Path(name).suffix.lower() not in ('.ldr', '.tfw', '.zfw', '.elf', '.bin', '.o', '.zip'), 'Native/private payload: ' + name)
        require(content_problem(data) is None, 'Private or executable payload: ' + name)
        if name.endswith(('.html', '.json', '.mjs')):
            text = data.decode()
            require(not any(value in text for value in ('/Users/', '/home/runner/', '127.0.0.1:')), 'Local metadata leaked into public assets: ' + name)
        if name.endswith('.html'):
            parser = References()
            parser.feed(data.decode())
            for reference in parser.references:
                url = urlsplit(reference)
                if url.scheme or url.netloc or reference.startswith('#'):
                    continue
                require(not url.path.startswith('/'), 'Root-relative URL breaks project Pages: ' + reference)
                target = (root / name).parent / unquote(url.path)
                if not url.path or url.path.endswith('/'):
                    target /= 'index.html'
                require(target.resolve().is_relative_to(root) and target.is_file(), 'Broken local URL: ' + reference)
    for record in records:
        if not record['playable']:
            continue
        folder = root / record['player']
        manifest = json.loads((folder / 'preview/manifest.json').read_text())
        require(not {'root', 'compiler', 'sources'} & manifest.keys(), 'Public audition contains local build metadata')
        require(hashlib.sha256((folder / 'preview/engine.wasm').read_bytes()).hexdigest() == manifest['wasm_sha256'], 'Changed audition audio payload')
        require(hashlib.sha256((folder / 'preview/panel.svg').read_bytes()).hexdigest() == manifest['graphics']['asset_sha256'], 'Changed audition artwork payload')
        app = (folder / 'harness/app.mjs').read_text()
        require("fetch('./preview/manifest.json')" in app and "addModule('./harness/processor.mjs')" in app,
                'Audition no longer resolves within its own project subpath')
        require(not re.search(r"['\"]/preview/|['\"]/harness/", app), 'Root-relative audition resource')
    print(f'PASS: {len(records)} entries, {sum(record["playable"] for record in records)} hashed auditions, local URLs and public file allowlist.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--site', type=Path, required=True)
    check(parser.parse_args().site.resolve())
