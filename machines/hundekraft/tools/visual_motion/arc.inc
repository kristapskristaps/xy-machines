    // Display model of the original x/v portrait; the SDK exposes no DSP trace.
    for(i32 lamp=0;lamp<(k[5]>0?2:1);++lamp){
        i32 ox=0,oy=0;
        for(i32 i=0;i<=48;++i){
            const i32 a=i*8192/48+t*(lamp?3:2),s=sn(a),c=cs(a);
            const i32 nonlinear=s*(1024+k[0])/(1024+(s<0?-s:s)*k[0]/1024);
            const i32 x=px(AG_CX)+nonlinear*(lamp?56:90),y=px(AG_CY)-c*(lamp?26:39);
            if(i)scene.line(ox,oy,x,y,lamp?GREY:BLUE);ox=x;oy=y;
        }
    }
    scene.line(px(AG_L_X0),px(AG_CY-5),px(AG_L_TIP),px(AG_CY-5),GREY);scene.line(px(AG_L_X0),px(AG_CY+5),px(AG_L_TIP),px(AG_CY+5),GREY);
    scene.line(px(AG_R_TIP),px(AG_CY-5),px(AG_R_X0),px(AG_CY-5),GREY);scene.line(px(AG_R_TIP),px(AG_CY+5),px(AG_R_X0),px(AG_CY+5),GREY);
    if(level>8){i32 ox=px(AG_L_TIP+1),oy=px(AG_CY);
        for(i32 i=1;i<=6;++i){const i32 x=px(AG_L_TIP+1)+i*px(AG_R_TIP-AG_L_TIP-2)/6;
            const i32 y=px(AG_CY)-mul(sn(i*4096/6),level)*9+mul(sn(t*143+i*1919),k[2])*3;
            scene.line(ox,oy,x,y,WHITE,px(2));ox=x;oy=y;}
        if(!reduced && state.age<250)for(i32 i=0;i<6*k[3]/1024;++i){const i32 a=4608+i*491;
            scene.line(px(160)+cs(a)*14,px(61)+sn(a)*14,px(160)+cs(a)*(18+level/32),px(61)+sn(a)*(18+level/32),ORANGE);}
    }
    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],amount(input.parameters[i])*6144/1024-3072,px(ag_dial_x[i]),px(AG_DIAL_Y));
