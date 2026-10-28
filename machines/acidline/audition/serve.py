"""Serve an authenticated SDK preview with Acidline's explicit audition allowlist."""
import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--sdk', type=Path, default=ROOT.parent / 'open-xy')
parser.add_argument('--preview', type=Path, required=True)
parser.add_argument('--port', type=int, default=8947)
args = parser.parse_args()
sys.path.insert(0, str(args.sdk.resolve()))
from tools.serve_harness import PreviewServer, Handler
from tooling.preview import preview_bundle

bundle = preview_bundle(args.preview.resolve())
for route, name, mime in [('/', 'index.html', 'text/html'),
                         ('/acidline/app.mjs', 'app.mjs', 'text/javascript'),
                         ('/acidline/patterns.mjs', 'patterns.mjs', 'text/javascript'),
                         ('/acidline/processor.mjs', 'processor.mjs', 'text/javascript'),
                         ('/acidline/style.css', 'style.css', 'text/css')]:
    path = ROOT / 'audition' / name
    if path.is_symlink():
        raise ValueError('Audition assets must be ordinary files')
    bundle[route] = (mime + '; charset=utf-8', path.read_bytes())
server = PreviewServer(('127.0.0.1', args.port), Handler)
server.preview = bundle
print(f'Acidline audition: http://127.0.0.1:{args.port}/', flush=True)
try:
    server.serve_forever()
except KeyboardInterrupt:
    pass
finally:
    server.server_close()
