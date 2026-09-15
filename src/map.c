#include "inc.h"
#include "map.h"
#include "particle.h"
#include <rlgl.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

linkedlistADT Wall = NULL;
WallT curwall = NULL;

int Row = 20, Col = 30;
int map[100][100];
int map_change[100][100];
int direction[4][2] = {{0,-1},{0,1},{1,0},{-1,0}};
int is_key = 0;
int X = 2, Y = 2;
int xk, yk;
int Hp = 0;
int is_start = 0;
int step = 0;
int start = 0, end = 0;

double cellSize = 32.0;
double gridOffsetX = 0.0;
double gridOffsetY = 60.0;

// Collectibles
Coin coins[MAX_COINS];
int coinCount = 0;

// Traps
Trap traps[MAX_TRAPS];
int trapCount = 0;

// Portals
Portal portals[MAX_PORTALS];
int portalCount = 0;

// Boxes
Box boxes[MAX_BOXES];
int boxCount = 0;

// Plates
Plate plates[MAX_PLATES];
int plateCount = 0;

// Hidden room
int hiddenRoomX = 0, hiddenRoomY = 0;
int hiddenRoomFound = 0;

// Ice map
int iceMap[100][100];

static float walkBob = 0.0f;

int can_set(int x, int y)
{
    int i, count = 0;
    for (i = 0; i < 4; i++) {
        int x1 = x + direction[i][0];
        int y1 = y + direction[i][1];
        if (map[x1][y1] == 0 || map[x1][y1] == -1) count++;
    }
    return (count <= 1) ? 1 : 0;
}

void CreatMap(void)
{
    int i, j;
    struct location road[10000];

    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            map[i][j] = 3;

    for (i = 0; i <= Row + 1; i++) {
        map[i][0] = -1;
        map[i][Col + 1] = -1;
    }
    for (j = 0; j <= Col + 1; j++) {
        map[0][j] = -1;
        map[Row + 1][j] = -1;
    }

    int head = 0, tail = 0;
    road[head].x = 2;
    road[head].y = 2;

    // RNG is seeded once per level in StartLevel() (reproducible via mapSeed)

    while (head <= tail) {
        int r = rand() % (tail - head + 1) + head;
        int x = road[r].x;
        int y = road[r].y;

        if (can_set(x, y)) {
            map[x][y] = 0;
            for (i = 0; i < 4; i++) {
                int x_next = x + direction[i][0];
                int y_next = y + direction[i][1];
                if (map[x_next][y_next] == 3) {
                    tail++;
                    road[tail].x = x_next;
                    road[tail].y = y_next;
                }
            }
        }

        struct location t = road[head];
        road[head] = road[r];
        road[r] = t;
        head++;
    }

    map[2][2] = 4;
    int found = 0;
    for (int col = Col - 1; col >= 1 && !found; col--) {
        for (i = Row; i >= 1; i--) {
            if (map[i][col] == 0) {
                map[i][col] = 5;
                found = 1;
                break;
            }
        }
    }

    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            map_change[i][j] = map[i][j];
}

void CreatWalllist(void)
{
    int i, j;
    if (Wall != NULL) {
        FreeLinkedList(Wall);
        Wall = NULL;
    }
    Wall = NewLinkedList();
    for (i = 1; i <= Row; i++) {
        for (j = 1; j <= Col; j++) {
            WallT rptr = (WallT)malloc(sizeof(*rptr));
            rptr->wx = gridOffsetX + (j - 0.5) * cellSize;
            rptr->wy = gridOffsetY + (i - 0.5) * cellSize;
            rptr->x0 = i;
            rptr->y0 = j;
            InsertNode(Wall, NULL, rptr);
        }
    }
}

static int noise2d(int x, int y)
{
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) % 1000;
}

