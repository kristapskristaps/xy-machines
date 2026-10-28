    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],amount(input.parameters[i])*6144/1024-3072,px(cs_dial_x[i]),px(CS_DIAL_Y));
    const i32 x=px(20)+k[0]*280,y=px(58);
    scene.line(px(20),y,px(300),y,k[7]?BLUE:GREY);
    scene.line(px(20),y-px(7),px(20),y+px(7),k[7]?GREY:RED,px(1.8));
    scene.line(x,y-px(16),x,y-px(10),BLUE,px(1.4));scene.line(x-px(6),y-px(16),x,y-px(16),BLUE,px(1.4));
    if(moving && !k[7])state.memory[0]=(state.memory[0]+dt*(1+k[2]/90))%65536;
    for(i32 i=0;i<4+k[2]*12/1024 && level>4;++i){const i32 p=(state.memory[0]+i*4096)%65536;
        const i32 xx=px(20)+p*280/64,h=px(2)+mul(sn(p/8)<0?-sn(p/8):sn(p/8),level)*14;
        scene.line(xx,y-h,xx,y+h,WHITE,px(1.4));}
