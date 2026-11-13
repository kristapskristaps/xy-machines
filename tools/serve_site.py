"""Serve only a generated gallery's audited file allowlist on localhost."""
import argparse
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import mimetypes
from pathlib import Path
from urllib.parse import unquote, urlsplit


def snapshot(root):
    manifest = json.loads((root / 'site-files.json').read_text())
    files = {}
    for name, expected in manifest.items():
        path = root / name
        if path.resolve() != path or not path.is_relative_to(root) or not path.is_file():
            raise ValueError('Redirected or missing exported file: ' + name)
        data = path.read_bytes()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError('Changed exported file: ' + name)
        mime = {'.wasm':'application/wasm', '.mjs':'text/javascript', '.svg':'image/svg+xml'}.get(path.suffix, mimetypes.guess_type(name)[0] or 'application/octet-stream')
        files[name] = mime, data
    return files


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.respond(False)

    def do_HEAD(self):
        self.respond(True)

    def respond(self, head):
        route = unquote(urlsplit(self.path).path)
        if route == '/':
            self.send_response(302)
            self.send_header('Location', self.server.prefix)
            self.end_headers()
            return
        if not route.startswith(self.server.prefix):
            self.send_error(404)
            return
        name = route[len(self.server.prefix):]
        if route.endswith('/'):
            name += 'index.html'
        asset = self.server.assets.get(name)
        if not asset:
            self.send_error(404)
            return
        mime, data = asset
        self.send_response(200)
        self.send_header('Content-Type', mime)
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Cache-Control', 'no-store')
        self.send_header('X-Content-Type-Options', 'nosniff')
        self.send_header('Content-Security-Policy', "default-src 'self'; script-src 'self' 'wasm-unsafe-eval'; style-src 'self'; img-src 'self' data:; frame-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'self'")
        self.end_headers()
        if not head:
            self.wfile.write(data)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--site', required=True, type=Path)
    parser.add_argument('--port', type=int, default=0)
    args = parser.parse_args()
    server = ThreadingHTTPServer(('127.0.0.1', args.port), Handler)
    server.assets = snapshot(args.site.resolve())
    server.prefix = '/xy-machines/'
    print(f'Gallery: http://127.0.0.1:{server.server_port}{server.prefix}', flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
