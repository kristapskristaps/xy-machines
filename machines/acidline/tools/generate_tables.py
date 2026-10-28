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
# A dense table around the steepest transfer curves prevents coarse foldback steps.
lines.append('static const opxy::i16 distortion[33][1025] = {')
for i in range(33):
    amount=i/32;gain=1+55*amount*amount;fold=max(0,(amount-.78)/.22);values=[]
    for j in range(1025):
        x=(j-512)/1024
        # Asymmetric clipping, DC removed later, followed by folding at the top of Drive.
        bias=.025*amount
        clipped=math.tanh((x+bias)*gain)-math.tanh(bias*gain)
        folded=2/math.pi*math.asin(math.sin(x*gain*1.3))
        y=(1-amount)*x+amount*.34*((1-.62*fold)*clipped+.62*fold*folded)
        values.append(max(-.49,min(.49,y)))
    lines.append('{'+','.join(str(round(v*Q)) for v in values)+'},')
lines+=['};','}}']
OUT.write_text('\n'.join(lines)+'\n')
print(OUT)
