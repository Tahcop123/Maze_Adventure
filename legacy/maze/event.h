#ifndef _event_h

#define _event_h



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
#include "interface.h"
#include "inc.h"
#include "solution.h"
//变量 
extern  is_mousedown ;//鼠标是否按下 
extern int is_edit;//是否编辑 
extern double mousex,mousey;//鼠标坐标


//函数 
void MouseEventProcess(int x, int y, int button, int event);
void MovePlayer(int dx, int dy);
void KeyboardEventProcess(int key,int event);



#endif
