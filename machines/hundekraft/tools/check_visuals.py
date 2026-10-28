#!/usr/bin/env python3
"""Run every machine's display invariants through the host compiler."""
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
SDK=ROOT.parents[2] / 'open-xy'
OUT=ROOT/'local/visual-tests'
OUT.mkdir(exist_ok=True)
def check(test):
    name=test.parents[1].name;binary=OUT/name
    command=['/opt/homebrew/opt/llvm/bin/clang++','-std=c++11','-O1','-g','-fsanitize=address,undefined',
             '-fno-sanitize=signed-integer-overflow,shift','-fno-sanitize-recover=all','-I'+str(SDK/'sdk/include'),str(test),'-o',str(binary)]
    result=subprocess.run(command,capture_output=True,text=True)
    if result.returncode:return name,False,result.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True)
    return name,result.returncode==0,result.stdout+result.stderr
failed=[]
with ThreadPoolExecutor(max_workers=4) as pool:
    for f in as_completed([pool.submit(check,p) for p in ROOT.glob('hk*/tests/display.cpp')]):
        name,ok,log=f.result();print(log.strip(),flush=True)
        if not ok:failed.append(name)
if failed:raise SystemExit('Failed: '+', '.join(failed))
