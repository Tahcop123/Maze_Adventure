#include "inc.h"
#include "particle.h"

int Row = 20, Col = 30;
int map[100][100];
int map_change[100][100];
int direction[4][2] = {{0,-1},{0,1},{1,0},{-1,0}};
int is_key = 0;
int X = 2, Y = 2;
int xk = -1, yk = -1;
int Hp = 0;
int step = 0;

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
static float trapTickFraction = 0;

// Ice map
int iceMap[100][100];

// Cached feature positions (refreshed whenever the maze layer is re-baked)
int endCellX = -1, endCellY = -1;
int startCellX = -1, startCellY = -1;
BonusCell bonusCells[32];
int bonusCellCount = 0;
static int colorKeyPos[3][2] = {{-1,-1},{-1,-1},{-1,-1}};
static int colorDoorPos[3][2] = {{-1,-1},{-1,-1},{-1,-1}};

static float walkBob = 0.0f;

// ========== Baked static layers ==========
// The floor/wall maze layer and the fog-of-war layer change rarely, so they
// are rendered into textures and re-baked only when marked dirty. This turns
// thousands of per-frame primitives into two textured quads.
static RenderTexture2D mazeRT = {0};
static RenderTexture2D fogRT = {0};
static int mazeDirty = 1;
static unsigned int mazeVersion = 0;

// Radial glow gradient shared by all light sources (baked once)
static Texture2D glowTex = {0};

void MarkMazeDirty(void)
{
    mazeDirty = 1;
    mazeVersion++;
}

unsigned int GetMazeVersion(void)
{
    return mazeVersion;
}

int ScreenToCell(double mx, double my, int *gx, int *gy)
{
    int j = (int)floor((mx - gridOffsetX) / cellSize) + 1;
    int i = (int)floor((my - gridOffsetY) / cellSize) + 1;
    if (i < 1 || i > Row || j < 1 || j > Col) return 0;
    *gx = i;
    *gy = j;
    return 1;
}

static void BakeGlowTexture(void)
{
    if (glowTex.id != 0) return;
    const int S = 128;
    Image img = GenImageColor(S, S, BLANK);
    Color *px = (Color*)img.data;
    float half = S / 2.0f;
    for (int y = 0; y < S; y++) {
        for (int x = 0; x < S; x++) {
            float dx = (x + 0.5f - half) / half;
            float dy = (y + 0.5f - half) / half;
            float d = sqrtf(dx*dx + dy*dy);
            float a = 1.0f - d;
            if (a < 0) a = 0;
            a = a * a * (3.0f - 2.0f * a); // smoothstep falloff
            px[y*S + x] = (Color){255, 255, 255, (unsigned char)(a * 255.0f)};
        }
    }
    glowTex = LoadTextureFromImage(img);
    UnloadImage(img);
}

static void EnsureMazeRT(void)
{
    int w = (int)(Col * cellSize) + 2;
    int h = (int)(Row * cellSize) + 2;
    if (mazeRT.texture.id == 0 || mazeRT.texture.width < w || mazeRT.texture.height < h) {
        if (mazeRT.texture.id != 0) UnloadRenderTexture(mazeRT);
        mazeRT = LoadRenderTexture(w, h);
        mazeDirty = 1;
    }
    if (fogRT.texture.id == 0 || fogRT.texture.width < w || fogRT.texture.height < h) {
        if (fogRT.texture.id != 0) UnloadRenderTexture(fogRT);
        fogRT = LoadRenderTexture(w, h);
        MarkFogDirty();
    }
}

static void BakeGlowRadial(double px, double py, double radius, Color tint)
{
    Rectangle src = {0, 0, (float)glowTex.width, (float)glowTex.height};
    Rectangle dst = {(float)(px - radius), (float)(py - radius),
                     (float)(radius * 2), (float)(radius * 2)};
    DrawTexturePro(glowTex, src, dst, (Vector2){0, 0}, 0, tint);
}

