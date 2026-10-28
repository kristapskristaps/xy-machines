    const i32 running=level;
    const i32 shake=mul(sn(t*117),mul(k[3],running))*3/2;
    scene.move(G_saw,shake/2,shake);
    scene.rotate(G_sprk,-t*36,px(SS_SPRK_X),px(SS_SPRK_Y));
    for(i32 run=0;run<3;++run){
        if(run==0 && k[0]<8)continue;if(run==2 && k[2]<8)continue;
        const i32 period=run==2?24:12,top=run==2?SS_BAR_BOT:SS_BAR_TOP,dir=run==2?1:-1;
        const i32 speed=run==0?5+k[0]/160:run==2?-2:5;
        const i32 offset=(t*speed/32)%(period*1024);
        for(i32 n=-1;n<16;++n){
            i32 x=px(SS_CHAIN_X0+n*period)-offset,xx=x+px(period);
            const i32 lo=px(SS_CHAIN_X0),hi=px(SS_CHAIN_X1);if(xx<lo||x>hi)continue;
            const i32 rise=px(period)*(1024-k[1])/1024,fall=px(period)-px(period/2)*k[1]/1024;
            const i32 h=run==2?k[2]*5:px(5),y=px(top)+shake;
            if(x>=lo && x+rise<=hi)scene.line(x,y,x+rise,y+dir*h,run==0?GREY:WHITE);
            if(x+rise>=lo && x+fall<=hi)scene.line(x+rise,y+dir*h,x+fall,y+dir*h,WHITE);
            if(x+fall>=lo && x+fall<=hi)scene.line(x+fall,y+dir*h,x+fall,y,run==0?GREY:WHITE);
        }
    }
    if(k[2]>16 && level>4)for(i32 i=0;i<3;++i){const i32 p=(t*4+i*1024)%3072;
        scene.circle(px(SS_EXH_X)-p*22/3,px(SS_EXH_Y)-p*34/3,px(1.5)+p*(1+k[2]/210)/3,GREY);}
    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],k[i]*6144/1024-3072,px(ss_dial_x[i]),px(SS_DIAL_Y));