static void DrawWall3D(int gx, int gy)
{
    double px = gridOffsetX + (gy - 1) * cellSize;
    double py = gridOffsetY + (gx - 1) * cellSize;
    float cs = (float)cellSize;
    int n = noise2d(gx, gy) % 30;

    // Drop shadow (bottom-right)
    DrawRectangle((int)(px + 3), (int)(py + cs - 1), (int)cs, 4, (Color){0, 0, 0, 100});
    DrawRectangle((int)(px + cs - 1), (int)(py + 3), 4, (int)cs, (Color){0, 0, 0, 80});

    // Main wall face - stone gray with slight variation
    Color base = (Color){(unsigned char)(110 + n/3), (unsigned char)(108 + n/4), (unsigned char)(105 + n/5), 255};
    DrawRectangle((int)px, (int)py, (int)cs, (int)cs, base);

    // Top-left highlight (light source from top-left)
    DrawRectangle((int)px, (int)py, (int)cs, 2, (Color){160, 158, 155, 255});
    DrawRectangle((int)px, (int)py, 2, (int)cs, (Color){145, 143, 140, 255});

    // Bottom-right shadow
    DrawRectangle((int)px, (int)(py + cs - 2), (int)cs, 2, (Color){60, 58, 55, 255});
    DrawRectangle((int)(px + cs - 2), (int)py, 2, (int)cs, (Color){75, 73, 70, 255});

    // Brick pattern - horizontal mortar lines
    int brickH = (int)(cs / 3);
    for (int by = 1; by < 3; by++) {
        DrawLine((int)px, (int)(py + by * brickH), (int)(px + cs), (int)(py + by * brickH), (Color){70, 68, 65, 200});
    }
    // Vertical mortar lines (staggered)
    int offset = (gx % 2) * (cs / 4);
    DrawLine((int)(px + cs/2 + offset), (int)py, (int)(px + cs/2 + offset), (int)(py + brickH), (Color){70, 68, 65, 200});
    DrawLine((int)(px + cs/4 - offset/2), (int)(py + brickH), (int)(px + cs/4 - offset/2), (int)(py + 2*brickH), (Color){70, 68, 65, 200});
    DrawLine((int)(px + 3*cs/4 - offset/2), (int)(py + brickH), (int)(px + 3*cs/4 - offset/2), (int)(py + 2*brickH), (Color){70, 68, 65, 200});

    // Random stone speckles
    if (n % 7 == 0) DrawPixel((int)(px + (n*7)%(int)cs), (int)(py + (n*13)%(int)cs), (Color){90, 88, 85, 255});
    if (n % 11 == 0) DrawPixel((int)(px + (n*17)%(int)cs), (int)(py + (n*19)%(int)cs), (Color){130, 128, 125, 255});
}

static void DrawFloor(int gx, int gy)
{
    double px = gridOffsetX + (gy - 1) * cellSize;
    double py = gridOffsetY + (gx - 1) * cellSize;
    float cs = (float)cellSize;
    int n = noise2d(gx + 100, gy + 100) % 40;

    // Dark stone floor
    Color base = (Color){(unsigned char)(55 + n/4), (unsigned char)(52 + n/5), (unsigned char)(48 + n/6), 255};
    DrawRectangle((int)px, (int)py, (int)cs, (int)cs, base);

    // Floor tile grid lines
    DrawRectangleLines((int)px, (int)py, (int)cs, (int)cs, (Color){40, 38, 35, 100});

    // Subtle cracks / speckles
    if (n % 13 == 0) {
        DrawLine((int)(px + n%(int)cs), (int)(py + (n*3)%(int)cs),
                 (int)(px + (n+10)%(int)cs), (int)(py + (n*3+8)%(int)cs), (Color){42, 40, 37, 150});
    }
}

