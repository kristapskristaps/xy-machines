    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],amount(input.parameters[i])*6144/1024-3072,px(70+i*60),px(133));
    // A labelled display study; no borrowed sample/peak pointer is available to draw::Input.
    for(i32 i=0;i<88;++i){const i32 wave=sn(i*711)*sn(i*113)/1024,amp=px(1)+(wave<0?-wave:wave)*28;
        scene.line(px(20)+i*px(280)/88,px(60)-amp,px(20)+i*px(280)/88,amp+px(60),GREY);}
    const i32 x=px(20)+k[0]*280;
    scene.line(x,px(20),x,px(100),BLUE);
    scene.line(clamp(x-k[2]*100,px(20),px(300)),px(104),clamp(x+k[2]*100,px(20),px(300)),px(104),WHITE);
    scene.line(x,px(16),clamp(x+px(4)+k[1]*100,px(20),px(300)),px(16),WHITE);
    if(level>0)for(i32 i=0;i<4;++i){const i32 phase=(t*(3+i)+i*2048)%8192;
        const i32 xx=clamp(x+sn(phase)*k[2]/10,px(20),px(300)),h=px(2)+(sn(phase/2)>0?sn(phase/2):0)*28;
        scene.line(xx,px(60)-h,xx,px(60)+h,ORANGE,px(1.5));}
