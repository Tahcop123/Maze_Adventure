#ifndef _solution_h

#define _solution_h


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
#include "event.h"
#include "inc.h"


//solution
extern int is_showsolution; //whether to show solution
extern int is_showtip;
extern int shortstep;

int can_go(int x,int y);
int BFSPath(int sx, int sy, int tx, int ty, int *next_x, int *next_y);
void OptimalSolution();
void DrawSolution();
void Solutionbutton();
void Draweditbox(int n);




#endif
