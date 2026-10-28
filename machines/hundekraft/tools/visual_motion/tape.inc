    const unsigned spokes[9]={G_s0,G_s1,G_s2,G_s3,G_s4,G_s5,G_s6,G_s7,G_s8};
    const i32 speed=2+(1024-k[2])/120,rot=t*speed;
    for(unsigned i=0;i<9;++i)scene.rotate(spokes[i],rot*tp_roll[i][2],tp_roll[i][0],tp_roll[i][1]);
    scene.rotate(G_pinch,-rot,px(TP_CAPSTAN_X),px(TP_TOP_Y-6));
    const unsigned bars[4]={G_bar0,G_bar1,G_bar2,G_bar3};
    for(unsigned i=0;i<4;++i)scene.rotate(bars[i],k[i]*6144/1024-3072,px(tp_dial[i][0]),px(tp_dial[i][1]));
    const i32 head[3]={TP_ERASE_X,TP_REC_X,TP_PLAY_X};
    for(i32 i=0;i<3;++i)scene.box(px(head[i]-6),px(TP_HEAD_TOP),px(head[i]+6),px(TP_TOP_Y),i==0?BLUE:i==1?RED:WHITE,px(1.2));
    scene.circle(px(TP_LED_X),px(TP_LED_Y),px(3),level>8?ORANGE:GREY,true);
    // Moving white registration marks follow the original tape's polygonal route.
    for(i32 d=(t*speed/16)%44;d<TP_LOOP_LEN;d+=44){i32 part=0,left=d;
        while(part<TP_N-1 && left>=tp_len[part])left-=tp_len[part++];const i32 next=(part+1)%TP_N;
        const i32 x=px(tp_pt[part][0])+(tp_pt[next][0]-tp_pt[part][0])*left*1024/tp_len[part];
        const i32 y=px(tp_pt[part][1])+(tp_pt[next][1]-tp_pt[part][1])*left*1024/tp_len[part];
        scene.line(x,y,x+px(2),y,GREY,px(2));}
    scene.line(px(TP_LED_X),px(25),px(TP_LED_X),px(TP_LANE_Y),GREY);
    scene.line(px(TP_PLAY_X),px(TP_LANE_Y),px(TP_SUM_X),px(TP_LANE_Y),WHITE);