// Scan the map once and cache the positions of everything rendered per-frame,
// so neither the top-down nor the FP renderer needs to sweep the grid.
static void CacheMapPoints(void)
{
    startCellX = startCellY = endCellX = endCellY = -1;
    for (int d = 0; d < 3; d++) {
        colorKeyPos[d][0] = colorKeyPos[d][1] = -1;
        colorDoorPos[d][0] = colorDoorPos[d][1] = -1;
    }
    bonusCellCount = 0;

    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            int v = map_change[i][j];
            switch (v) {
                case CELL_START: startCellX = i; startCellY = j; break;
                case CELL_EXIT:  endCellX = i; endCellY = j; break;
                case CELL_KEY:   xk = i; yk = j; break;
                case CELL_KEY_RED: case CELL_KEY_BLUE: case CELL_KEY_GREEN:
                    colorKeyPos[v - CELL_KEY_RED][0] = i;
                    colorKeyPos[v - CELL_KEY_RED][1] = j;
                    break;
                case CELL_DOOR_RED: case CELL_DOOR_BLUE: case CELL_DOOR_GREEN:
                    colorDoorPos[v - CELL_DOOR_RED][0] = i;
                    colorDoorPos[v - CELL_DOOR_RED][1] = j;
                    break;
                case CELL_COIN: case CELL_GEM:
                    if (bonusCellCount < 32) {
                        bonusCells[bonusCellCount].x = i;
                        bonusCells[bonusCellCount].y = j;
                        bonusCells[bonusCellCount].kind = v;
                        bonusCellCount++;
                    }
                    break;
            }
        }
    }
}

// ========== Map Generation ==========
int can_set(int x, int y)
{
    int i, count = 0;
    for (i = 0; i < 4; i++) {
        int x1 = x + direction[i][0];
        int y1 = y + direction[i][1];
        if (map[x1][y1] == CELL_ROAD || map[x1][y1] == CELL_BORDER) count++;
    }
    return (count <= 1) ? 1 : 0;
}

void CreatMap(void)
{
    int i, j;
    struct location road[10000];

    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            map[i][j] = CELL_WALL;

    for (i = 0; i <= Row + 1; i++) {
        map[i][0] = CELL_BORDER;
        map[i][Col + 1] = CELL_BORDER;
    }
    for (j = 0; j <= Col + 1; j++) {
        map[0][j] = CELL_BORDER;
        map[Row + 1][j] = CELL_BORDER;
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
            map[x][y] = CELL_ROAD;
            for (i = 0; i < 4; i++) {
                int x_next = x + direction[i][0];
                int y_next = y + direction[i][1];
                if (map[x_next][y_next] == CELL_WALL) {
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

    map[2][2] = CELL_START;
    int found = 0;
    for (int col = Col - 1; col >= 1 && !found; col--) {
        for (i = Row; i >= 1; i--) {
            if (map[i][col] == CELL_ROAD) {
                map[i][col] = CELL_EXIT;
                found = 1;
                break;
            }
        }
    }

    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            map_change[i][j] = map[i][j];

    MarkMazeDirty();
}

// ========== Baked Rendering ==========
static int noise2d(int x, int y)
{
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) % 1000;
}

static void DrawWallAt(double px, double py, int gx, int gy)
{
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

static void DrawFloorAt(double px, double py, int gx, int gy)
{
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

static void BakeMaze(void)
{
    EnsureMazeRT();
    BeginTextureMode(mazeRT);
    ClearBackground(BLANK);
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            int v = map_change[i][j];
            double px = (j - 1) * cellSize;
            double py = (i - 1) * cellSize;
            if (v == CELL_WALL || v == CELL_BORDER) DrawWallAt(px, py, i, j);
            else DrawFloorAt(px, py, i, j);
        }
    }
    EndTextureMode();
    mazeDirty = 0;
    CacheMapPoints();
}

static void BakeFog(void)
{
    if (!IsFogDirty()) return;
    EnsureMazeRT();
    BeginTextureMode(fogRT);
    ClearBackground(BLANK);
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (!IsExplored(i, j)) {
                double fx = (j - 1) * cellSize;
                double fy = (i - 1) * cellSize;
                DrawRectangle((int)fx, (int)fy, (int)cellSize + 1, (int)cellSize + 1, BLACK);
            }
        }
    }
    EndTextureMode();
    ClearFogDirty();
}

// ========== Animated Feature Drawing ==========
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
}

static const Color keyColors[3] = {{210, 70, 60, 255}, {70, 100, 210, 255}, {70, 170, 80, 255}};

