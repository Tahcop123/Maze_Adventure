#ifndef _MAP_H
#define _MAP_H

#include <raylib.h>
#include "linkedlist.h"

// Wall cell structure for mouse picking
typedef struct {
    double wx, wy;
    int x0, y0;
} *WallT;

struct location {
    int x;
    int y;
};

// Collectible
typedef struct {
    int x, y;
    int value;
    int active;
    float animTime;
} Coin;

// Trap
typedef struct {
    int x, y;
    int type;
    int active;
    int cycle;
} Trap;

// Portal
typedef struct {
    int x1, y1, x2, y2;
    int active;
    int used;
    float cooldown;
} Portal;

// Box (pushable)
typedef struct {
    int x, y;
    int active;
} Box;

// Pressure plate
typedef struct {
    int x, y;
    int active;
    int pressed;
} Plate;

extern linkedlistADT Wall;
extern WallT curwall;
extern int Row, Col;
extern int map[100][100];
extern int map_change[100][100];
extern int direction[4][2];
extern int X, Y;
extern int start, end;
extern int step;
extern int is_key;
extern int xk, yk;
extern int Hp;
extern int is_start;

// Grid layout
extern double cellSize;
extern double gridOffsetX;
extern double gridOffsetY;

// Collectibles
#define MAX_COINS 60
extern Coin coins[MAX_COINS];
extern int coinCount;

// Traps
#define MAX_TRAPS 15
extern Trap traps[MAX_TRAPS];
extern int trapCount;

// Portals
#define MAX_PORTALS 5
extern Portal portals[MAX_PORTALS];
extern int portalCount;

// Boxes
#define MAX_BOXES 8
extern Box boxes[MAX_BOXES];
extern int boxCount;

// Plates
#define MAX_PLATES 5
extern Plate plates[MAX_PLATES];
extern int plateCount;

// Hidden room
extern int hiddenRoomX, hiddenRoomY;
extern int hiddenRoomFound;

int can_set(int x, int y);
void CreatMap(void);
void CreatWalllist(void);
void Drawmap(int a[][100]);
void DrawLighting(void);
void DrawVignette(void);
void AddWalkBob(void);
void UpdateWalkBob(void);
double distWall(double x, double y, WallT rect);
WallT SelectNearestNode(linkedlistADT Wall, double mx, double my);
void Judgekey(void);
void Randomkey(void);

// Shared cell-entry logic (pickups, traps, portals) for both top-down and FP modes
void OnEnterCell(void);

// New map functions
void InitMapEntities(void);
void SpawnCollectibles(int count);
void SpawnTraps(int count);
void SpawnPortals(int count);
void SpawnBoxes(int count);
void SpawnPlates(int count);
void CreateHiddenRoom(void);
void UpdateTraps(void);
void CheckTraps(void);
void CheckPortals(void);
void CheckCollectibles(void);
int CanMoveTo(int x, int y);
int TryPushBox(int bx, int by, int dx, int dy);
void ExplodeBomb(int cx, int cy);

#endif
