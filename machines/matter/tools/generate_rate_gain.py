"""Generate compensation for box-input and linear-output modal resampling."""
import argparse
import math
from pathlib import Path


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    lines=['// Generated resampling response compensation, Q24.','#pragma once',
           '#include <opxy/types.hpp>','namespace matter {']
    for rate in (2,4,8):
        values=[]
        for i in range(353):
            internal=i/1024
            gain=(rate*math.sin(math.pi*internal/rate)/math.sin(math.pi*internal))**3 if i else 1
            values.append(round(gain*16777216))
        lines.append(f'static const opxy::u32 rate_gain_{rate}[353]={{')
        lines += ['    '+', '.join(str(x)+'u' for x in values[i:i+8])+',' for i in range(0,len(values),8)]
        lines.append('};')
    lines.append('}')
    args.out.write_text('\n'.join(lines)+'\n')


if __name__=='__main__':
    main()
