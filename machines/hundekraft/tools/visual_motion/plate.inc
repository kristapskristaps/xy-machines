    const i32 max=30+k[0]*60/1024;
    for(i32 i=0;i<6;++i){const i32 r=(i*15+t/22)%90;if(r<2 || r>max)continue;
        scene.box(px(clamp(132-r,105,215)),px(clamp(68-r*13/16,23,113)),px(clamp(132+r,105,215)),px(clamp(68+r*13/16,23,113)),level?BLUE:GREY);}
    const i32 left=214-(1024-k[1])*64/1024;
    if(left<212){scene.box(px(left),px(28),px(212),px(108),GREY);
        for(i32 x=left-72;x<212;x+=8){i32 a=x,b=x+80,ya=108,yb=28;if(a<left){ya-=left-a;a=left;}if(b>212){yb+=b-212;b=212;}
            if(a<b)scene.line(px(a),px(ya),px(b),px(yb),GREY);}}
    i32 ox=px(84),oy=px(68);
    for(i32 x=86;x<=128;x+=2){const i32 window=clamp(x<88?x-84:x>124?128-x:4,0,4);
        const i32 y=px(68)+sn((x-84)*(120+k[2]))*window;
        scene.line(ox,oy,px(x),y,WHITE);ox=px(x);oy=y;}
    const char* labels[4]={"decay","tone","pre","dry"};
    for(unsigned i=0;i<4;++i){const i32 x=px(i<2?14:246),y=px((i&1)?80:4);char value[8]={};
        header_amount(value,sizeof(value),input.slots[i]);scene.label(i*2,x,y,labels[i]);scene.label(i*2+1,x,y+px(20),value,1536);}
