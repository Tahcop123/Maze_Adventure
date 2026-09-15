#include "interface.h"
#include <string.h>

int   is_change = 0; // 是否进入特殊模式 
int   is_interface = 1;//是否在主界面 
int	  is_maze = 0;//是否进入迷宫

int   is_e = 0; //是否进入地图编辑菜单 
int   is_solve = 0; //是否进入求解菜单
int   is_help = 0; //是否进入帮助菜单
int   is_help1 = 0; //是否进入帮助菜单
int   is_about = 0; //是否进入关于菜单 
int   is_file = 0; //是否进入文件菜单
int   is_file1 = 0;


int is_save = 0;//是否保存  

int Hp=0;//Mao
int is_start = 0;//Mao 只有点击了start之后才能开始游戏

char HpString[20];
double HeartSize =0.1;


 
//主界面
void drawInterface()
{
	double fH = GetFontHeight();
	double h = fH*2;  // 控件高度
	double x = winwidth/2.5;  
	double y = winheight/2-h; 
	double w = winwidth/5; // 控件宽度
	double menuwidth = w;//菜单宽度 
    double menuheight = h;//菜单高度 
	DrawPic("background.bmp",1920/3*2,1080/3*2);
	if(is_interface){
		if( button(GenUIID(0), x-w/2, y+h, w*2, h*2, "Start Game") ){
			is_interface = 0;
			CreatMap();
			CreatWalllist();
			Randomkey();
			X=Y=2;
			OptimalSolution();
			Hp = shortstep*2;
			Drawmap(map_change);
			Draweditbox(is_edit);
			is_maze = 1;
			
		}
	}
	if(is_interface){
		if(button(GenUIID(0), 0, winheight-menuheight, menuwidth, menuheight, "File(F)")){
			is_file = ! is_file;
			if(is_file){ is_e = 0; is_help = 0; }
		}
		if( is_file ) {
			int x1,x2,x3,x4;
			x1=button(GenUIID(0), 0, winheight-menuheight*2, menuwidth, menuheight, "NewFile");
			x2=button(GenUIID(0), 0, winheight-menuheight*3, menuwidth, menuheight, "OpenFile");
			x3=button(GenUIID(0), 0, winheight-menuheight*4, menuwidth, menuheight, "SaveFile");
			x4=button(GenUIID(0), 0, winheight-menuheight*5, menuwidth, menuheight, "Exit");
			if(x1==1){
				is_interface=0;
				DisplayClear();
			    DrawPic("background.bmp",1920/3*2,1080/3*2);
			    CreatMap();
			    CreatWalllist();
			    Randomkey();
			    X=Y=2;
			    OptimalSolution();
			    Hp = shortstep*2;
			    Drawmap(map_change);
			    is_maze =1;
			    Draweditbox(is_edit);

			}
			if(x2==1){
				is_interface=0;
				DisplayClear();
			    DrawPic("background.bmp",1920/3*2,1080/3*2);
				CreatMap(); 
	        	CreatWalllist();
				Openfile();
				X=Y=2;
				//Randomkey();
				step = 0;//每次步数清零 
	            is_start = 0;//timer starts on first move 
	            is_key=0;
	            shortstep = 0;
	            OptimalSolution();
			    Hp = shortstep*2;
	        	Drawmap(map_change);
	        	is_maze =1;
	        	Draweditbox(is_edit);
			}
			if(x3==1){
				Savefile();
			}
			if(x4==1){
				exit(0);
			}
		}
		
	}
	if(is_interface){
		if(button(GenUIID(0), menuwidth, winheight-menuheight, menuwidth, menuheight, "MapEdit(E)")){
			is_e = ! is_e;
			if(is_e){ is_file = 0; is_help = 0; }
		}
		if( is_e){
			int y;
			y=button(GenUIID(0), menuwidth, winheight-menuheight*2, menuwidth, menuheight, "Random Generate");
			if(y==1){
				is_interface = 0;
				DisplayClear();
			    DrawPic("background.bmp",1920/3*2,1080/3*2);
			    CreatMap();
			    CreatWalllist();
			    X=Y=2; 
			    Randomkey(); 
			    OptimalSolution();
			    Hp = shortstep*2;
			    Drawmap(map_change);
			    is_maze = 1;
			    Draweditbox(is_edit);
			}
		}
		
	}
	if(is_interface){
		if(button(GenUIID(0), menuwidth*2, winheight-menuheight, menuwidth, menuheight, "Help(H)")){
			is_help = ! is_help;
			if(is_help){ is_file = 0; is_e = 0; }
		}
		if( is_help){
			if(button(GenUIID(0), menuwidth*2, winheight-menuheight*2, menuwidth, menuheight, "Help")){
				is_interface = 0;
		    	is_help1 = 1;
			}
			if(button(GenUIID(0), menuwidth*2, winheight-menuheight*3, menuwidth, menuheight, "About")){
				is_interface = 0;
		    	is_about = 1;
			}
		}
	}


}


void Interfacedisplay()
{
	// 清屏
//	DisplayClear();
			
	// 按钮
	drawInterface();
}



