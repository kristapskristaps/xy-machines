    const i32 position=k[1]*12;
    scene.move(G_gate_person,position,0);
    const i32 scale=1626-k[0]*1234/1024;
    scene.scale(G_face,scale,px(156),px(94));scene.move(G_face,0,-px(12));
    const i32 gate=level*14;
    if(k[1]){scene.line(px(104)+position-gate,px(108),px(104)+position-gate,px(137),WHITE,px(2));
        scene.line(px(104)+position+gate,px(108),px(104)+position+gate,px(137),WHITE,px(2));}
    char value[8]={};header_amount(value,sizeof(value),input.slots[0]);
    scene.label(0,px(14),px(11),"distance");scene.label(1,px(14),px(32),value,1536);
    scene.label(2,px(14),px(126),"gate");scene.label(3,px(260),px(126),"color");scene.label(4,px(284),px(11),"dry");
    scene.line(px(302),px(36),px(302),px(82),GREY,px(5));
    scene.line(px(302),px(82)-k[3]*46,px(302),px(82),RED,px(5));
