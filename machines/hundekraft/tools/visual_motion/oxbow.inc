    const i32 sx=901+k[0]*246/1024,droop=k[0]<371?-(371-k[0])/4:0;
    scene.axis(G_hornl,sx);scene.axis(G_hornr,sx);
    scene.rotate(G_hornl,droop,141149,36508);scene.rotate(G_hornr,-droop,210761,39200);
    scene.move(G_birdl,(1024-k[3])*9,(1024-k[3])*4);scene.move(G_birdr,-(1024-k[3])*10,(1024-k[3])*4);
    scene.rotate(G_bell,mul(sn(t*7),k[1])*600/1024*level/1024,148612,88996);
    const i32 env=k[2]>512?k[2]-512:512-k[2];
    scene.scale(G_breath,512+env*600/512+mul(sn(t*3),level)/10,224799,78237);
    const unsigned stars[6]={G_s0,G_s1,G_s2,G_s3,G_s4,G_s5};
    for(unsigned i=0;i<6;++i)scene.show(stars[i],input.held_notes<=128 && i<input.held_notes);
