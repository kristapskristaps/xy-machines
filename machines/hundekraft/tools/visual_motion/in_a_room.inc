    const i32 w=px(35)+k[0]*65,h=w*(450+k[1]/2)/1024;
    const i32 x0=px(234)-w/2,x1=x0+w,y0=px(40)-h/2,y1=y0+h;
    scene.box(x0,y0,x1,y1,BLUE);
    for(i32 i=1;i<5;++i){scene.line(x0+w*i/5,y0,x0+w*i/5,y1,level>0?WHITE:GREY);scene.line(x0,y0+h*i/5,x1,y0+h*i/5,GREY);}
    scene.line(px(297),y0,px(297),y1,WHITE);
    for(i32 i=0;i<80;++i){const i32 x=px(16)+i*px(288)/80;
        const i32 speech=sn(i*957)*sn(i*291)/1024;
        const i32 room=sn(i*(420+k[1]))/2;
        const i32 wave=lerp(speech,room,k[2]),amp=px(1)+mul(wave<0?-wave:wave,level)*10;
        scene.line(x,px(90)-amp,x,px(90)+amp,GREY);}
    scene.line(px(16)+(t%4096)*72,px(75),px(16)+(t%4096)*72,px(105),RED);
    scene.label(0,px(16),px(15),k[7]?"REC":"GEN --",1536);
    const char* shapes[4]={"CUBE","GOLDEN","BOLT","LUCIER"};scene.label(1,px(16),px(43),shapes[k[1]*4/1024]);
    scene.label(2,px(16),px(59),"ROOM MODES");
    const char* names[4]={"size","shape","erosion","dry"};
    for(i32 i=0;i<4;++i){scene.label(3+i,px(16+i*75),px(119),names[i]);scene.line(px(16+i*75),px(133),px(16+i*75)+k[i]*63,px(133),i==0?BLUE:WHITE,px(3));}
