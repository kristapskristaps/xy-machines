    port::Dsp curve;
    const i32 bits=curve.crush_bits(input.slots[0]>>16),drive=k[2]*24/1024;
    const opxy::u32 rate=static_cast<opxy::u32>(curve.crush_exp2((((input.slots[1]>>16)-32767)*14336)>>15))*44100u>>16;
    scene.line(px(106),px(21+drive),px(214),px(21+drive),GREY);
    scene.line(px(106),px(108-drive),px(214),px(108-drive),GREY);
    i32 ox=px(110),oy=px(68),qx=ox,qy=oy;
    const i32 spacing=clamp(rate?32768/static_cast<i32>(rate):20,2,20),quant=bits<10?1024>>bits:1;
    for(i32 x=110;x<=212;x+=2){const i32 s=sn(t*10+x*375),y=px(68)-s*30;
        if(x>110)scene.line(ox,oy,px(x),y,GREY);ox=px(x);oy=y;
        if((x-110)%spacing<2){i32 sample=clamp(s*30,-px(40-drive),px(40-drive));sample=sample/quant*quant;
            const i32 yy=px(68)-sample;scene.line(qx,qy,px(x),qy,BLUE,px(1.4));scene.line(px(x),qy,px(x),yy,BLUE,px(1.4));qx=px(x);qy=yy;}
    }
    const char* labels[4]={"bits","rate","drive","dry"};
    for(unsigned i=0;i<4;++i){const i32 x=px(i<2?14:246),y=px((i&1)?80:4);char value[8]={};
        if(i==0)number(value,bits);else if(i==1)khz(value,rate);else if(i==2)number(value,drive);else header_amount(value,sizeof(value),input.slots[i]);
        scene.label(i*2,x,y,labels[i]);scene.label(i*2+1,x,y+px(20),value,1536);}