//退出按钮 
void Exitbutton(){
	double fH = GetFontHeight();
	double h = fH*2;  // 控件高度
	double x = winwidth * 7 / 8;  
	double y = h; 
	double w = winwidth/8; // 控件宽度
	
	if( button(GenUIID(0), x, y, w, h, "Exit") ){
		is_maze = 0;
		DisplayClear();	
		is_interface = 1;
		if(is_save == 0){
			FreeLinkedList(Wall);
		} 
		is_edit = 0;
		is_help1 = 0;
		is_about = 0;
		is_file1 = 0;
		is_help = 0;
		drawInterface();
	}

}


void Exitbuttondisplay(){
	
	DisplayClear();
	Exitbutton();
	Drawmap(map_change);
}

void DrawHelp()
{
	double cx,cy;
	double textheight = 0.4;
	cx=GetWindowWidth()/4;
	cy=GetWindowHeight()/4*3;
	DrawPic("help.bmp",1920/3*2,1080/3*2);
	SetPenColor("Red");
	SetPointSize(5); 
	MovePen(cx,cy);
	DrawTextString("欢迎来到走迷宫小游戏！");
	SetPenColor("Black");
	MovePen(cx,cy-textheight);
	DrawTextString("游戏规则");
	MovePen(cx,cy-textheight*2);
	DrawTextString("鼠标左键单击Nomal scenarios键即可开始游戏");
	MovePen(cx,cy-textheight*3);
	DrawTextString("当然您也可以通过MapEdit下拉菜单中的Random Generate随机生成一张地图");
	MovePen(cx,cy-textheight*4);
	DrawTextString("进入游戏后,您需要通过键盘的上↑下↓左←右→键控制小人移动,最终走出迷宫");
	MovePen(cx,cy-textheight*5);
	DrawTextString("但走出迷宫的前提是您需要携带钥匙");
	MovePen(cx,cy-textheight*6);
	DrawTextString("在游戏界面您可以通过Solution按钮查看最短路径");
	MovePen(cx,cy-textheight*7);
	DrawTextString("也可以通过Tip按钮查看提示");
	MovePen(cx,cy-textheight*8);
	DrawTextString("可以按x键进行地图编辑功能,鼠标所指出现红圈即可开始编辑(w加墙,q破坏墙)");
	MovePen(cx,cy-textheight*9);
	DrawTextString("但是不支持编辑起点终点和钥匙哦!");
	MovePen(cx,cy-textheight*10);
	DrawTextString("如果对刚才所玩地图感兴趣,可以通过FIle下拉栏的SaveFile将地图选择路径保存");
	MovePen(cx,cy-textheight*11);
	DrawTextString("同样,也可以通过OpenFile打开指定地图哦(只支持txt文件)");
	MovePen(cx,cy-textheight*12);
	SetPenColor("Red");
	DrawTextString("下面是快捷键");
	MovePen(cx,cy-textheight*13);
	DrawTextString("s(S)--保存地图 o(O)--打开地图 e(E)--随机生成地图 x(X)--地图编辑 h(H)--帮助菜单");
	SetPointSize(1);
}
void DrawAbout()
{
	double cx,cy;
	double textheight = 0.25;
	cx=0;
	cy=GetWindowHeight()/3*2;
	DrawPic("help.bmp",1920/3*2,1080/3*2);
	SetPenColor("Black");
	MovePen(cx,cy);
	DrawTextString("************************");
	MovePen(cx,cy-textheight);
	DrawTextString("**************************");
	MovePen(cx,cy-textheight*2);
	DrawTextString("C大太难了");
	MovePen(cx,cy-textheight*3);
	DrawTextString("Welcome to this game!");
	MovePen(cx,cy-textheight*4);
	DrawTextString("Welcome to this game!");
}
void DrawPic(char* path, int w, int h){
    // Bug 8+9 fix: cache bitmap to avoid repeated LoadImage and GDI leak
    static HBITMAP bg_cached = NULL;
    static char cached_path[256] = "";
    if (bg_cached == NULL || strcmp(cached_path, path) != 0) {
        if (bg_cached != NULL) DeleteObject(bg_cached);
        bg_cached = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, w, h, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
        strncpy(cached_path, path, sizeof(cached_path)-1);
        cached_path[sizeof(cached_path)-1] = '\0';
    }
    if (bg_cached != NULL) {
        SelectObject(osdc, bg_cached);
    }
} 
void Randomkey()
{
	xk=rand() % 6 + 10;//5~105
	yk=rand() % 11 + 10;//生成5~20的随机数 
	map_change[xk][yk]=2;
	map_change[xk-1][yk]=0;//留条通路，防止钥匙四面都是墙 
	is_key = 0;
	step = 0;//每次步数清零 
	shortstep = 0;
	start=clock();//重新计时 
} 
void DrawHp(int num) {
	double fH = GetFontHeight();
	double h = fH*2;  // 控件高度
	double x = 0;  
	double y = 2*h; 
	double w = winwidth/8; // 控件宽度
	if(is_maze){
		SetPenColor("Red");
	    MovePen(winwidth/5,winheight*0.9);
	    SetPenSize(3);
	    StartFilledRegion(1.0);
	    DrawArc(HeartSize,0,180);
	    DrawLine(HeartSize*2,-0.2);
	    DrawLine(HeartSize*2,0.2);
	    DrawArc(HeartSize,360,180);
	    EndFilledRegion();
	    SetPointSize(22);
        sprintf(HpString,"%d",Hp);
	    drawLabel(winwidth/5+0.35,winheight*0.9-0.15,HpString);
	    SetPenSize(1);
	    SetPointSize(1);
	}
} 