static void DrawColoredKeysAndDoors(float cs)
{
    for (int d = 0; d < 3; d++) {
        if (colorKeyPos[d][0] >= 1 && IsExplored(colorKeyPos[d][0], colorKeyPos[d][1])) {
            double px = gridOffsetX + (colorKeyPos[d][1] - 0.5) * cellSize;
            double py = gridOffsetY + (colorKeyPos[d][0] - 0.5) * cellSize;
            DrawCircle((int)px, (int)py, cs * 0.14f, keyColors[d]);
            DrawCircleLines((int)px, (int)py, cs * 0.14f, (Color){255, 255, 255, 180});
        }
        if (colorDoorPos[d][0] >= 1 && IsExplored(colorDoorPos[d][0], colorDoorPos[d][1])) {
            double px = gridOffsetX + (colorDoorPos[d][1] - 1) * cellSize;
            double py = gridOffsetY + (colorDoorPos[d][0] - 1) * cellSize;
            Color c = keyColors[d];
            DrawRectangle((int)px, (int)py, (int)cs, (int)cs, (Color){c.r, c.g, c.b, 110});
            DrawRectangleLines((int)px, (int)py, (int)cs, (int)cs, c);
        }
    }
}

// Bake the maze layer (and refresh feature caches) if dirty. The FP renderer
// relies on the caches without going through Drawmap, so it calls this too.
void EnsureMapCaches(void)
{
    if (mazeDirty) BakeMaze();
}

void Drawmap(int a[][100])
{
    EnsureMapCaches();
    float cs = (float)cellSize;

    // Static floor/wall layer: one baked texture quad
    DrawTextureRec(mazeRT.texture,
                   (Rectangle){0, 0, (float)mazeRT.texture.width, -(float)mazeRT.texture.height},
                   (Vector2){(float)gridOffsetX, (float)gridOffsetY}, WHITE);

    // Solution path glow
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (a[i][j] == CELL_PATH) {
                double px = gridOffsetX + (j - 1) * cellSize;
                double py = gridOffsetY + (i - 1) * cellSize;
                DrawRectangle((int)px, (int)py, (int)cs, (int)cs, (Color){0, 200, 220, 80});
            }
        }
    }

    // Animated features (cached positions instead of full-map scans)
    if (startCellX >= 1) {
        DrawStartPoint(gridOffsetX + (startCellY - 1) * cellSize,
                       gridOffsetY + (startCellX - 1) * cellSize, cs);
    }
    if (endCellX >= 1) {
        DrawEndPoint(gridOffsetX + (endCellY - 1) * cellSize,
                     gridOffsetY + (endCellX - 1) * cellSize, cs);
    }
    if (!is_key && xk >= 1) {
        DrawKeyItem(gridOffsetX + (yk - 1) * cellSize,
                    gridOffsetY + (xk - 1) * cellSize, cs);
    }

    DrawColoredKeysAndDoors(cs);

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

    if (glowTex.id == 0) BakeGlowTexture();

    // === Fog of war: one baked texture of black cells over unexplored area ===
    BakeFog();
    DrawTextureRec(fogRT.texture,
                   (Rectangle){0, 0, (float)fogRT.texture.width, -(float)fogRT.texture.height},
                   (Vector2){(float)gridOffsetX, (float)gridOffsetY}, WHITE);

    // Semi-transparent darkness over explored area
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){5, 5, 15, 130});

    // === Additive glow for light sources (gradient textures instead of concentric circles) ===
    BeginBlendMode(BLEND_ADDITIVE);

    // Player torch - warm flickering glow
    float flicker = 0.92f + 0.08f * sinf(time * 12.0f) + 0.04f * sinf(time * 27.0f);
    BakeGlowRadial(px, py, cellSize * 2.5 * flicker, (Color){255, 180, 80, 25});
    DrawCircle((int)px, (int)py, (float)(cellSize * 0.8), (Color){255, 200, 100, 8});

    // Key light (golden)
    if (!is_key && xk >= 1) {
        double kx = gridOffsetX + (yk - 0.5) * cellSize;
        double ky = gridOffsetY + (xk - 0.5) * cellSize;
        BakeGlowRadial(kx, ky, cellSize * 1.5, (Color){255, 215, 0, 40});
    }

    // End point light (purple magic)
    if (endCellX >= 1) {
        double ex = gridOffsetX + (endCellY - 0.5) * cellSize;
        double ey = gridOffsetY + (endCellX - 0.5) * cellSize;
        BakeGlowRadial(ex, ey, cellSize * 1.8, (Color){150, 80, 255, 45});
    }

    // Torch item boost
    if (torchBoostTime > 0) {
        DrawCircle((int)px, (int)py, (float)(cellSize * 2.0), (Color){255, 200, 100, 30});
    }

    EndBlendMode();
}

