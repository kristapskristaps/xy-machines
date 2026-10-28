    scene.move(G_active,0,px(400));
    scene.move(G_dots,(t*3)%px(4),0);
    scene.rotate(G_gear0,t*(2+k[1]/128)+k[1]*2,px(91.59),px(113.46));
    scene.rotate(G_gear1,-2*(t*(2+k[1]/128)+k[1]*2),px(139.07),px(116.26));
    scene.move(G_press,0,(1024-k[2])*10+mul(sn(t*7)+1024,level)*3/4);
    scene.axis(G_coil,1024-k[3]*461/1024);
    port::Dsp wave;
    for(i32 i=0;i<24;++i){
        const opxy::u32 a=static_cast<opxy::u32>(i)*178956970u,b=static_cast<opxy::u32>(i+1)*178956970u;
        const i32 ya=wave.wavetable_shape(input.slots[0]>>16,a)*px(3)/30000;
        const i32 yb=wave.wavetable_shape(input.slots[0]>>16,b)*px(3)/30000;
        scene.line(px(33.16)+i*px(25.23)/24,px(33.35)-ya,px(33.16)+(i+1)*px(25.23)/24,px(33.35)-yb,BLUE,px(1.8));
    }
