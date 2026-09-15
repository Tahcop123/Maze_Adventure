#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <raylib.h>
#include "map.h"
#include "solution.h"

extern int is_edit;
extern int is_file;
extern int is_e;
extern int is_help;

void DrawInterface(void);
void DrawTopBar(void);
void DrawHp(void);
void DrawHelp(void);
void DrawAbout(void);
void DrawCharSelect(void);
void DrawDifficultySelect(void);
void DrawLeaderboard(void);
void DrawAchievements(void);
int Button(int x, int y, int w, int h, const char *label, Color bg, Color textColor);
int MenuButton(int x, int y, int w, int h, const char *label);

#endif
