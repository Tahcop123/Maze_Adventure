#ifndef _creatmap_h

#define _creatmap_h


#include "graphics.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"
#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <windows.h>
#include <time.h>
#include <math.h> 

#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

#include "imgui.h"
#include "linkedlist.h"

#include "interface.h"
#include "event.h"
#include "inc.h"
#include "solution.h"

//变量 
typedef struct {/*墙链表*/
	double wx, wy; //中心坐标 
    int x0,y0;//数组里的第几个 
    
    bool isSelected; /*选中*/ 
} *WallT;

struct location  /*用于记录坐标的结构类型*/ 
{
    int x;
    int y;
};




extern WallT curwall;

extern linkedlistADT Wall;

extern int Row,Col;

extern int map[100][100];           /*存储用随机生成的原始迷宫*/


extern int map_change[100][100];    /*存储走迷宫、设计迷宫中的改变后的迷宫*/

extern int direction[4][2];  /*人移动的方向数组*/

extern int X,Y;//人的位置 

extern int start,end;
extern int step;

extern int is_key; 
extern int xk,yk;

//函数

int can_set(int x,int y);
void CreatMap(); 
void CreatWalllist();
void DrawWall(void *rect, int n);
void DrawSelectedWall(void *rect, int n);
void DrawPeople(WallT rect);
void Drawmap(int a[][100]);
void Judge();//判断是否到达终点 

void Judgekey();






#endif 
