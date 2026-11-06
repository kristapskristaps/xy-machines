"""Render original musical material through Gen Tape's exact C++ callback."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import wave
import numpy as np

MACHINE=Path(__file__).resolve().parents[1]
ROOT=MACHINE.parents[1]
RATE=44100

def material(kind,seconds=12):
    n=round(seconds*RATE);t=np.arange(n)/RATE;audio=np.zeros((n,2));rng=np.random.default_rng(1991)
    beat=60/92
    def add(start,signal,pan=0):
        at=round(start*RATE);length=min(len(signal),n-at)
        if length>0:audio[at:at+length]+=signal[:length,None]*np.array([np.cos((pan+1)*np.pi/4),np.sin((pan+1)*np.pi/4)])
    if kind in ('keys','mix'):
        chords=[(57,60,64,71),(53,57,60,67),(48,55,59,64),(55,59,62,69)]
        for bar,chord in enumerate(chords):
            for step in range(4):
                for voice,note in enumerate(chord):
                    local=np.arange(round(1.8*RATE))/RATE;f=440*2**((note-69)/12)
                    env=(1-np.exp(-local*230))*np.exp(-local*3.5)
                    # Bell partials with independently decaying upper harmonics.
                    signal=.10*env*(np.sin(2*np.pi*f*local)+.28*np.exp(-local*2)*np.sin(2*np.pi*f*2.004*local)+.12*np.exp(-local*5)*np.sin(2*np.pi*f*3.99*local))
                    add(bar*4*beat+step*beat*.75+voice*.013,signal,(voice-1.5)/3)
    if kind in ('pad','mix'):
        chords=[(45,52,57,60,64),(41,48,53,57,60),(48,55,59,64,67),(43,50,55,59,62)]
        for bar,chord in enumerate(chords):
            local=np.arange(round(3.5*RATE))/RATE;env=np.minimum(local/.25,1)*np.exp(-np.maximum(local-1.6,0)*2)
            for voice,note in enumerate(chord):
                f=440*2**((note-69)/12);signal=np.zeros(len(local))
                for harmonic in range(1,10):
                    signal+=np.sin(2*np.pi*f*harmonic*local+voice*.7)/(harmonic**1.6)
                add(bar*4*beat,.028*env*signal,(voice-2)/3)
    if kind in ('drums','mix'):
        for step in range(32):
            start=step*beat/2;local=np.arange(round(.5*RATE))/RATE
            if step%8 in (0,3,4):
                kick=.42*np.sin(2*np.pi*(48*local+2.5*(1-np.exp(-local*45))))*np.exp(-local*16)*(1-np.exp(-local*1500));add(start,kick)
            if step%8 in (2,6):
                noise=rng.normal(0,1,len(local));noise=np.convolve(noise,[.5,-.5],mode='same')
                snare=(noise*.11+np.sin(2*np.pi*185*local)*.08)*np.exp(-local*22)*(1-np.exp(-local*1500));add(start,snare,.1)
            noise=rng.normal(0,1,len(local));noise=np.convolve(noise,[.25,-.5,.25],mode='same')
            add(start,.035*noise*np.exp(-local*(80 if step%4 else 30)),-.3 if step%2 else .3)
        for step,note in enumerate([33,33,40,45,29,29,36,41,36,36,43,47,31,31,38,43]):
            local=np.arange(round(.5*RATE))/RATE;f=440*2**((note-69)/12);env=(1-np.exp(-local*350))*np.exp(-local*8)
            add(step*beat,.15*env*(np.sin(2*np.pi*f*local)+.18*np.sin(4*np.pi*f*local)))
    audio*=np.minimum(t/.02,1)[:,None]*np.minimum(np.maximum(seconds-t,0)/.08,1)[:,None]
    return audio

def wav(path,audio):
    # 24-bit PCM, common gain across examples; no per-file peak normalization.
    value=np.rint(np.clip(audio,-1,1-2**-23)*8388608).astype('<i4')
    packed=value.view(np.uint8).reshape(-1,4)[:,:3].copy().tobytes()
    with wave.open(str(path),'wb') as f:f.setparams((2,3,RATE,len(audio),'NONE','not compressed'));f.writeframes(packed)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True);p.add_argument('--sdk',type=Path);p.add_argument('--cxx');a=p.parse_args()
    config=json.loads((ROOT/'local/dev.json').read_text()) if (ROOT/'local/dev.json').exists() else {}
    sdk=a.sdk or Path(config.get('sdk',ROOT.parent/'xy-machines-sdk'))
    a.cxx=a.cxx or config.get('cxx','c++')
    a.out.mkdir(parents=True,exist_ok=False)
    scenarios=[('01-cassette-keys','keys',[.36,.23,.38,0,.625,.35,0,0]),('02-vhs-pad','pad',[.62,.34,.42,.25,.375,.55,.06,0]),('03-worn-drum-machine','drums',[.42,.48,.7,.7,.375,.4,.035,0]),('04-broken-dictation','mix',[.78,.65,.55,.92,.875,.7,.08,0])]
    playback_gain=2.0
    manifest=dict(sample_rate=RATE,pcm_bits=24,common_playback_gain=playback_gain,source='Original synthesized material; exact fx.cpp callback',hardware_cpu_measured=False,examples=[])
    with tempfile.TemporaryDirectory(prefix='gentape-render-') as folder:
        folder=Path(folder);binary=folder/'render';subprocess.run([a.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(MACHINE/'tools/render.cpp'),'-o',str(binary)],check=True)
        for name,kind,parameters in scenarios:
            dry=material(kind);raw=folder/'input.raw';np.rint(dry*2147483647).astype('<i4').tofile(raw);wetraw=folder/'wet.raw'
            subprocess.run([str(binary),str(raw),str(wetraw),*map(str,parameters)],check=True)
            wet=np.fromfile(wetraw,dtype='<i4').reshape(-1,2)/2147483648.
            # Match the fixed clean-path group delay for immediate A/B listening.
            aligned=np.pad(dry,((512,0),(0,0)))[:len(dry)]
            comparison=np.concatenate([aligned,np.zeros((RATE//2,2)),wet])
            for suffix,audio in [('dry',aligned),('wet',wet),('dry-then-wet',comparison)]:wav(a.out/f'{name}-{suffix}.wav',audio*playback_gain)
            manifest['examples'].append(dict(name=name,parameters=parameters,order=['dry','0.5s gap','wet'],peak_dry=float(np.max(np.abs(dry))),peak_wet=float(np.max(np.abs(wet))),rms_dry=float(np.sqrt(np.mean(dry**2))),rms_wet=float(np.sqrt(np.mean(wet**2)))))
    manifest['source_sha256']={name:hashlib.sha256((MACHINE/name).read_bytes()).hexdigest() for name in ('fx.cpp','core.h','tables.h','definition.json')}
    (a.out/'examples.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest,indent=2))

if __name__=='__main__':main()
