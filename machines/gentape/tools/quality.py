"""Measure saturation-table error and foldback through the production callback."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import numpy as np

MACHINE=Path(__file__).resolve().parents[1]
ROOT=MACHINE.parents[1]
RATE=44100

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
    config=json.loads((ROOT/'local/dev.json').read_text());sdk=Path(config['sdk'])
    # Exercise the compiled lookup and complete fx.cpp renders.
    with tempfile.TemporaryDirectory(prefix='gentape-quality-') as temp:
        temp=Path(temp);source=temp/'quality.cpp';binary=temp/'quality'
        source.write_text('#include "'+str(MACHINE/'fx.cpp')+'"\n#include <cstdio>\nint main(){int a;while(std::scanf("%d",&a)==1)std::printf("%d\\n",gentape::shape(a));}\n')
        subprocess.run([config['cxx'],'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(source),'-o',str(binary)],check=True)
        rng=np.random.default_rng(2701);values=rng.uniform(-3.99,3.99,12000);integers=np.rint(values*(1<<27)).astype(np.int64)
        actual=np.array(subprocess.check_output([str(binary)],input=''.join(f'{x}\n' for x in integers),text=True).split(),dtype=float)/(1<<27)
        expected=np.tanh(integers/(1<<27));error=actual-expected
        report={'table_samples':len(actual),'max_curve_error':float(np.max(np.abs(error))),'rms_curve_error':float(np.sqrt(np.mean(error**2))),'hardware_cpu_measured':False}
        if report['max_curve_error']>.00001:raise ValueError('Saturation lookup error exceeds design bound')
        # Coherent sine: fundamental and harmonics occupy exact FFT bins. Energy
        # outside legal harmonic bins measures foldback, without window leakage.
        render=temp/'render';subprocess.run([config['cxx'],'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(MACHINE/'tools/render.cpp'),'-o',str(render)],check=True)
        n=65536;report['alias_tests']=[]
        for bin_number in (3001,9001,15001):
            freq=bin_number*RATE/n;t=np.arange(3*n);x=.72*np.sin(2*np.pi*bin_number*t/n)
            stereo=np.repeat(x[:,None],2,axis=1);inp=temp/'input.raw';out=temp/'out.raw';np.rint(stereo*2147483647).astype('<i4').tofile(inp)
            subprocess.run([str(render),str(inp),str(out),'0','0','1','0','0','0','0','0'],check=True)
            y=np.fromfile(out,dtype='<i4').reshape(-1,2)[-n:,0]/2147483648
            naive=np.tanh(x[-n:]*3.5)/3
            allowed=np.zeros(n//2+1,dtype=bool);allowed[0]=True
            for harmonic in range(1,n//(2*bin_number)+1):allowed[harmonic*bin_number]=True
            def alias(signal):
                energy=np.abs(np.fft.rfft(signal))**2
                return float(10*np.log10(max(np.sum(energy[~allowed]),1e-30)/energy[bin_number]))
            measured=alias(y)
            if measured>-75:raise ValueError(f'Unexpected foldback at {freq}: {measured} dBc')
            report['alias_tests'].append({'frequency_hz':freq,'shaped_saturation_alias_dbc':measured,'naive_tanh_alias_dbc':alias(naive)})
    a.out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
