    scene.move(G_water,0,px(400));
    const i32 phase=t*(1+k[0]/100),swing=182+k[1]*501/1024;
    scene.rotate(G_socks,mul(sn(phase),swing),px(100),px(76));
    const unsigned bubbles[9]={G_bub0,G_bub1,G_bub2,G_bub3,G_bub4,G_bub5,G_bub6,G_bub7,G_bub8};
    for(i32 i=0;i<9;++i){scene.show(bubbles[i],i<2+k[2]*7/1024);scene.move(bubbles[i],0,-mul(sn(t*13+i*1500)+1024,level));}
    scene.move(G_sock,-px(45)+k[3]*60,-mul(sn(t*35)<0?-sn(t*35):sn(t*35),level)*2);
    for(i32 i=0;i<2;++i)scene.arc(px(100),px(76),px(69),phase+i*4096,phase+i*4096+569+k[0]*1707/1024,BLUE,12,px(2));
    i32 ox=0,oy=0;bool previous=false;
    for(i32 i=0;i<=48;++i){const i32 x=px(49)+i*px(102)/48,y=px(106)-sn(i*8192*23/480+phase)*(2+k[1]*7/1024);
        const i32 dx=(x-px(100))/64,dy=(y-px(76))/64;const bool inside=dx*dx+dy*dy<784*784;
        if(i && inside && previous)scene.line(ox,oy,x,y,WHITE,px(2));ox=x;oy=y;previous=inside;}
