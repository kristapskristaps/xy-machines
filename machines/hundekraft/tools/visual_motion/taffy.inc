    scene.move(G_ribbon,0,px(400));
    scene.rotate(G_roll0,t*(4+k[1]/80),px(62.67),px(72.21));
    scene.rotate(G_roll1,-t*(4+k[1]/80),px(257.33),px(72.21));
    scene.axis(G_spring,358+k[3]*870/1024+mul(sn(t*18),level)/16);
    scene.show(G_spark,!reduced && level>280 && k[3]>256 && state.age<180);
    scene.rotate(G_crank,k[2]*6827/1024-910,px(257.33),px(74.48));
    i32 oldx=px(72.11),oldy=px(58.2);
    port::Dsp wave;
    for(i32 i=0;i<=112;++i){
        const i32 x=px(77.78)+i*px(158.77)/112;
        const i32 sample=wave.taffy_wave_at(input.slots[0]>>16,input.slots[1]>>16,input.slots[3]>>16,i*65536/112);
        const i32 y=px(80.76)-sample*15/32;
        scene.line(oldx,oldy,x,y,BLUE,px(2));oldx=x;oldy=y;
    }
    scene.line(oldx,oldy,px(248.26),px(58.2),BLUE,px(2));
