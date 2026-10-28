    scene.rotate(G_clapper,mul(sn(t*20),level)*250/1024,px(160),px(46));
    const i32 n=1+k[1]*3/1024,h=180+level*620/1024;
    for(i32 i=0;i<n;++i){scene.arc(px(160),px(78),px(56+i*9),-h,h,WHITE,6);scene.arc(px(160),px(78),px(56+i*9),4096-h,4096+h,WHITE,6);}
    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],k[i]*6144/1024-3072,px(i%2?284:36),px(i<2?32:128));
