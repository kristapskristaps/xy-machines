# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/sid`. Original notices follow.

sid.c

/* Six OP-1 voices, each a three-oscillator cRSID-derived mini-chip. */

sid_audio.c

/* Dry DSP evidence generator; native firmware ADSR is deliberately absent. */

sid_dsp.c

/* SID primitives adapted from cRSID 1.58, Hermit (Mihaly Horvath).
 * License is still WTF: Do what the frick you want with this code, but
 * it would be nice mentioning me as the original author.
 * Local changes: six independent plugin instances; native OP-1 envelope;
 * block-cached edge coefficients; selected pure waveforms; bounded 8580 LPF.
 * No CPU/player/register bus/digi emulation. */

sid_render_test.c

/* Actual production engine wrapper: current-voice per-sample pitch array. */

sid_screen.c

/* Owner hardware polish: artwork and native readouts move up four screen pixels. */

sid_screen_test.c

#define OP1_HOST
#include "../../op1_synth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int16_t host_knobs[8][8];int host_slot;static int first=1;
typedef struct{int x,y,size;uint8_t color[4];char str[32];} txt;
void *host_text_ctor(void *obj,int32_t x,int32_t y,int32_t size,const uint8_t *c){txt *t=obj;t->x=x;t->y=y;t->size=size;memcpy(t->color,c,4);return obj;}
void host_text_set(void *obj,const char *s){snprintf(((txt*)obj)->str,32,"%s",s);}
void host_text_draw(void *obj,void *r,const int32_t *m){(void)r;(void)m;txt *t=obj;printf("%s[\"text\",%d,%d,%d,0,[%u,%u,%u,%u],\"%s\"]",first?"":",",t->x,t->y,t->size,t->color[0],t->color[1],t->color[2],t->color[3],t->str);first=0;}
void host_screen_draw(void *s,int dt){(void)s;(void)dt;}
void *host_raster(void){return 0;}
#include "sid_screen.c"
int main(int argc,char **argv){for(int i=0;i<4;i++)host_knobs[0][i]=argc>i+1?atoi(argv[i+1]):16384;puts("{\"ops\":[");SCREEN_sid.init(0);SCREEN_sid.update(0,16);puts("]}");return 0;}


sid_test.c

#include "sid_dsp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 sid_state a,b;int16_t q[4]={0,16384,24576,8192};
 for(int mode=0;mode<6;mode++)for(int pitch=0;pitch<5;pitch++)for(int edge=0;edge<4;edge++){
  q[0]=mode*5462;q[1]=(edge&1)?32767:0;q[2]=(edge&2)?32767:0;q[3]=(edge&1)?32767:0;
  sid_reset(&a);sid_reset(&b);uint32_t inc=(uint32_t)1000000u<<(pitch*2);
  sid_configure(&a,inc,q);sid_configure(&b,inc,q);
  for(int i=0;i<4096;i++){int16_t v=sid_sample(&a);assert(v==sid_sample(&b));for(int j=0;j<3;j++)assert(a.phase[j]<=0xfffffff);}
 }
 sid_reset(&a);sid_reset(&b);q[0]=8192;q[2]=24576;q[3]=8192;sid_configure(&a,42852281,q);
 int nonzero=0;for(int i=0;i<1000;i++)nonzero|=sid_sample(&a)!=0;assert(nonzero);assert(!memcmp(&b,&(sid_state){.noise={0x7fffff,0x7fffff,0x7fffff}},sizeof b));
 for(int mode=0;mode<6;mode++)for(int knob=1;knob<4;knob++){
  int16_t x[4]={mode*5462,12000,20000,10000},y[4];memcpy(y,x,sizeof x);y[knob]=30000;
  sid_reset(&a);sid_reset(&b);sid_configure(&a,42852281,x);sid_configure(&b,42852281,y);
  unsigned changes=0;for(int i=0;i<4096;i++)changes+=sid_sample(&a)!=sid_sample(&b);assert(changes>100);
 }
 for(int mode=0;mode<6;mode++)for(int edge=0;edge<4;edge++){
  int16_t q[4]={mode*5462,(edge&1)?32767:0,(edge&2)?32767:0,(edge&1)?32767:0};sid_reset(&a);sid_reset(&b);
  for(int block=0;block<32;block++){
   uint32_t inc=1000000u+(unsigned)block*10000000u;sid_configure(&a,inc,q);sid_configure(&b,inc,q);
   int16_t actual[32];sid_render_samples(&a,actual,32);
   for(int i=0;i<32;i++)assert(actual[i]==sid_sample(&b));assert(!memcmp(&a,&b,sizeof a));
  }
 }
 puts("SID six-mode endpoint/pitch determinism, bounds and independent-state checks PASS");return 0;
}


Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
