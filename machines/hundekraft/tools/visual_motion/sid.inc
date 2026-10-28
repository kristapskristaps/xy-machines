    const char* names[6]={"triangle","saw","pulse","noise","sync","ring"};
    const char* labels[4]={"wave","color","cutoff","res"};
    for(unsigned i=0;i<4;++i){
        text(frame,i*2,X+px(238)*S/1024,Y+px(18+i*35)*S/1024,unit,labels[i]);
        char value[8]={};header_amount(value,sizeof(value),input.slots[i]);
        text(frame,i*2+1,X+px(238)*S/1024,Y+px(32+i*35)*S/1024,unit,i==0?names[k[0]*6/1024]:value);
    }
