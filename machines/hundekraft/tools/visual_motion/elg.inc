    scene.scale(G_antlers,819+k[0]*205/1024,px(160),px(61.73));
    scene.scale(G_breath,614+k[1]*461/1024+level/10,px(160),px(110.55));
    scene.scale(G_hum,512+k[2]+level/4,px(160),px(94.28));
    scene.axis(G_aurora,410+k[3]*922/1024+mul(sn(t*(1+k[3]/160)),level)/10);
    scene.axis(G_eyes,moving && t%4608<100?154:1024);
