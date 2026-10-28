    const i32 divs[6]={1,2,4,8,16,32},div=divs[k[0]*6/1024],slice=t*div/512;
    scene.rotate(G_reel,(slice%div)*8192/div,SX_REEL_X,SX_REEL_Y);
    const i32 xs[4]={SX_RING0_X,SX_RING2_X,SX_RING1_X,SX_RING3_X},ys[4]={SX_RING0_Y,SX_RING2_Y,SX_RING1_Y,SX_RING3_Y};
    for(i32 i=0;i<4;++i){const i32 value=amount(input.parameters[i]),a=3072+value*6144/1024;
        scene.arc(xs[i],ys[i],SX_RING_R,3072,a,i==0?BLUE:WHITE,8);scene.circle(xs[i],ys[i],SX_RING_R*3/4,GREY);
        scene.line(xs[i],ys[i],xs[i]+mul(cs(a),SX_RING_R*5/8),ys[i]+mul(sn(a),SX_RING_R*5/8),WHITE);}
    const i32 mode=k[1]*4/1024,cw=(SX_PIC_X1-SX_PIC_X0)/16,top=SX_PIC_Y0+px(3),bot=SX_PIC_Y1-px(2);
    for(i32 i=0;i<16;++i){const i32 x=SX_PIC_X0+cw*i+cw/2;const bool repeat=((i+slice)*7919)%1024<k[3];
        if(!repeat)scene.line(x,bot-px(3),x,bot,GREY);
        else if(mode==3)for(i32 y=top;y<bot;y+=px(7))scene.line(x,y,x,y+px(3),WHITE);
        else scene.line(x+(mode==2?px(.8):0),top+(mode==1?(i&3)*px(5):0),x-(mode==2?px(.8):0),bot,i==15?WHITE:GREY);}
    if(k[5])scene.circle(SX_RING3_X,SX_RING3_Y,SX_RING_R/2,BLUE);
