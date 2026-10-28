    const unsigned snow[3]={G_snow0,G_snow1,G_snow2};
    if(moving && !k[4])state.memory[0]=(state.memory[0]+dt*(2+level/160))%8192;
    const i32 spin=reduced?0:state.memory[0];
    for(i32 i=0;i<3;++i){scene.show(snow[i],i<1+(k[0]*2+512)/1024);scene.rotate(snow[i],(i&1?-1:1)*spin*(3-i)/2,px(160),px(68.91));}
    scene.scale(G_shimmer,563+k[1]*614/1024+mul(sn(t*42),level)/20,px(186.27),px(105.03));
    scene.scale(G_halo,870+k[2]*358/1024,px(160),px(68.91));
    scene.rotate(G_key,-k[3]*1365/1024+(k[4]?t*2:0),px(204),px(121.19));
    scene.move(G_smoke,0,level>12?-(t*7)%px(3):0);
