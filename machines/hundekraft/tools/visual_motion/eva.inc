    const unsigned masks[4]={G_m0,G_m1,G_m2,G_m3};
    for(unsigned i=0;i<4;++i){const i32 n=(k[i]*16+512)/1024;scene.move(masks[i],n>=16?px(200):n*5796,0);}
    scene.move(G_cross,k[1]*20152/1024,0);
    const i32 breath=mul(k[2],128+(sn(t*2)>0?sn(t*2)*896/1024:0));
    scene.axis(G_fog,clamp(breath,20,1024));
    scene.show(G_strip,k[3]>0 && !(moving && k[3]>128 && (t/64)%13==0));
