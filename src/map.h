#ifndef _MAP_H
#define _MAP_H

#include <raylib.h>

struct location {
    int x;
    int y;
};

// ========== Map cell codes ==========
// Grid values shared by generation, rendering, pathfinding and save files.
// Numeric values are part of the map file format - do not renumber.
enum {
    CELL_ROAD      = 0,   // walkable floor
    CELL_PATH      = 1,   // BFS solution overlay (never stored in map files)
    CELL_KEY       = 2,   // golden key
    CELL_WALL      = 3,   // stone wall (blocks movement)
    CELL_START     = 4,   // player spawn
    CELL_EXIT      = 5,   // goal flag (needs the golden key)
    CELL_KEY_RED   = 6,   // colored keys
    CELL_KEY_BLUE  = 7,
    CELL_KEY_GREEN = 8,
    CELL_DOOR_RED   = 10, // colored doors (opened by matching key)
    CELL_DOOR_BLUE  = 11,
    CELL_DOOR_GREEN = 12,
    CELL_COIN       = 20, // legacy map-file collectibles (FP renderer only)
    CELL_GEM        = 21,
    CELL_BORDER     = -1  // out-of-bounds border ring
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

extern int Row, Col;
extern int map[100][100];
extern int map_change[100][100];
extern int direction[4][2];
extern int X, Y;
extern int step;
extern int is_key;
extern int xk, yk;   // golden key cell (cache: kept fresh by CacheMapPoints)
extern int Hp;

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

// Cached feature positions, refreshed whenever the maze layer is re-baked
typedef struct { int x, y, kind; } BonusCell;
extern int endCellX, endCellY;     // exit cell (-1 when absent)
extern int startCellX, startCellY;
extern BonusCell bonusCells[32];   // legacy map-file coin/gem cells (FP view)
extern int bonusCellCount;

int can_set(int x, int y);
void CreatMap(void);

// Static-maze baking: the floor/wall layer is rendered once into a texture
// and only re-baked when the map changes. Call MarkMazeDirty() after any
// direct map_change[][] write.
void MarkMazeDirty(void);
unsigned int GetMazeVersion(void); // bumps on every MarkMazeDirty (solution cache)
void EnsureMapCaches(void);        // bake now if dirty (also refreshes caches)
void Drawmap(int a[][100]);
void DrawLighting(void);
void DrawVignette(void);
void AddWalkBob(void);
void UpdateWalkBob(void);

// Convert a screen position to a grid cell; returns 1 if inside [1..Row][1..Col]
int ScreenToCell(double mx, double my, int *gx, int *gy);

// Per-frame ambient particles (key sparkles, exit magic) - update side only
void SpawnAmbientParticles(void);
void UpdateCollectibles(float dt);

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
