#ifndef _FOG_H
#define _FOG_H

void InitFog(void);
void UpdateFog(void);
int IsExplored(int x, int y);
int IsVisible(int x, int y);
void DrawMinimap(int x, int y, int w, int h);
void RevealArea(int cx, int cy, int radius);

#endif