static void DrawStartPoint(double px, double py, float cs)
{
    float time = GetTime();
    float pulse = 0.85f + 0.15f * sinf(time * 3.0f);

    // Glow
    DrawCircle((int)(px + cs/2), (int)(py + cs/2), cs * 0.45f * pulse, (Color){255, 60, 60, 60});
    DrawCircle((int)(px + cs/2), (int)(py + cs/2), cs * 0.35f * pulse, (Color){255, 80, 80, 100});
    // Core
    DrawCircle((int)(px + cs/2), (int)(py + cs/2), cs * 0.22f, (Color){220, 40, 40, 255});
    DrawCircle((int)(px + cs/2), (int)(py + cs/2), cs * 0.1f, (Color){255, 200, 200, 255});
}

static void DrawEndPoint(double px, double py, float cs)
{
    float time = GetTime();
    int cx = (int)(px + cs/2);
    int cy = (int)(py + cs/2);

    // Magic circle glow
    float pulse = 0.8f + 0.2f * sinf(time * 2.5f);
    DrawCircle(cx, cy, cs * 0.5f * pulse, (Color){150, 80, 255, 50});
    DrawCircle(cx, cy, cs * 0.38f * pulse, (Color){180, 100, 255, 80});

    // Magic ring
    DrawCircleLines(cx, cy, cs * 0.4f, (Color){200, 130, 255, 200});
    DrawCircleLines(cx, cy, cs * 0.3f, (Color){180, 100, 255, 150});

    // Flag pole
    DrawLine(cx - (int)(cs*0.15), cy - (int)(cs*0.35), cx - (int)(cs*0.15), cy + (int)(cs*0.3), (Color){60, 60, 70, 255});

    // Waving flag
    float wave = sinf(time * 4.0f) * cs * 0.06f;
    DrawTriangle((Vector2){cx - (float)(cs*0.15), cy - (float)(cs*0.35)},
                 (Vector2){cx + (float)(cs*0.2) + wave, cy - (float)(cs*0.22)},
                 (Vector2){cx - (float)(cs*0.15), cy - (float)(cs*0.08)}, (Color){220, 50, 80, 255});
    DrawTriangle((Vector2){cx - (float)(cs*0.15), cy - (float)(cs*0.35)},
                 (Vector2){cx + (float)(cs*0.2) + wave, cy - (float)(cs*0.22)},
                 (Vector2){cx - (float)(cs*0.15) + 2, cy - (float)(cs*0.2)}, (Color){255, 100, 120, 255});
}

static void DrawKeyItem(double px, double py, float cs)
{
    float time = GetTime();
    float bob = sinf(time * 2.5f) * cs * 0.08f;
    int cx = (int)(px + cs/2);
    int cy = (int)(py + cs/2 + bob);

    // Golden glow
    DrawCircle(cx, cy, cs * 0.4f, (Color){255, 215, 0, 50});
    DrawCircle(cx, cy, cs * 0.3f, (Color){255, 220, 50, 90});

    // Key body - rotating effect via scale
    float rot = sinf(time * 2.0f) * 0.3f;
    int ringX = cx - (int)(cs * 0.12f * cosf(rot));
    DrawCircle(ringX, cy, (float)(cs*0.16), GOLD);
    DrawCircle(ringX, cy, (float)(cs*0.07), (Color){55, 52, 48, 255});

    // Key shaft
    DrawLine(ringX + (int)(cs*0.12), cy, cx + (int)(cs*0.22), cy, GOLD);
    DrawLine(ringX + (int)(cs*0.12), cy-1, cx + (int)(cs*0.22), cy-1, (Color){255, 240, 150, 255});

    // Key teeth
    DrawLine(cx + (int)(cs*0.1), cy, cx + (int)(cs*0.1), cy + (int)(cs*0.1), GOLD);
    DrawLine(cx + (int)(cs*0.2), cy, cx + (int)(cs*0.2), cy + (int)(cs*0.12), GOLD);

    // Sparkle particles
    if (rand() % 3 == 0) SpawnKeyGlow((float)cx, (float)cy);
}

