"""Build and serve Halo's own effect audition. Serves an explicit generated-file allowlist."""
import argparse,hashlib,http.server,json,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
NAMES=('index.html','app.mjs','worklet.mjs','halo.wasm','definition.json')
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def build(args):
 out=args.out.resolve()
 if out.exists():raise ValueError('Choose a fresh output directory')
 out.mkdir(parents=True)
 try:
  command=[args.cxx,'--target=wasm32','-std=c++11','-O2','-fno-exceptions','-fno-rtti','-ffreestanding','-fno-builtin','-nostdlib','-I'+str(args.sdk.resolve()/'sdk/include'),str(ROOT/'audition/bridge.cpp'),'-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=1048576','-Wl,-z,stack-size=16384']
  command+=['-Wl,--export='+name for name in ('halo_left','halo_right','halo_parameters','halo_reset','halo_process')]
  subprocess.run(command+['-o',str(out/'halo.wasm')],check=True)
  for name in NAMES:
   if name!='halo.wasm':shutil.copyfile(ROOT/('definition.json' if name=='definition.json' else 'audition/'+name),out/name)
  manifest={'files':{name:digest(out/name) for name in NAMES},'source':{name:digest(ROOT/name) for name in ('fx.cpp','core.h','tables.h','definition.json','audition/bridge.cpp','audition/index.html','audition/app.mjs','audition/worklet.mjs')}}
  (out/'audition.json').write_text(json.dumps(manifest,indent=2)+'\n')
 except BaseException:
  shutil.rmtree(out);raise
 print('Built Halo audition:',out)
def serve(args):
 folder=args.serve.resolve();manifest=json.loads((folder/'audition.json').read_text())
 if set(manifest['files'])!=set(NAMES):raise ValueError('Unexpected audition files')
 for name,expected in manifest['files'].items():
  if digest(folder/name)!=expected:raise ValueError('Audition file changed: '+name)
 for name,expected in manifest['source'].items():
  if digest(ROOT/name)!=expected:raise ValueError('Audition is stale; rebuild: '+name)
 class Handler(http.server.BaseHTTPRequestHandler):
  def do_GET(self):
   name=self.path.split('?',1)[0].removeprefix('/') or 'index.html'
   if name not in NAMES:self.send_error(404);return
   data=(folder/name).read_bytes()
   kind={'html':'text/html; charset=utf-8','mjs':'text/javascript; charset=utf-8','json':'application/json','wasm':'application/wasm'}[name.rsplit('.',1)[1]]
   self.send_response(200);self.send_header('Content-Type',kind);self.send_header('Content-Length',str(len(data)));self.send_header('Cache-Control','no-store');self.send_header('X-Content-Type-Options','nosniff');self.end_headers();self.wfile.write(data)
  def log_message(self,*args):pass
 print(f'Halo audition: http://127.0.0.1:{args.port}',flush=True)
 http.server.ThreadingHTTPServer(('127.0.0.1',args.port),Handler).serve_forever()
p=argparse.ArgumentParser(description=__doc__)
g=p.add_mutually_exclusive_group(required=True);g.add_argument('--out',type=Path);g.add_argument('--serve',type=Path)
p.add_argument('--sdk',type=Path,default=ROOT.parents[2] / 'open-xy');p.add_argument('--cxx',default='clang++');p.add_argument('--port',type=int,default=8977)
a=p.parse_args()
try:
 if a.out:build(a)
 else:serve(a)
except (OSError,ValueError,subprocess.CalledProcessError) as error:p.exit(1,str(error)+'\n')
