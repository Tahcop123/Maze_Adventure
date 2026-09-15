#ifndef _interface_h

#define _interface_h



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

#include "map.h"
#include "event.h"
#include "inc.h"
#include "solution.h"

//变量 

extern int   is_change; // 是否进入特殊模式 
extern int   is_interface;//是否在主界面 
extern int	 is_maze ;//是否进入迷宫

int is_save;//是否保存 

extern int   is_e;  //是否进入地图编辑菜单 
extern int   is_solve; //是否进入求解菜单
extern int   is_help; //是否进入帮助菜单
extern int   is_help1; //是否进入帮助菜单
extern int   is_about; //是否进入关于菜单 
extern int   is_file; //是否进入文件菜单

extern HDC gdc, osdc;
extern int Hp;
extern int start;
extern int is_start;
extern char HpString[20];
extern double HeartSize;
//函数 
void drawInterface();
void Interfacedisplay();
void Exitbutton();

void Exitbuttondisplay();
void DrawHelp();
void DrawAbout();
void DrawPic(char* path, int w, int h); //插入背景图 

void Randomkey();//随机生成钥匙位置 

void DrawHp(int num);


#endif