void Drawmap(int a[][100])
{
    int i, j;
    float cs = (float)cellSize;

    // Draw floors first
    for (i = 1; i <= Row; i++) {
        for (j = 1; j <= Col; j++) {
            if (a[i][j] != 3 && a[i][j] != -1) {
                DrawFloor(i, j);
            }
        }
    }

    // Draw solution path glow
    for (i = 1; i <= Row; i++) {
        for (j = 1; j <= Col; j++) {
            if (a[i][j] == 1) {
                double px = gridOffsetX + (j - 1) * cellSize;
                double py = gridOffsetY + (i - 1) * cellSize;
                DrawRectangle((int)px, (int)py, (int)cs, (int)cs, (Color){0, 200, 220, 80});
            }
        }
    }

    // Draw walls (on top of floors for overlap shadow)
    for (i = 1; i <= Row; i++) {
        for (j = 1; j <= Col; j++) {
            if (a[i][j] == 3 || a[i][j] == -1) {
                DrawWall3D(i, j);
            }
        }
    }

    // Draw items on top
    for (i = 1; i <= Row; i++) {
        for (j = 1; j <= Col; j++) {
            double px = gridOffsetX + (j - 1) * cellSize;
            double py = gridOffsetY + (i - 1) * cellSize;
            if (a[i][j] == 4) DrawStartPoint(px, py, cs);
            if (a[i][j] == 5) {
                DrawEndPoint(px, py, cs);
                if (rand() % 4 == 0) SpawnMagic((float)(px + cs/2), (float)(py + cs/2));
            }
            if (a[i][j] == 2) DrawKeyItem(px, py, cs);
        }
    }

    // Draw player (animated)
    double ppx = gridOffsetX + (Y - 0.5) * cellSize;
    double ppy = gridOffsetY + (X - 0.5) * cellSize;
    DrawPlayerAnimated(ppx, ppy, cs);

    // Hint text at bottom
    DrawText("Tip: Grab the key first, then reach the flag!",
             (int)(gridOffsetX + cellSize * Col / 2 - 250),
             (int)(gridOffsetY + cellSize * Row + 10), 16, (Color){180, 180, 200, 200});
}

void DrawLighting(void)
{
    float time = GetTime();
    double px = gridOffsetX + (Y - 0.5) * cellSize;
    double py = gridOffsetY + (X - 0.5) * cellSize;

    // === Fog of war: black out unexplored cells ===
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (!IsExplored(i, j)) {
                double fx = gridOffsetX + (j - 1) * cellSize;
                double fy = gridOffsetY + (i - 1) * cellSize;
                DrawRectangle((int)fx, (int)fy, (int)cellSize + 1, (int)cellSize + 1, BLACK);
            }
        }
    }

    // Semi-transparent darkness over explored area
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){5, 5, 15, 130});

    // === Additive glow for light sources ===
    BeginBlendMode(BLEND_ADDITIVE);

    // Player torch - warm flickering glow (smaller)
    float flicker = 0.92f + 0.08f * sinf(time * 12.0f) + 0.04f * sinf(time * 27.0f);
    float pr = (float)(cellSize * 2.5) * flicker;
    for (float r = pr; r > 0; r -= 4.0f) {
        float alpha = 25.0f * (1.0f - r / pr);
        if (alpha > 1.0f)
            DrawCircle((int)px, (int)py, r, (Color){255, 180, 80, (unsigned char)alpha});
    }
    DrawCircle((int)px, (int)py, (float)(cellSize * 0.8), (Color){255, 200, 100, 8});

    // Key light (golden, smaller)
    if (!is_key) {
        for (int i = 1; i <= Row; i++) {
            for (int j = 1; j <= Col; j++) {
                if (map_change[i][j] == 2) {
                    double kx = gridOffsetX + (j - 0.5) * cellSize;
                    double ky = gridOffsetY + (i - 0.5) * cellSize;
                    float kr = (float)(cellSize * 1.5);
                    for (float r = kr; r > 0; r -= 4.0f) {
                        float alpha = 40.0f * (1.0f - r / kr);
                        if (alpha > 1.0f)
                            DrawCircle((int)kx, (int)ky, r, (Color){255, 215, 0, (unsigned char)alpha});
                    }
                }
            }
        }
    }

    // End point light (purple magic, smaller)
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (map_change[i][j] == 5) {
                double ex = gridOffsetX + (j - 0.5) * cellSize;
                double ey = gridOffsetY + (i - 0.5) * cellSize;
                float er = (float)(cellSize * 1.8);
                for (float r = er; r > 0; r -= 4.0f) {
                    float alpha = 45.0f * (1.0f - r / er);
                    if (alpha > 1.0f)
                        DrawCircle((int)ex, (int)ey, r, (Color){150, 80, 255, (unsigned char)alpha});
                }
            }
        }
    }

    // Torch item boost
    if (torchBoostTime > 0) {
        DrawCircle((int)px, (int)py, (float)(cellSize * 2.0), (Color){255, 200, 100, 30});
    }

    EndBlendMode();
}

