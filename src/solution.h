#ifndef _SOLUTION_H
#define _SOLUTION_H

#include "map.h"

extern int is_showsolution;
extern int is_showtip;
extern int shortstep;
extern int xx, yy;

int can_go(int x, int y);
int BFSPath(int sx, int sy, int tx, int ty, int *next_x, int *next_y);
void OptimalSolution(void);

#endif