void DrawVignette(void)
{
    static Texture2D vignetteTex = {0};
    static int bakedForW = 0, bakedForH = 0;

    int w = GetScreenWidth();
    int h = GetScreenHeight();
    if (vignetteTex.id == 0 || bakedForW != w || bakedForH != h) {
        if (vignetteTex.id != 0) UnloadTexture(vignetteTex);
        // Bake at half resolution and upscale - the gradient is smooth anyway
        int tw = w / 2, th = h / 2;
        Image img = GenImageColor(tw, th, BLANK);
        Color *pxs = (Color*)img.data;
        int v = 60; // 120 screen px at half res
        for (int y = 0; y < th; y++) {
            int dv = (y < v) ? y : (y >= th - v ? th - 1 - y : v);
            float av = 100.0f / 255.0f * (1.0f - (float)dv / v);
            for (int x = 0; x < tw; x++) {
                int dh = (x < v) ? x : (x >= tw - v ? tw - 1 - x : v);
                float ah = 80.0f / 255.0f * (1.0f - (float)dh / v);
                // Corners got both overlays in the original: combine the alphas
                float a = 1.0f - (1.0f - av) * (1.0f - ah);
                pxs[y*tw + x] = (Color){0, 0, 0, (unsigned char)(a * 255.0f)};
            }
        }
        vignetteTex = LoadTextureFromImage(img);
        UnloadImage(img);
        bakedForW = w;
        bakedForH = h;
    }

    DrawTexturePro(vignetteTex,
                   (Rectangle){0, 0, (float)vignetteTex.width, (float)vignetteTex.height},
                   (Rectangle){0, 0, (float)w, (float)h},
                   (Vector2){0, 0}, 0, WHITE);
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

// ========== Per-frame entity updates (kept out of the draw functions) ==========
void UpdateCollectibles(float dt)
{
    for (int i = 0; i < coinCount; i++) {
        if (coins[i].active) coins[i].animTime += dt;
    }
}

void SpawnAmbientParticles(void)
{
    // Key sparkle
    if (!is_key && xk >= 1 && rand() % 3 == 0) {
        SpawnKeyGlow((float)(gridOffsetX + (yk - 0.5) * cellSize),
                     (float)(gridOffsetY + (xk - 0.5) * cellSize));
    }
    // Exit magic
    if (endCellX >= 1 && rand() % 4 == 0) {
        SpawnMagic((float)(gridOffsetX + (endCellY - 0.5) * cellSize),
                   (float)(gridOffsetY + (endCellX - 0.5) * cellSize));
    }
}

// ========== Cell-entry logic ==========

void Judgekey(void)
{
    if (map_change[X][Y] == CELL_KEY) {
        map_change[X][Y] = CELL_ROAD;
        is_key = 1;
        MarkMazeDirty();
    }
}

void Randomkey(void)
{
    int roads[1000][2];
    int roadCount = 0;
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (map_change[i][j] == CELL_ROAD && !(i == 2 && j == 2)) {
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
        map_change[xk][yk] = CELL_KEY;
    }
    is_key = 0;
    step = 0;
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

    if (!hiddenRoomFound && X == hiddenRoomX && Y == hiddenRoomY &&
        hiddenRoomX > 0 && hiddenRoomY > 0) {
        hiddenRoomFound = 1;
        AddScore(100);
    }

    // Multi-key pickup (colored keys)
    if (map_change[X][Y] >= CELL_KEY_RED && map_change[X][Y] <= CELL_KEY_GREEN) {
        int keyIdx = map_change[X][Y] - CELL_KEY_RED;
        keysCollected[keyIdx] = 1;
        map_change[X][Y] = CELL_ROAD;
        PlayKeySound();
        AddScore(50);
        MarkMazeDirty(); // refresh the colored-key/door position caches
        if (keysCollected[0] && keysCollected[1] && keysCollected[2])
            UnlockAchievement(6); // Key Master
    }
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
    hiddenRoomX = hiddenRoomY = 0;
    trapTickFraction = 0;
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
        if (map_change[cx][cy] == CELL_ROAD && !(cx == 2 && cy == 2)) {
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
        if (map_change[tx][ty] == CELL_ROAD && (abs(tx - 2) + abs(ty - 2)) > 4) {
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
            if (map_change[x1][y1] == CELL_ROAD && map_change[x2][y2] == CELL_ROAD &&
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
            if (map_change[bx + direction[d][0]][by + direction[d][1]] == CELL_KEY) nearKey = 1;
        }
        if (map_change[bx][by] == CELL_ROAD && !nearKey && (abs(bx-2)+abs(by-2)) > 5) {
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
        if (map_change[px][py] == CELL_ROAD) {
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
    // Only carve ordinary wall/road cells, preserving the start, exit and keys.
    int hx = 0, hy = 0;
    hiddenRoomX = hiddenRoomY = 0;
    for (int attempt = 0; attempt < 200; attempt++) {
        int side = rand() % 4;
        if (side == 0) { hx = 3; hy = rand() % (Col - 6) + 3; }
        else if (side == 1) { hx = Row - 4; hy = rand() % (Col - 6) + 3; }
        else if (side == 2) { hx = rand() % (Row - 6) + 3; hy = 3; }
        else { hx = rand() % (Row - 6) + 3; hy = Col - 4; }

        int safe = 1, roads = 0;
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {
                int cell = map_change[hx + i][hy + j];
                if (cell != CELL_WALL && cell != CELL_ROAD) safe = 0;
                if (cell == CELL_ROAD) roads++;
            }
        if (safe && roads > 0) {
            hiddenRoomX = hx;
            hiddenRoomY = hy;
            break;
        }
    }
    if (hiddenRoomX == 0) return;

    // Carve room
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if (hx+i >= 1 && hx+i <= Row && hy+j >= 1 && hy+j <= Col)
                map_change[hx+i][hy+j] = CELL_ROAD;

    // Put gems and items in room
    if (coinCount < MAX_COINS) {
        coins[coinCount].x = hx; coins[coinCount].y = hy;
        coins[coinCount].value = 50; coins[coinCount].active = 1;
        coins[coinCount].animTime = 0; coinCount++;
    }

    SpawnItem(hx, hy + (hy < Col/2 ? 1 : -1), ITEM_BOMB);
    MarkMazeDirty();
}

void UpdateTraps(float dt)
{
    trapTickFraction += dt * 60.0f;
    int elapsedTicks = (int)trapTickFraction;
    trapTickFraction -= elapsedTicks;
    for (int i = 0; i < trapCount; i++) {
        if (traps[i].active) {
            traps[i].cycle = (traps[i].cycle + elapsedTicks) % 121;
        }
    }
    for (int i = 0; i < portalCount; i++) {
        if (portals[i].cooldown > 0) portals[i].cooldown -= dt;
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
    if (map_change[x][y] == CELL_WALL) return 0;
    // Check boxes
    for (int i = 0; i < boxCount; i++) {
        if (boxes[i].active && boxes[i].x == x && boxes[i].y == y) return 2; // box
    }
    // Check doors
    if (map_change[x][y] >= CELL_DOOR_RED && map_change[x][y] <= CELL_DOOR_GREEN) {
        int doorIdx = map_change[x][y] - CELL_DOOR_RED;
        if (keysCollected[doorIdx]) return 1; // can open
        return 0;
    }
    return 1;
}

int OpenDoorIfUnlocked(int x, int y)
{
    if (x < 1 || x > Row || y < 1 || y > Col) return 0;
    int cell = map_change[x][y];
    if (cell < CELL_DOOR_RED || cell > CELL_DOOR_GREEN) return 1;
    if (!keysCollected[cell - CELL_DOOR_RED]) return 0;
    map_change[x][y] = CELL_ROAD;
    PlayDoorSound();
    MarkMazeDirty();
    return 1;
}

int TryPushBox(int bx, int by, int dx, int dy)
{
    int nx = bx + dx, ny = by + dy;
    if (nx < 1 || nx > Row || ny < 1 || ny > Col) return 0;
    if (map_change[nx][ny] == CELL_WALL) return 0;
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
                            map_change[doorPositions[d][0]][doorPositions[d][1]] = CELL_ROAD;
                            doorExists[d] = 0;
                            PlayDoorSound();
                            MarkMazeDirty();
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
                if (map_change[x][y] == CELL_WALL) {
                    map_change[x][y] = CELL_ROAD;
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
    MarkMazeDirty();
}
