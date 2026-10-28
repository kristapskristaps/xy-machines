    const unsigned primary[3]={G_l0,G_l1,G_l2},secondary[3]={G_p0,G_p1,G_p2};
    for(unsigned i=0;i<3;++i){scene.show(primary[i],input.shown[i]<4);scene.show(secondary[i],input.shown[i]>=4);}
    const i32 row0[4]={25768,59846,93923,128000},row1[4]={36761,70838,104915,138993};
    for(i32 i=0;i<4;++i){const i32 slot=input.shown[i]<8?input.shown[i]:i,n=(k[slot]*12+512)/1024;
        for(i32 j=0;j<n;++j)scene.line(248503+j*6991,row0[i],248503+j*6991,row1[i],i==0?BLUE:WHITE,px(1.4));}
    if(level>8){
        for(i32 i=0;i<2;++i)scene.arc(152867,95022,px(4+i*3)+level*3+sn(t*90),6656,9216,WHITE,6);
        scene.arc(141874,106015,px(6)+(t%340)*60,3072,6144,GREY,8);
        for(i32 c=0;c<2;++c)for(i32 j=0;j<k[3]*6/1024;++j){const i32 a=j*1365+t*49;
            scene.line((c?142974:71521)+cs(a)*11,(c?31265:48853)+sn(a)*11,
                (c?142974:71521)+cs(a)*14,(c?31265:48853)+sn(a)*14,RED);}
    }