void DrawVignette(void)
{
    // Corner darkening using gradient rectangles
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    int v = 120;

    // Top
    for (int i = 0; i < v; i++) {
        float a = 100.0f * (1.0f - (float)i / v);
        DrawLine(0, i, w, i, (Color){0, 0, 0, (unsigned char)a});
    }
    // Bottom
    for (int i = 0; i < v; i++) {
        float a = 100.0f * (1.0f - (float)i / v);
        DrawLine(0, h - 1 - i, w, h - 1 - i, (Color){0, 0, 0, (unsigned char)a});
    }
    // Left
    for (int i = 0; i < v; i++) {
        float a = 80.0f * (1.0f - (float)i / v);
        DrawLine(i, 0, i, h, (Color){0, 0, 0, (unsigned char)a});
    }
    // Right
    for (int i = 0; i < v; i++) {
        float a = 80.0f * (1.0f - (float)i / v);
        DrawLine(w - 1 - i, 0, w - 1 - i, h, (Color){0, 0, 0, (unsigned char)a});
    }
}

void AddWalkBob(void)
{
    walkBob = -3.0f;
}

void UpdateWalkBob(void)
{
    if (walkBob < 0.0f) {
        walkBob += 0.5f;
        if (walkBob > 0.0f) walkBob = 0.0f;
    }
}

double distWall(double x, double y, WallT rect)
{
    return fabs(x - rect->wx) + fabs(y - rect->wy);
}

WallT SelectNearestNode(linkedlistADT Wall, double mx, double my)
{
    linkedlistADT nearestnode = NULL, ptr;
    double mindistance, dist;
    ptr = NextNode(Wall, Wall);
    if (ptr == NULL) return NULL;
    nearestnode = ptr;
    mindistance = distWall(mx, my, (WallT)NodeObj(Wall, ptr));
    while (NextNode(Wall, ptr) != NULL) {
        ptr = NextNode(Wall, ptr);
        dist = distWall(mx, my, (WallT)NodeObj(Wall, ptr));
        if (dist < mindistance) {
            nearestnode = ptr;
            mindistance = dist;
        }
    }
    return (WallT)NodeObj(Wall, nearestnode);
}

// Shared cell-entry logic used by both top-down grid movement (event.c)
// and first-person cell crossing (fpview.c), so no interaction is missed.
void OnEnterCell(void)
{
    Judgekey();
    CheckCollectibles();
    CheckItemPickup();
    CheckTraps();
    CheckPortals();

    // Multi-key pickup (map values 6,7,8 = red,blue,green keys)
    if (map_change[X][Y] >= 6 && map_change[X][Y] <= 8) {
        int keyIdx = map_change[X][Y] - 6;
        keysCollected[keyIdx] = 1;
        map_change[X][Y] = 0;
        PlayKeySound();
        AddScore(50);
        if (keysCollected[0] && keysCollected[1] && keysCollected[2])
            UnlockAchievement(6); // Key Master
    }
}

