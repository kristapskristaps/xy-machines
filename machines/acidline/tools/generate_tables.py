"""Generate the portable fixed-point DSP tables. Run from any working directory."""
from pathlib import Path
import math

OUT = Path(__file__).resolve().parents[1] / 'tables.h'
Q = 16384
lines = ['#pragma once', '#include <opxy/types.hpp>', 'namespace acidline { namespace tables {']

def array(name, values):
    lines.append('static const opxy::i16 ' + name + ' = {' + ','.join(str(round(v * Q)) for v in values) + '};')

lines.append('static const opxy::i16 saw[8][513] = {')
for count in [1, 2, 4, 8, 16, 32, 64, 128]:
    values = [sum(-math.sin(2*math.pi*k*i/512)/k for k in range(1,count+1))*.09 for i in range(513)]
    lines.append('{'+','.join(str(round(v*65536)) for v in values)+'},')
lines.append('};')
lines.append('static const opxy::i16 triangle[8][513] = {')
for count in [1, 2, 4, 8, 16, 32, 64, 128]:
    values = [sum(math.cos(2*math.pi*k*i/512)/(k*k) for k in range(1,count+1,2))*8/(math.pi*math.pi)*10000 for i in range(513)]
    lines.append('{'+','.join(str(round(v)) for v in values)+'},')
lines.append('};')
lines.append('static const opxy::i16 sine[513] = {'+','.join(str(round(math.sin(2*math.pi*i/512)*32767)) for i in range(513))+'};')
array('tone[65]', [1-math.exp(-2*math.pi*(150*120**(i/64))/44100) for i in range(65)])
array('pole[65]', [math.tan(math.pi*(45*(12000/45)**(i/64))/88200)/(1+math.tan(math.pi*(45*(12000/45)**(i/64))/88200)) for i in range(65)])
array('feedback[65]', [.985*(i/64)**.7 for i in range(65)])  # Quarter of the loop gain.
lines.append('static const opxy::i16 inverse[65][65] = {')
for r in range(65):
    gain=3.94*(r/64)**.7;row=[]
    for i in range(65):
        g=math.tan(math.pi*(45*(12000/45)**(i/64))/88200);g=g/(1+g)
        row.append(1/(1+gain*g**4))
    lines.append('{'+','.join(str(round(v*Q)) for v in row)+'},')
lines.append('};')
array('decay[65]',[math.exp(-64/(44100*(.055*24**(i/64)))) for i in range(65)])
for count in [31]:
    center=(count-1)//2;values=[]
    for i in range(count):
        n=i-center
        ideal=.5 if n==0 else (0 if n%2==0 else math.sin(math.pi*n/2)/(math.pi*n))
        window=.42-.5*math.cos(2*math.pi*i/(count-1))+.08*math.cos(4*math.pi*i/(count-1))
        values.append(ideal*window)
    total=sum(values)
    normalized=[v/total for v in values]
    assert sum(abs(round(v*Q)) for v in normalized)*49152 < 2147483647
    array(f'halfband{count}[{count}]',normalized)
# Positive half of tanh, input 0..8 in Q15, output 0..32767. Odd symmetry
# removes DC bias; runtime drive gain is a cheap fixed-point multiplication.
lines.append('static const opxy::i16 clip[257] = {'+','.join(str(round(math.tanh(i/32)*32767)) for i in range(257))+'};')
lines+=['}}']
OUT.write_text('\n'.join(lines)+'\n')
print(OUT)
