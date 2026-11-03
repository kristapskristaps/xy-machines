"""Record numeric sound features from an unchanged Git baseline, retaining old hashes."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

MACHINE=Path(__file__).resolve().parents[1]
REPO=MACHINE.parents[1]
SOURCE=r'''
#include @ENGINE@
#include @FEATURES@
#include <cstdio>
int main() {
    static short samples[32768];
    std::puts("static const ReferenceQuality reference_quality[15][4] = {");
    for(unsigned model=0;model<15;++model) {
        std::puts("{");
        for(unsigned variant=0;variant<4;++variant) {
            weave_dsp::macro_voice voice;weave_dsp::macro_init(&voice);voice.rng=0x9e3779b9u+1u;
            unsigned char p[7]={};p[0]=model*8;
            p[1]=variant==1?0:variant==2?127:64;
            p[2]=variant==1?32:variant==2?127:64;
            p[3]=variant==1?16:variant==2?127:64;
            p[4]=variant==3?127:0;weave_dsp::macro_trig(&voice);
            for(unsigned block=0;block<2048;++block) {
                if(block==1024)weave_dsp::macro_trig(&voice);
                weave_dsp::macro_render(&voice,p,42852281u,samples+block*16,16);
            }
            const auto q=quality_features(samples,32768);
            std::printf("{%.17g,%.17g,{",q.energy,q.mean);
            for(unsigned i=0;i<8;++i)std::printf("%s%.17g",i?",":"",q.correlation[i]);
            std::printf("},{");for(unsigned i=0;i<8;++i)std::printf("%s%.17g",i?",":"",q.envelope[i]);
            std::puts("}},");
        }
        std::puts("},");
    }
    std::puts("};");
}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline',required=True)
    parser.add_argument('--sdk',type=Path,required=True)
    parser.add_argument('--cxx',default='c++')
    parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    baseline=subprocess.check_output(['git','rev-parse',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    with tempfile.TemporaryDirectory(prefix='weave-reference-quality-') as temp:
        scratch=Path(temp);vendor=scratch/'vendor';vendor.mkdir()
        paths=subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/weave/vendor'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/weave/vendor')
            if relative.suffix not in ('.h','.cpp'):continue
            target=vendor/relative;target.parent.mkdir(parents=True,exist_ok=True)
            target.write_bytes(subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO))
        cpp=SOURCE.replace('@ENGINE@',json.dumps(str(vendor/'macro.cpp'))).replace('@FEATURES@',json.dumps(str(MACHINE/'tests/quality_features.h')))
        (scratch/'reference.cpp').write_text(cpp);binary=scratch/'reference'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(args.sdk.resolve()/'sdk/include'),str(scratch/'reference.cpp'),'-o',str(binary)],check=True)
        features=subprocess.check_output([str(binary)],text=True)
    args.out.write_text('// Numeric sound features rendered from unchanged baseline '+baseline+'.\n'
                        '// Same 60 fixtures as reference_vectors.h; those original hashes remain intact.\n'
                        '#pragma once\n#include "quality_features.h"\n'+features)


if __name__=='__main__':
    main()
