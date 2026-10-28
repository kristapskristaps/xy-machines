    scene.axis(G_pipes0,461+k[0]*563/1024);scene.axis(G_pipes1,461+k[1]*563/1024);scene.axis(G_pipes2,461+k[2]*563/1024);
    const i32 rotor=t*(1+k[3]/90);
    scene.axis(G_horn,cs(rotor));
    scene.move(G_drum,(rotor%2048)*px(7.78)/2048,0);
    scene.rotate(G_organist,mul(sn(t*6),level)/16,px(137.85),px(113.98));
    const i32 sweep=683+k[3]*2503/1024;
    for(i32 i=0;i<2;++i)scene.arc(px(280.75),px(74.63),px(17),rotor+2048+i*4096-sweep/2,rotor+2048+i*4096+sweep/2,ORANGE,10,px(1.4));
