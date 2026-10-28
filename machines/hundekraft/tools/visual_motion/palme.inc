    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],amount(input.parameters[i])*6144/1024-3072,px(pg_dial[i][0]),px(pg_dial[i][1]));
    if(k[4]>20){scene.circle(px(160),px(73),px(22),GREY);scene.circle(px(160),px(73),px(30),GREY);scene.circle(px(160),px(73),px(7),WHITE);}
    for(i32 i=0;i<PG_NS;++i){i32 ox=pg_bot[i][0],oy=pg_bot[i][1];
        for(i32 j=1;j<=7;++j){const i32 window=sn(j*4096/7),displacement=mul(mul(sn(t*(3+i)+j*8192/7),window),level)*4;
            const i32 x=pg_bot[i][0]+(pg_top[i][0]-pg_bot[i][0])*j/7+mul(pg_nrm[i][0],displacement);
            const i32 y=pg_bot[i][1]+(pg_top[i][1]-pg_bot[i][1])*j/7+mul(pg_nrm[i][1],displacement);
            scene.line(ox,oy,x,y,WHITE);ox=x;oy=y;}}
    for(i32 side=0;side<2;++side){const i32 i0=side?11:0,hx=pg_dmp[i0][0],hy=pg_dmp[i0][1],angle=k[7]?(side?700:-700):0;
        i32 ox=hx,oy=hy;
        for(i32 j=1;j<6;++j){const i32 i=side?11-j:j,dx=pg_dmp[i][0]-hx,dy=pg_dmp[i][1]-hy;
            const i32 x=hx+mul(dx,cs(angle))-mul(dy,sn(angle)),y=hy+mul(dx,sn(angle))+mul(dy,cs(angle));
            scene.line(ox,oy,x,y,k[7]?RED:GREY,px(2));ox=x;oy=y;}}
    scene.circle(px(160),px(147),px(2)+level*4,WHITE);
