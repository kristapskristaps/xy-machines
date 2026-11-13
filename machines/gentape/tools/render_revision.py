"""Render the installed prototype against this revision, plus a native-routing Dry sweep."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import numpy as np
from render_examples import MACHINE, ROOT, RATE, material, wav


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--before',default='4fe5772');p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();config=json.loads((ROOT/'local/dev.json').read_text());sdk=Path(config['sdk']);cxx=config['cxx']
    a.out.mkdir(parents=True,exist_ok=False)
    commit=subprocess.check_output(['git','rev-parse',a.before+'^{commit}'],cwd=ROOT,text=True).strip()
    gain=1.5;report=dict(before_commit=commit,after_source_sha256={n:hashlib.sha256((MACHINE/n).read_bytes()).hexdigest() for n in ('core.h','fx.cpp','tables.h','definition.json')},playback_gain=gain,hardware_cpu_measured_for_revision=False,comparisons=[],dry_sweep=[])
    with tempfile.TemporaryDirectory(prefix='gentape-revision-') as folder:
        temp=Path(folder);reference=temp/'reference';(reference/'tools').mkdir(parents=True)
        for name in ('core.h','fx.cpp','tables.h','tools/render.cpp'):
            (reference/name).write_text(subprocess.check_output(['git','show',commit+':machines/gentape/'+name],cwd=ROOT,text=True))
        binaries={}
        for label,source in [('before',reference/'tools/render.cpp'),('after',MACHINE/'tools/render.cpp')]:
            binary=temp/label;subprocess.run([cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(source),'-o',str(binary)],check=True);binaries[label]=binary
        def process(audio,parameters,label):
            inp=temp/'input.raw';out=temp/'output.raw';np.rint(audio*2147483647).astype('<i4').tofile(inp)
            subprocess.run([str(binaries[label]),str(inp),str(out),*map(str,parameters)],check=True)
            return np.fromfile(out,dtype='<i4').reshape(-1,2)/2147483648
        def save(name,audio):
            if np.max(np.abs(audio*gain))>=1:raise ValueError('Audition clips: '+name)
            wav(a.out/name,audio*gain)
        for name,kind,values in [
            ('01-musical-keys','keys',[.55,.35,.65,.25,.375,.5,.02,0]),
            ('02-damaged-recorder','mix',[.9,.75,.85,.85,.875,.7,.08,0]),
            ('03-warbling-pad','pad',[.8,.6,.6,.5,.625,.65,.03,0]),
            ('04-full-controls','mix',[1,1,1,1,.375,1,1,0])]:
            dry=material(kind);before=process(dry,values,'before');after=process(dry,values,'after')
            save(name+'-old-then-new.wav',np.concatenate([before,np.zeros((RATE//2,2)),after]))
            save(name+'-new.wav',after)
            report['comparisons'].append(dict(name=name,controls=values,before_rms=float(np.sqrt(np.mean(before**2))),after_rms=float(np.sqrt(np.mean(after**2))),sequence=['installed prototype','0.5s gap','local revision']))
        dry=material('mix',6);segments=[]
        for value in (0,.25,.5,.75,1):
            controls=[.65,.4,.7,.4,.625,.4,.04,value]
            wet=process(dry,controls,'after')
            # At full source send, the SDK source mixer contributes Dry*source.
            complete=dry*value+wet
            save(f'dry-{round(value*100):03}.wav',complete);segments += [complete,np.zeros((RATE//4,2))]
            report['dry_sweep'].append(dict(dry=value,complete_rms=float(np.sqrt(np.mean(complete**2))),wet_return_rms=float(np.sqrt(np.mean(wet**2)))))
        save('05-dry-0-25-50-75-100.wav',np.concatenate(segments[:-1]))
    (a.out/'comparison.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
