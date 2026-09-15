#include "inc.h"

double winwidth, winheight;   // 窗口尺寸
double wh,ww;    //墙体的宽和高 

int start=0,end=0;          /*用于给游戏计时*/ 
int step=0;//步数 
void Main(){
	
	SetWindowTitle("maze...");
	
	SetWindowSize(15.0, 10.0);
	
	InitGraphics();
	
	//获取窗口尺寸 
	winwidth = GetWindowWidth();
    winheight = GetWindowHeight();
    ww = winwidth/Col;
    wh = 5.0 * winheight /7.0 / Row;
    MessageBoxA(NULL, "欢迎来到走迷宫小游戏,按H键了解详细信息", "提示", 0);
	
//	Wall = NewLinkedList();//建立强的链表 
    
//	CreatMap();
//	Drawmap(map_change);
	
	
	registerMouseEvent(MouseEventProcess); 
	registerKeyboardEvent(KeyboardEventProcess);
	
}
