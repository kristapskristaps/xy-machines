    for(i32 f=0;f<2;++f){i32 ox=0,oy=0;const i32 peak=px(f?202:158)+k[4+f]*(f?105:135);
        for(i32 x=150;x<=310;x+=4){const i32 delta=(px(x)-peak)/128,q=24+k[6]/5,z=delta*q/128;
            const i32 y=px(116)-px(f?38:54)*256/(256+(z*z/256));
            if(x>150)scene.line(ox,oy,px(x),y,GREY);ox=px(x);oy=y;}
    }
    for(i32 i=0;i<4;++i){const i32 n=k[i]*25/1024;if(!n)continue;
        const i32 y=px(9)+(n-1)*4608,length=px(284)/n;
        const Ink col=i==0?BLUE:i==3?ORANGE:WHITE;
        scene.line(px(24),y,px(24)+length,y,col,px(1.4));
        if(level>4){const i32 tick=t/143,pos=tick%n;scene.circle(px(24)+length-length*pos/n,y,px(1.8),col,true);}
    }