void Judgekey(void)
{
    if (map_change[X][Y] == 2) {
        map_change[X][Y] = 0;
        is_key = 1;
    }
}

void Randomkey(void)
{
    int roads[1000][2];
    int roadCount = 0;
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (map_change[i][j] == 0 && !(i == 2 && j == 2)) {
                roads[roadCount][0] = i;
                roads[roadCount][1] = j;
                roadCount++;
            }
        }
    }
    if (roadCount > 0) {
        int idx = rand() % roadCount;
        xk = roads[idx][0];
        yk = roads[idx][1];
        map_change[xk][yk] = 2;
    }
    is_key = 0;
    step = 0;
    is_start = 0;
}


// ========== New Map Entity Functions ==========

void InitMapEntities(void)
{
    coinCount = 0;
    trapCount = 0;
    portalCount = 0;
    boxCount = 0;
    plateCount = 0;
    hiddenRoomFound = 0;
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++)
            iceMap[i][j] = 0;
}

void SpawnCollectibles(int count)
{
    int spawned = 0, attempts = 0;
    while (spawned < count && attempts < 300 && coinCount < MAX_COINS) {
        attempts++;
        int cx = rand() % Row + 1;
        int cy = rand() % Col + 1;
        if (map_change[cx][cy] == 0 && !(cx == 2 && cy == 2)) {
            coins[coinCount].x = cx;
            coins[coinCount].y = cy;
            coins[coinCount].value = (rand() % 5 == 0) ? 50 : 10;
            coins[coinCount].active = 1;
            coins[coinCount].animTime = (float)(rand() % 100) / 10.0f;
            coinCount++;
            spawned++;
        }
    }
}

void SpawnTraps(int count)
{
    int spawned = 0, attempts = 0;
    while (spawned < count && attempts < 200 && trapCount < MAX_TRAPS) {
        attempts++;
        int tx = rand() % (Row - 4) + 3;
        int ty = rand() % (Col - 4) + 3;
        if (map_change[tx][ty] == 0 && (abs(tx - 2) + abs(ty - 2)) > 4) {
            traps[trapCount].x = tx;
            traps[trapCount].y = ty;
            traps[trapCount].type = 0;
            traps[trapCount].active = 1;
            traps[trapCount].cycle = rand() % 60;
            trapCount++;
            spawned++;
        }
    }
}

void SpawnPortals(int count)
{
    for (int p = 0; p < count && portalCount < MAX_PORTALS; p++) {
        int x1, y1, x2, y2, ok = 0, attempts = 0;
        while (!ok && attempts < 200) {
            attempts++;
            x1 = rand() % (Row - 4) + 2;
            y1 = rand() % (Col / 2) + 2;
            x2 = rand() % (Row - 4) + 2;
            y2 = rand() % (Col / 2) + Col / 2;
            if (map_change[x1][y1] == 0 && map_change[x2][y2] == 0 &&
                (abs(x1-x2)+abs(y1-y2)) > 8) ok = 1;
        }
        if (ok) {
            portals[portalCount].x1 = x1;
            portals[portalCount].y1 = y1;
            portals[portalCount].x2 = x2;
            portals[portalCount].y2 = y2;
            portals[portalCount].active = 1;
            portals[portalCount].used = 0;
            portals[portalCount].cooldown = 0;
            portalCount++;
        }
    }
}

