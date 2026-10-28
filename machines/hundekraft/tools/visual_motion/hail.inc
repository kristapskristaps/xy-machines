    const unsigned primary[3]={G_primary0,G_primary1,G_primary2},secondary[3]={G_secondary0,G_secondary1,G_secondary2};
    for(unsigned i=0;i<3;++i){scene.show(primary[i],input.shown[i]<4);scene.show(secondary[i],input.shown[i]>=4);}
    if(moving && !k[7])state.memory[0]=(state.memory[0]+dt*(1+k[3]/160))%65536;
    const i32 phase=reduced?0:state.memory[0];
    const i32 count=4+(k[3]>512?k[3]-512:512-k[3])*16/512;
    for(i32 i=0;i<count && level>4;++i){const i32 u=(phase+i*65536/count)%65536;
        const i32 at=u*(BD_THREAD_N-1)/65536,f=u*(BD_THREAD_N-1)%65536;
        const i32 x=BD_THREAD[at][0]+(BD_THREAD[at+1][0]-BD_THREAD[at][0])*f/65536;
        const i32 y=BD_THREAD[at][1]+(BD_THREAD[at+1][1]-BD_THREAD[at][1])*f/65536;
        const i32 len=px(4)+k[1]*10;
        scene.line(x-len/2,y,x+len/2,y,WHITE,px(2));
        scene.arc(x,y-px(5),px(3),4096,8192,GREY,4);
    }
    const i32 u=k[0]*(BD_THREAD_N-1),at=clamp(u/1024,0,BD_THREAD_N-2),f=u%1024;
    scene.circle(lerp(BD_THREAD[at][0],BD_THREAD[at+1][0],f),lerp(BD_THREAD[at][1],BD_THREAD[at+1][1],f),px(5),BLUE);
    for(i32 i=0;i<count/2;++i)scene.circle(BD_PILE[i][0],BD_PILE[i][1],px(2),i==0?ORANGE:GREY,true);
    const i32 row[4]={BD_RING0_Y,BD_RING1_Y,BD_RING2_Y,BD_RING3_Y};
    for(i32 i=0;i<4;++i){const i32 value=amount(input.parameters[i]);scene.arc(BD_RING_X,row[i],BD_RING_R,3072,3072+value*6144/1024,i==0?BLUE:WHITE,8);
        scene.circle(BD_RING_X,row[i],BD_RING_R*3/4,GREY);const i32 a=3072+value*6144/1024;
        scene.line(BD_RING_X,row[i],BD_RING_X+mul(cs(a),BD_RING_R*5/8),row[i]+mul(sn(a),BD_RING_R*5/8),WHITE);}
