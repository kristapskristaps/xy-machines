"""Generate a phase-quantized Q15 lookup from the retained DX7 linear sine table."""
import argparse
import re
from pathlib import Path

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bits',type=int,choices=range(10,16),default=14)
    parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    source=(Path(__file__).resolve().parents[1]/'vendor/macro_dx7.h').read_text()
    match=re.search(r'dx7_sine\[[^]]*\]\s*=\s*\{([^}]+)\}',source)
    values=[int(x) for x in re.findall(r'-?\d+',match.group(1))]
    count=1<<args.bits
    result=[]
    for phase in range(count):
        numerator=phase*512
        index,fraction=divmod(numerator,count)
        value=values[index]+(values[index+1]-values[index])*fraction/count
        result.append(max(-32768,min(32767,int((value+16384)//32768))))
    lines=['// Generated from the retained MIT DX7 sine table; phase-rounded Q15.','#pragma once',
           f'static const unsigned dx_sine_phase_bits={args.bits};',f'static const int16_t dx_sine_q15[{count}]={{']
    lines+=['    '+', '.join(str(x) for x in result[i:i+16])+',' for i in range(0,count,16)]
    lines+=['};']
    args.out.write_text('\n'.join(lines)+'\n')

if __name__=='__main__':
    main()