void SpawnBoxes(int count)
{
    int spawned = 0, attempts = 0;
    while (spawned < count && attempts < 200 && boxCount < MAX_BOXES) {
        attempts++;
        int bx = rand() % (Row - 4) + 3;
        int by = rand() % (Col - 4) + 3;
        // Never block the golden key or its only entrance: skip its 4 neighbors
        int nearKey = 0;
        for (int d = 0; d < 4; d++) {
            if (map_change[bx + direction[d][0]][by + direction[d][1]] == 2) nearKey = 1;
        }
        if (map_change[bx][by] == 0 && !nearKey && (abs(bx-2)+abs(by-2)) > 5) {
            boxes[boxCount].x = bx;
            boxes[boxCount].y = by;
            boxes[boxCount].active = 1;
            boxCount++;
            spawned++;
        }
    }
}

void SpawnPlates(int count)
{
    int spawned = 0, attempts = 0;
    while (spawned < count && attempts < 200 && plateCount < MAX_PLATES) {
        attempts++;
        int px = rand() % (Row - 4) + 3;
        int py = rand() % (Col - 4) + 3;
        if (map_change[px][py] == 0) {
            plates[plateCount].x = px;
            plates[plateCount].y = py;
            plates[plateCount].active = 1;
            plates[plateCount].pressed = 0;
            plateCount++;
            spawned++;
        }
    }
}

void CreateHiddenRoom(void)
{
    // Create a 3x3 hidden room at a random edge, behind a breakable wall
    int side = rand() % 4;
    int hx, hy;
    if (side == 0) { hx = 3; hy = rand() % (Col - 6) + 3; }
    else if (side == 1) { hx = Row - 4; hy = rand() % (Col - 6) + 3; }
    else if (side == 2) { hx = rand() % (Row - 6) + 3; hy = 3; }
    else { hx = rand() % (Row - 6) + 3; hy = Col - 4; }

    hiddenRoomX = hx;
    hiddenRoomY = hy;

    // Carve room
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if (hx+i >= 1 && hx+i <= Row && hy+j >= 1 && hy+j <= Col)
                map_change[hx+i][hy+j] = 0;

    // Put gems and items in room
    coins[coinCount].x = hx; coins[coinCount].y = hy;
    coins[coinCount].value = 50; coins[coinCount].active = 1;
    coins[coinCount].animTime = 0; coinCount++;

    SpawnItem(hx, hy + (hy < Col/2 ? 1 : -1), ITEM_BOMB);
}

void UpdateTraps(void)
{
    for (int i = 0; i < trapCount; i++) {
        if (traps[i].active) {
            traps[i].cycle++;
            if (traps[i].cycle > 120) traps[i].cycle = 0;
        }
    }
    for (int i = 0; i < portalCount; i++) {
        if (portals[i].cooldown > 0) portals[i].cooldown -= GetFrameTime();
    }
}

void CheckTraps(void)
{
    for (int i = 0; i < trapCount; i++) {
        if (!traps[i].active) continue;
        if (traps[i].x == X && traps[i].y == Y) {
            // Spike active in first half of cycle
            if (traps[i].cycle < 60) {
                if (shieldActive) {
                    shieldActive = 0;
                } else {
                    Hp -= 15;
                    tookDamageThisLevel = 1;
                    TriggerShake(5.0f, 0.2f);
                    TriggerFlash((Color){255, 0, 0, 100}, 0.15f);
                    PlayHurtSound();
                }
                if (Hp <= 0) { gameOverReason = 0; gameState = STATE_GAMEOVER; }
            }
        }
    }
}

void CheckPortals(void)
{
    for (int i = 0; i < portalCount; i++) {
        if (!portals[i].active || portals[i].cooldown > 0) continue;
        if (X == portals[i].x1 && Y == portals[i].y1) {
            X = portals[i].x2;
            Y = portals[i].y2;
            portals[i].cooldown = 1.0f;
            portals[i].used = 1;
            PlayDoorSound();
            for (int p = 0; p < 10; p++) SpawnMagic((float)(gridOffsetX + Y*cellSize), (float)(gridOffsetY + X*cellSize));
        } else if (X == portals[i].x2 && Y == portals[i].y2) {
            X = portals[i].x1;
            Y = portals[i].y1;
            portals[i].cooldown = 1.0f;
            portals[i].used = 1;
            PlayDoorSound();
        }
    }

    // Portal Master achievement: every portal pair in this level used
    if (portalCount > 0) {
        int allUsed = 1;
        for (int i = 0; i < portalCount; i++)
            if (portals[i].active && !portals[i].used) allUsed = 0;
        if (allUsed) UnlockAchievement(14);
    }
}

