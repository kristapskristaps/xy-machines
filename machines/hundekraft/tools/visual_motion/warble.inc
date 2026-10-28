    const i32 depth=mul(k[1],k[1]),rot=t*14+mul(sn(t*(1+k[0]/90)),depth)*4;
    for(i32 c=0;c<2;++c){const i32 x=px(c?188:132),y=px(49);
        scene.circle(x,y,px(c?12:17),GREY);scene.circle(x,y,px(7),BLUE);
        for(i32 n=0;n<6;++n){const i32 a=rot+n*8192/6;scene.line(x+cs(a)*3,y+sn(a)*3,x+cs(a)*6,y+sn(a)*6,WHITE);}}
    i32 ox=px(118),oy=px(124);
    for(i32 x=120;x<=202;x+=2){const i32 window=clamp(x<126?x-118:x>194?202-x:8,0,8);
        const i32 y=px(124)+mul(sn(t*(1+k[0]/90)+(x-118)*8192/(64-k[0]*52/1024)),depth)*window;
        scene.line(ox,oy,px(x),y,WHITE);ox=px(x);oy=y;}
    const char* labels[4]={"rate","depth","tone","dry"};
    for(unsigned i=0;i<4;++i){const i32 x=px(i<2?14:246),y=px((i&1)?80:4);char value[8]={};
        header_amount(value,sizeof(value),input.slots[i]);scene.label(i*2,x,y,labels[i]);scene.label(i*2+1,x,y+px(20),value,1536);}
