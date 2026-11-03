"""Generate a unity-DC 147-phase Blackman SRC, padded to the existing history."""
import argparse
import math
from pathlib import Path


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--taps',type=int,choices=(8,12,24),required=True)
    parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    begin=(24-args.taps)//2
    rows=[]
    for phase in range(147):
        coefficients=[]
        for i in range(args.taps):
            x=i-(args.taps-1)/2+phase/147
            sinc=math.sin(2*math.pi*.44*x)/(math.pi*x) if x else .88
            window=.42-.5*math.cos(2*math.pi*i/(args.taps-1))+.08*math.cos(4*math.pi*i/(args.taps-1))
            coefficients.append(sinc*window)
        total=sum(coefficients)
        row=[round(x*32768/total) for x in coefficients]
        row[args.taps//2-1]+=32768-sum(row)
        if sum(row)!=32768 or any(x < -32768 or x >32767 for x in row):
            raise ValueError('Invalid Q15 phase')
        rows.append([0]*begin+row+[0]*begin)
    lines=[f'// 48 kHz to 44.1 kHz: 147 phases, {args.taps}-tap Blackman-windowed sinc.',
           '// Padded to the existing 24-sample history; exact unity DC gain in every phase.',
           '#pragma once','#include <opxy/types.hpp>','namespace weave {',
           'static const opxy::i16 resample_coefficients[147][24] = {']
    lines += ['    {'+', '.join(map(str,row))+'},' for row in rows]
    lines += ['};','inline opxy::i32 resample_sum(const opxy::i16* history,opxy::u32 phase) {',
              '    const opxy::i16* c=resample_coefficients[phase];','    opxy::i32 sum=0;']
    for tap in range(begin+1,begin+args.taps-1):
        lines += [f'    sum+=static_cast<opxy::i32>(history[-{tap}])*c[{tap}];']
    lines += ['    return sum;','}','}']
    args.out.write_text('\n'.join(lines)+'\n')


if __name__=='__main__':
    main()
