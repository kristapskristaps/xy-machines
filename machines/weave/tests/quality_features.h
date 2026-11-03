#pragma once

struct ReferenceQuality {
    double energy, mean, correlation[8], envelope[8];
};
inline ReferenceQuality quality_features(const short* samples,unsigned count) {
    ReferenceQuality result={};
    for(unsigned i=0;i<count;++i) {
        result.energy+=double(samples[i])*samples[i];
        result.mean+=samples[i];
    }
    const double total=result.energy>0?result.energy:1;
    for(unsigned j=0;j<8;++j) {
        const unsigned lag=1u<<j;
        for(unsigned i=lag;i<count;++i)result.correlation[j]+=double(samples[i])*samples[i-lag];
        result.correlation[j]/=total;
        for(unsigned i=j*(count/8);i<(j+1)*(count/8);++i)
            result.envelope[j]+=double(samples[i])*samples[i];
        result.envelope[j]/=total;
    }
    result.energy/=count;
    result.mean/=count;
    return result;
}
