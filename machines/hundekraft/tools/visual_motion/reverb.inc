    const unsigned arms[6]={G_karm1,G_karm2,G_karm3,G_karm4,G_karm5,G_karm6};
    for(i32 i=0;i<6;++i)scene.show(arms[i],i<1+(k[0]*5+512)/1024);
    scene.show(G_kfire,k[1]>102);scene.tint(G_kskull,k[2]>512?WHITE:GREY);
    const unsigned mix[10]={G_kmix0,G_kmix1,G_kmix2,G_kmix3,G_kmix4,G_kmix5,G_kmix6,G_kmix7,G_kmix8,G_kmix9};
    for(i32 i=0;i<10;++i)scene.show(mix[i],i>=10-(k[3]*10+512)/1024);
    scene.rotate(G_ktime,k[0]*4096/1024,KALI_TIME_X,KALI_TIME_Y);
    scene.rotate(G_kdiff,k[2]*4096/1024,KALI_DIFF_X,KALI_DIFF_Y);
    scene.rotate(G_ktone,k[1]*4096/1024,KALI_TONE_X,KALI_TONE_Y);