void CheckCollectibles(void)
{
    for (int i = 0; i < coinCount; i++) {
        if (coins[i].active && coins[i].x == X && coins[i].y == Y) {
            coins[i].active = 0;
            AddScore(coins[i].value);
            if (coins[i].value >= 50) gemsCollected++;
            else coinsCollected++;
            PlayCoinSound();
        }
    }
}

int CanMoveTo(int x, int y)
{
    if (x < 1 || x > Row || y < 1 || y > Col) return 0;
    if (map_change[x][y] == 3) return 0;
    // Check boxes
    for (int i = 0; i < boxCount; i++) {
        if (boxes[i].active && boxes[i].x == x && boxes[i].y == y) return 2; // box
    }
    // Check doors (10=red,11=blue,12=green)
    if (map_change[x][y] >= 10 && map_change[x][y] <= 12) {
        int doorIdx = map_change[x][y] - 10;
        if (keysCollected[doorIdx]) return 1; // can open
        return 0;
    }
    return 1;
}

int TryPushBox(int bx, int by, int dx, int dy)
{
    int nx = bx + dx, ny = by + dy;
    if (nx < 1 || nx > Row || ny < 1 || ny > Col) return 0;
    if (map_change[nx][ny] == 3) return 0;
    // Check another box
    for (int i = 0; i < boxCount; i++) {
        if (boxes[i].active && boxes[i].x == nx && boxes[i].y == ny) return 0;
    }
    // Move box
    for (int i = 0; i < boxCount; i++) {
        if (boxes[i].active && boxes[i].x == bx && boxes[i].y == by) {
            boxes[i].x = nx;
            boxes[i].y = ny;
            // Check pressure plate
            for (int p = 0; p < plateCount; p++) {
                if (plates[p].active && plates[p].x == nx && plates[p].y == ny) {
                    plates[p].pressed = 1;
                    // Open a random door
                    for (int d = 0; d < 3; d++) {
                        if (doorExists[d]) {
                            map_change[doorPositions[d][0]][doorPositions[d][1]] = 0;
                            doorExists[d] = 0;
                            PlayDoorSound();
                            break;
                        }
                    }
                }
            }
            return 1;
        }
    }
    return 0;
}

void ExplodeBomb(int cx, int cy)
{
    // Destroy walls in 3x3 area
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int x = cx + dx, y = cy + dy;
            if (x >= 1 && x <= Row && y >= 1 && y <= Col) {
                if (map_change[x][y] == 3) {
                    map_change[x][y] = 0;
                    // Check hidden room
                    if (x == hiddenRoomX && y == hiddenRoomY && !hiddenRoomFound) {
                        hiddenRoomFound = 1;
                        AddScore(100);
                    }
                }
            }
        }
    }
    // Damage enemies in range
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active && abs(enemies[i].x - cx) <= 1 && abs(enemies[i].y - cy) <= 1) {
            enemies[i].active = 0;
            AddScore(30);
        }
    }
    // Particles
    for (int i = 0; i < 30; i++) {
        float angle = (float)rand() / RAND_MAX * PI * 2;
        float speed = (float)rand() / RAND_MAX * 4 + 1;
        SpawnParticle((float)(gridOffsetX + cy*cellSize), (float)(gridOffsetY + cx*cellSize),
                      cosf(angle)*speed, sinf(angle)*speed, 0.5f, 3, (Color){255, 150, 50, 200});
    }
    CreatWalllist();
    OptimalSolution();
}
