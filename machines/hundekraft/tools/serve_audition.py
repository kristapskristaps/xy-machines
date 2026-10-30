#!/usr/bin/env python3
"""Serve only authenticated audition artifacts and SDK-allowlisted synth previews."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import mimetypes
import re
import sys
import threading
from urllib.parse import unquote,urlsplit
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/'tools'))
from workspace import sdk_path
SDK=sdk_path()
sys.path.insert(0,str(SDK))
from tooling.preview import preview_bundle
spec=importlib.util.spec_from_file_location('sdk_preview_server',SDK/'tools/serve_harness.py')
preview_server=importlib.util.module_from_spec(spec);spec.loader.exec_module(preview_server)

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--folder',required=True,type=Path);parser.add_argument('--port',default=9097,type=int);args=parser.parse_args()
    folder=args.folder.resolve();manifest=json.loads((folder/'audition.json').read_text());files={}
    for name,expected in manifest['files'].items():
        if not re.fullmatch(r'(?:audio/[a-z0-9]+\.wav|live/[a-z0-9]+\.wasm|visuals/[a-z0-9]+\.(?:wasm|svg|json)|(?:index|gallery)\.html|(?:app|worklet|visuals|visual-host|display|controls)\.mjs|style\.css|machines\.json)',name):raise ValueError('Unexpected public artifact: '+name)
        p=folder/name
        if p.resolve()!=p.absolute() or digest(p)!=expected:raise ValueError('Changed public artifact: '+name)
        files['/'+name]=(mimetypes.guess_type(name)[0] or 'application/octet-stream',p.read_bytes())
    files['/']=files['/index.html']
    for name,expected in manifest['source'].items():
        p=Path(name)
        if not p.resolve().is_relative_to(ROOT) or digest(p)!=expected:raise ValueError('Sources changed; rebuild audition: '+name)
    for name,expected in manifest.get('sdkSource',{}).items():
        p=Path(name)
        if not p.resolve().is_relative_to(SDK) or digest(p)!=expected:raise ValueError('SDK display host changed; rebuild audition: '+name)
    machines=json.loads((folder/'machines.json').read_text())
    servers=[]
    for i,m in enumerate(m for m in machines if m['kind']=='synth'):
        server=preview_server.PreviewServer(('127.0.0.1',args.port+i+1),preview_server.Handler)
        server.preview=preview_bundle(folder/'previews'/m['id']);servers.append(server)
    class Handler(BaseHTTPRequestHandler):
        def do_HEAD(self):self.respond(True)
        def do_GET(self):self.respond(False)
        def respond(self,head):
            artifact=files.get(unquote(urlsplit(self.path).path))
            if artifact is None:self.send_error(404);return
            mime,data=artifact;start=0;end=len(data)-1
            match=re.fullmatch(r'bytes=(\d+)-(\d*)',self.headers.get('Range',''))
            if match:
                start=int(match[1]);end=min(end,int(match[2])) if match[2] else end
                if start>end:self.send_error(416);return
            self.send_response(206 if match else 200)
            self.send_header('Content-Type',mime);self.send_header('Content-Length',str(end-start+1))
            self.send_header('Accept-Ranges','bytes');self.send_header('Cache-Control','no-store')
            self.send_header('X-Content-Type-Options','nosniff')
            self.send_header('Content-Security-Policy',"default-src 'self'; script-src 'self' 'wasm-unsafe-eval'; style-src 'self'; object-src 'none'; base-uri 'none'")
            if match:self.send_header('Content-Range',f'bytes {start}-{end}/{len(data)}')
            self.end_headers()
            if not head:self.wfile.write(data[start:end+1])
        def log_message(self,*args):pass
    root=ThreadingHTTPServer(('127.0.0.1',args.port),Handler)
    for s in servers:threading.Thread(target=s.serve_forever,daemon=True).start()
    print(f'Hundekraft auditions: http://127.0.0.1:{args.port}',flush=True)
    try:root.serve_forever()
    except KeyboardInterrupt:pass
    finally:
        root.server_close()
        for s in servers:s.shutdown();s.server_close()
if __name__=='__main__':main()
