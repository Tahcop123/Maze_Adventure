#include "inc.h"

// ========== Global Variable Definitions ==========
GameState gameState = STATE_MENU;
int screenWidth = 1280;
int screenHeight = 800;
PendingAction pendingAction = PENDING_NONE;

// Transient HUD feedback for save/load actions
static float hudMsgTime = 0;
static char hudMsg[64] = {0};

static void ShowHudMsg(const char *msg)
{
    snprintf(hudMsg, sizeof(hudMsg), "%s", msg);
    hudMsgTime = 2.0f;
}

// Character & Level
int selectedCharacter = CHAR_KNIGHT;
int currentLevel = 0;
int difficulty = DIFF_NORMAL;
unsigned int mapSeed = 0;
int timedMode = 0;
float timeRemaining = 0;

// Score & Collectibles
int score = 0;
int coinsCollected = 0;
int gemsCollected = 0;
int totalCoins = 0;
int totalGems = 0;

// Stamina
float stamina = 100.0f;
float maxStamina = 100.0f;
int isSprinting = 0;

// Inventory
int inventory[4] = {0, 0, 0, 0};
int shieldActive = 0;
float speedBoostTime = 0;
float torchBoostTime = 0;
int rogueFreeSteps = 0;

// Fog & View
int viewRadius = 4;

// Ice map (defined in map.c)

// Multi-key multi-door
int keysCollected[3] = {0, 0, 0};
int doorPositions[3][2] = {{0,0},{0,0},{0,0}};
int doorExists[3] = {0, 0, 0};

// Screen shake
float shakeAmount = 0;
float shakeTime = 0;

// Game over reason
int gameOverReason = 0;

// Flash effect
float flashTime = 0;
Color flashColor = {0, 0, 0, 0};

// Animation
int playerDir = 1;
float animFrame = 0;
float animTimer = 0;
int playerAttacking = 0;
float attackAnimTime = 0;

// Combat
int playerAttack = 10;
int playerDefense = 0;
float attackCooldown = 0;
int kills = 0;

// Run statistics (achievement tracking)
int tookDamageThisLevel = 0;
int bombsUsedTotal = 0;
int hintsUsed = 0;
int iceSlideCount = 0;

// First Person
int fpMode = 0;
float fpDirX = 1, fpDirY = 0;
float fpPlaneX = 0, fpPlaneY = 0.66f;
float fpPosX = 2.5f, fpPosY = 2.5f;
float fpMoveSpeed = 3.0f;
int fpMouseLook = 0;

// ========== Helper Functions ==========
void TriggerShake(float amount, float duration)
{
    shakeAmount = amount;
    shakeTime = duration;
}

void TriggerFlash(Color color, float duration)
{
    flashColor = color;
    flashTime = duration;
}

void UpdateEffects(void)
{
    float dt = GetFrameTime();
    if (shakeTime > 0) {
        shakeTime -= dt;
        if (shakeTime <= 0) shakeAmount = 0;
    }
    if (flashTime > 0) {
        flashTime -= dt;
    }
    if (speedBoostTime > 0) speedBoostTime -= dt;
    if (torchBoostTime > 0) torchBoostTime -= dt;
}

void AddScore(int points)
{
    score += points;
}

// Menu torch-flicker particles (update side; spawn no longer lives in DrawInterface)
static void UpdateMenuAmbient(void)
{
    if (rand() % 3 == 0) {
        SpawnParticle(rand() % screenWidth, screenHeight * 0.3f + rand() % (int)(screenHeight*0.4f),
                      (float)(rand()%100-50)/50.0f, -1.0f - (float)(rand()%50)/50.0f,
                      1.5f, 3.0f, (Color){255, 150+rand()%80, 50, 200});
    }
}

// ========== Game Init ==========
static void InitGame(void)
{
    LoadHighScores();
    LoadAchievements();
    InitAudio();
    InitParticles();
    StartLevel(0);
}

// ========== Drawing ==========
// The BFS solution is only recomputed when the player moved or the map
// changed - never once per frame.
static int solCacheX = -1, solCacheY = -1;
static unsigned int solCacheVersion = 0;

static void UpdateSolutionCache(void)
{
    if (X == solCacheX && Y == solCacheY && solCacheVersion == GetMazeVersion()) return;
    OptimalSolution();
    solCacheX = X;
    solCacheY = Y;
    solCacheVersion = GetMazeVersion();
}

static void DrawCollectibles(void)
{
    for (int i = 0; i < coinCount; i++) {
        if (!coins[i].active) continue;
        if (!IsExplored(coins[i].x, coins[i].y)) continue;
        double px = gridOffsetX + (coins[i].y - 0.5) * cellSize;
        double py = gridOffsetY + (coins[i].x - 0.5) * cellSize;
        float bob = sinf(coins[i].animTime * 3.0f) * cellSize * 0.05f;
        if (coins[i].value >= 50) {
            // Gem
            DrawCircle((int)px, (int)(py + bob), (float)cellSize * 0.18f, (Color){100, 200, 255, 255});
            DrawCircle((int)(px - 2), (int)(py + bob - 2), (float)cellSize * 0.08f, (Color){180, 230, 255, 255});
        } else {
            // Coin
            DrawCircle((int)px, (int)(py + bob), (float)cellSize * 0.15f, GOLD);
            DrawCircle((int)px, (int)(py + bob), (float)cellSize * 0.08f, (Color){255, 220, 100, 255});
        }
    }
}

static void DrawTraps(void)
{
    for (int i = 0; i < trapCount; i++) {
        if (!traps[i].active) continue;
        if (!IsExplored(traps[i].x, traps[i].y)) continue;
        double px = gridOffsetX + (traps[i].y - 1) * cellSize;
        double py = gridOffsetY + (traps[i].x - 1) * cellSize;
        int active = (traps[i].cycle < 60);
        // Base plate
        DrawRectangle((int)(px + 2), (int)(py + 2), (int)(cellSize - 4), (int)(cellSize - 4), (Color){60, 60, 65, 200});
        if (active) {
            // Spikes
            for (int s = 0; s < 3; s++) {
                int sx = (int)(px + cellSize * 0.2 + s * cellSize * 0.25);
                DrawTriangle((Vector2){sx, (float)(py + cellSize * 0.7)},
                             (Vector2){sx + cellSize * 0.12f, (float)(py + cellSize * 0.7)},
                             (Vector2){sx + cellSize * 0.06f, (float)(py + cellSize * 0.25)}, (Color){180, 180, 190, 255});
            }
        } else {
            // Warning dots
            DrawCircle((int)(px + cellSize*0.3), (int)(py + cellSize*0.5), 2, (Color){255, 100, 100, 150});
            DrawCircle((int)(px + cellSize*0.7), (int)(py + cellSize*0.5), 2, (Color){255, 100, 100, 150});
        }
    }
}

static void DrawPortals(void)
{
    for (int i = 0; i < portalCount; i++) {
        if (!portals[i].active) continue;
        float t = GetTime();
        Color colors[2] = {{150, 80, 255, 200}, {80, 150, 255, 200}};
        int positions[2][2] = {{portals[i].x1, portals[i].y1}, {portals[i].x2, portals[i].y2}};
        for (int p = 0; p < 2; p++) {
            if (!IsExplored(positions[p][0], positions[p][1])) continue;
            double px = gridOffsetX + (positions[p][1] - 0.5) * cellSize;
            double py = gridOffsetY + (positions[p][0] - 0.5) * cellSize;
            float rot = t * 2.0f + p * PI;
            DrawCircle((int)px, (int)py, (float)cellSize * 0.35f, (Color){colors[p].r, colors[p].g, colors[p].b, 60});
            for (int a = 0; a < 6; a++) {
                float angle = rot + a * PI / 3;
                DrawCircle((int)(px + cosf(angle) * cellSize * 0.25),
                           (int)(py + sinf(angle) * cellSize * 0.25), 3, colors[p]);
            }
            DrawCircle((int)px, (int)py, (float)cellSize * 0.12f, (Color){255, 255, 255, 150});
        }
    }
}

static void DrawBoxes(void)
{
    for (int i = 0; i < boxCount; i++) {
        if (!boxes[i].active) continue;
        if (!IsExplored(boxes[i].x, boxes[i].y)) continue;
        double px = gridOffsetX + (boxes[i].y - 1) * cellSize;
        double py = gridOffsetY + (boxes[i].x - 1) * cellSize;
        DrawRectangle((int)(px + 3), (int)(py + 3), (int)(cellSize - 6), (int)(cellSize - 6), (Color){139, 90, 43, 255});
        DrawRectangleLines((int)(px + 3), (int)(py + 3), (int)(cellSize - 6), (int)(cellSize - 6), (Color){80, 50, 20, 255});
        DrawLine((int)(px + 3), (int)(py + cellSize/2), (int)(px + cellSize - 3), (int)(py + cellSize/2), (Color){80, 50, 20, 200});
        DrawLine((int)(px + cellSize/2), (int)(py + 3), (int)(px + cellSize/2), (int)(py + cellSize - 3), (Color){80, 50, 20, 200});
    }
}

static void DrawPlates(void)
{
    for (int i = 0; i < plateCount; i++) {
        if (!plates[i].active) continue;
        if (!IsExplored(plates[i].x, plates[i].y)) continue;
        double px = gridOffsetX + (plates[i].y - 1) * cellSize;
        double py = gridOffsetY + (plates[i].x - 1) * cellSize;
        Color c = plates[i].pressed ? (Color){100, 200, 100, 200} : (Color){150, 150, 160, 200};
        DrawCircle((int)(px + cellSize/2), (int)(py + cellSize/2), (float)cellSize * 0.3f, c);
        DrawCircle((int)(px + cellSize/2), (int)(py + cellSize/2), (float)cellSize * 0.2f, (Color){c.r+30, c.g+30, c.b+30, 200});
    }
}

static void DrawIce(void)
{
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (iceMap[i][j] && IsExplored(i, j)) {
                double px = gridOffsetX + (j - 1) * cellSize;
                double py = gridOffsetY + (i - 1) * cellSize;
                DrawRectangle((int)px, (int)py, (int)cellSize, (int)cellSize, (Color){150, 200, 230, 100});
                DrawLine((int)(px + cellSize*0.2), (int)(py + cellSize*0.3), (int)(px + cellSize*0.5), (int)(py + cellSize*0.6), (Color){200, 230, 255, 150});
                DrawLine((int)(px + cellSize*0.6), (int)(py + cellSize*0.2), (int)(px + cellSize*0.8), (int)(py + cellSize*0.5), (Color){200, 230, 255, 150});
            }
        }
    }
}

static void DrawInventoryHUD(void)
{
    int ix = 20, iy = screenHeight - 70;
    const char *icons[4] = {"1", "2", "3", "4"};
    const char *names[4] = {"Bomb", "Speed", "Shield", "Torch"};
    Color colors[4] = {{80,80,90,220}, {80,150,255,220}, {200,170,50,220}, {255,150,50,220}};

    for (int i = 0; i < 4; i++) {
        int bx = ix + i * 65;
        DrawRectangleRounded((Rectangle){bx, iy, 55, 55}, 0.15, 4, colors[i]);
        DrawRectangleRoundedLines((Rectangle){bx, iy, 55, 55}, 0.15, 4, (Color){200,200,210,200});
        DrawText(icons[i], bx + 5, iy + 3, 12, WHITE);
        char count[8];
        snprintf(count, sizeof(count), "x%d", inventory[i]);
        DrawText(count, bx + 15, iy + 25, 16, WHITE);
        DrawText(names[i], bx + 5, iy + 58, 10, (Color){180,180,190,200});
    }

    // Active effects
    int ex = ix + 4 * 65 + 20;
    if (shieldActive) { DrawText("SHIELD", ex, iy + 10, 14, (Color){255, 220, 100, 255}); ex += 70; }
    if (speedBoostTime > 0) { DrawText(TextFormat("SPD %.0fs", speedBoostTime), ex, iy + 10, 14, (Color){100, 200, 255, 255}); ex += 80; }
    if (torchBoostTime > 0) { DrawText(TextFormat("TORCH %.0fs", torchBoostTime), ex, iy + 10, 14, (Color){255, 180, 50, 255}); }
}

static void DrawGame(void)
{
    ClearBackground((Color){8, 8, 15, 255});

    // Screen shake
    Camera2D cam = {0};
    cam.zoom = 1.0f;
    if (shakeTime > 0) {
        float ox = ((rand() / (float)RAND_MAX) - 0.5f) * shakeAmount;
        float oy = ((rand() / (float)RAND_MAX) - 0.5f) * shakeAmount;
        cam.target = (Vector2){ox, oy};
        cam.offset = (Vector2){-ox, -oy};
    }
    if (fpMode) {
        // First person raycasting view
        DrawFPView();
    } else {
        BeginMode2D(cam);

        // Draw maze base (solution BFS runs only when stale, not per frame)
        if (is_showsolution) {
            UpdateSolutionCache();
            Drawmap(map);
        } else {
            Drawmap(map_change);
        }

        // Draw entities
        DrawIce();
        DrawPlates();
        DrawCollectibles();
        DrawTraps();
        DrawPortals();
        DrawBoxes();
        DrawItems();
        DrawEnemies();

        // Tip highlight
        if (is_showtip && !is_showsolution) {
            UpdateSolutionCache();
            double px = gridOffsetX + (yy - 0.5) * cellSize;
            double py = gridOffsetY + (xx - 0.5) * cellSize;
            DrawRectangle((int)(px - cellSize/2), (int)(py - cellSize/2), (int)cellSize, (int)cellSize, (Color){0, 220, 220, 150});
        }

        // Lighting
        DrawLighting();

        // Particles
        DrawParticles();

        // Edit mode
        if (is_edit) {
            Vector2 mouse = GetMousePosition();
            int gx, gy;
            if (ScreenToCell(mouse.x, mouse.y, &gx, &gy)) {
                double px = gridOffsetX + (gy - 1) * cellSize;
                double py = gridOffsetY + (gx - 1) * cellSize;
                DrawRectangleLines((int)px, (int)py, (int)cellSize, (int)cellSize, RED);
            }
        }

        EndMode2D();
    }

    // Flash effect
    if (flashTime > 0) {
        Color fc = flashColor;
        fc.a = (unsigned char)(fc.a * (flashTime / 0.15f));
        DrawRectangle(0, 0, screenWidth, screenHeight, fc);
    }

    // HUD (not affected by shake/lighting)
    DrawTopBar();
    DrawMinimap(screenWidth - 180, 55, 160, 110);
    DrawInventoryHUD();

    // Vignette
    DrawVignette();

    // Edit mode text
    if (is_edit) {
        DrawText("EDIT MODE - Drag to swap | Q: wall | W: road | 1-4: items",
                 (int)gridOffsetX, (int)(gridOffsetY + Row * cellSize + 8), 14, (Color){255, 100, 100, 255});
    }

    // Time limit
    if (timedMode) {
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "Time: %.0f", timeRemaining);
        Color tc = (timeRemaining < 15) ? RED : WHITE;
        DrawText(timeStr, screenWidth / 2 - 40, 50, 24, tc);
    }
}

static void DrawPauseMenu(void)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});
    const char *title = "PAUSED";
    DrawText(title, (screenWidth - MeasureText(title, 48)) / 2, 150, 48, WHITE);

    int bw = 200, bh = 45, bx = (screenWidth - bw) / 2, by = 250;
    if (Button(bx, by, bw, bh, "Resume", (Color){50, 150, 80, 255}, WHITE)) { gameState = STATE_MAZE; return; }
    by += bh + 15;
    // Save/Load do file I/O: return so later buttons can't re-trigger, and
    // report the outcome instead of failing silently
    if (Button(bx, by, bw, bh, "Save Game", (Color){50, 120, 180, 255}, WHITE)) {
        ShowHudMsg(SaveGame() ? "Game saved" : "Save failed!");
        gameState = STATE_MAZE;
        return;
    }
    by += bh + 15;
    if (Button(bx, by, bw, bh, "Load Game", (Color){120, 100, 180, 255}, WHITE)) {
        if (!LoadGame()) ShowHudMsg("Load failed (no save file?)");
        return; // LoadGame switches to STATE_MAZE itself on success
    }
    by += bh + 15;
    if (Button(bx, by, bw, bh, "Quit to Menu", (Color){180, 80, 80, 255}, WHITE)) { gameState = STATE_MENU; return; }
}

static void DrawGameOver(void)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){20, 0, 0, 220});
    const char *title = "GAME OVER";
    DrawText(title, (screenWidth - MeasureText(title, 56)) / 2, 120, 56, RED);

    const char *reason = (gameOverReason == 1) ? "Time's up!" : (gameOverReason == 2 ? "Defeated by enemy!" : "HP depleted!");
    DrawText(reason, (screenWidth - MeasureText(reason, 24)) / 2, 200, 24, (Color){255, 150, 150, 255});

    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %d  |  Steps: %d  |  Level: %d", score, step, currentLevel + 1);
    DrawText(scoreStr, (screenWidth - MeasureText(scoreStr, 22)) / 2, 260, 22, WHITE);

    int bw = 200, bh = 45, bx = (screenWidth - bw) / 2, by = 340;
    if (Button(bx, by, bw, bh, "Retry Level", (Color){50, 150, 80, 255}, WHITE)) { score = 0; StartLevel(currentLevel); gameState = STATE_MAZE; return; }
    by += bh + 15;
    if (Button(bx, by, bw, bh, "Main Menu", (Color){100, 100, 120, 255}, WHITE)) { gameState = STATE_MENU; return; }
}

static void DrawWin(void)
{
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 20, 10, 220});
    const char *title = "VICTORY!";
    DrawText(title, (screenWidth - MeasureText(title, 56)) / 2, 120, 56, (Color){100, 255, 100, 255});

    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Final Score: %d  |  Steps: %d", score, step);
    DrawText(scoreStr, (screenWidth - MeasureText(scoreStr, 24)) / 2, 210, 24, WHITE);

    char collectStr[64];
    snprintf(collectStr, sizeof(collectStr), "Coins: %d/%d  |  Gems: %d/%d", coinsCollected, totalCoins, gemsCollected, totalGems);
    DrawText(collectStr, (screenWidth - MeasureText(collectStr, 20)) / 2, 255, 20, (Color){200, 200, 200, 255});

    int bw = 200, bh = 45, bx = (screenWidth - bw) / 2, by = 330;
    if (currentLevel < 2) {
        if (Button(bx, by, bw, bh, "Next Level", (Color){50, 150, 80, 255}, WHITE)) {
            NextLevel();
            gameState = STATE_MAZE;
            return;
        }
        by += bh + 15;
    }
    if (Button(bx, by, bw, bh, "Main Menu", (Color){100, 100, 120, 255}, WHITE)) { gameState = STATE_MENU; return; }
    // NOTE: high score is logged exactly once at the win trigger (main loop),
    // not per frame in this draw function.
}

// ========== Main Loop ==========
// ========== Animation ==========
void UpdateAnimation(float dt)
{
    animTimer += dt;
    if (animTimer > 0.15f) {
        animTimer = 0;
        animFrame = (float)(((int)animFrame + 1) % 4);
    }
    if (attackCooldown > 0) attackCooldown -= dt;
    if (playerAttacking) {
        attackAnimTime -= dt;
        if (attackAnimTime <= 0) playerAttacking = 0;
    }
}

void DrawPlayerAnimated(double px, double py, float cs)
{
    float radius = cs * 0.35f;
    float bob = sinf(animFrame * 1.57f) * 2.0f;
    float cy = (float)py + bob;

    // Shadow
    DrawCircle((int)px, (int)(py + radius * 0.9f), radius * 0.7f, (Color){0,0,0,100});

    // Body color by character
    Color bodyCol, accentCol;
    if (selectedCharacter == CHAR_KNIGHT) { bodyCol = (Color){70,80,110,255}; accentCol = (Color){200,180,100,255}; }
    else if (selectedCharacter == CHAR_MAGE) { bodyCol = (Color){90,50,120,255}; accentCol = (Color){100,200,255,255}; }
    else { bodyCol = (Color){50,100,60,255}; accentCol = (Color){200,200,80,255}; }

    // Body
    DrawCircle((int)px, (int)cy, radius, bodyCol);
    DrawCircle((int)(px - radius*0.3), (int)(cy - radius*0.3), radius*0.4f, (Color){bodyCol.r+30,bodyCol.g+30,bodyCol.b+30,255});

    // Head
    DrawCircle((int)px, (int)(cy - radius*0.6f), radius*0.55f, (Color){230,200,170,255});

    // Eyes based on direction
    float eyeOffX = 0, eyeOffY = 0;
    if (playerDir == 0) eyeOffY = -radius*0.15f;
    else if (playerDir == 1) eyeOffY = radius*0.15f;
    else if (playerDir == 2) eyeOffX = -radius*0.15f;
    else if (playerDir == 3) eyeOffX = radius*0.15f;
    DrawCircle((int)(px - radius*0.2f + eyeOffX), (int)(cy - radius*0.6f + eyeOffY), radius*0.1f, (Color){30,30,40,255});
    DrawCircle((int)(px + radius*0.2f + eyeOffX), (int)(cy - radius*0.6f + eyeOffY), radius*0.1f, (Color){30,30,40,255});

    // Accent (hat/helmet/hair)
    if (selectedCharacter == CHAR_KNIGHT) {
        DrawRectangle((int)(px - radius*0.5f), (int)(cy - radius*1.1f), (int)(radius), (int)(radius*0.4f), accentCol);
    } else if (selectedCharacter == CHAR_MAGE) {
        DrawTriangle((Vector2){px, cy - radius*1.5f}, (Vector2){px - radius*0.5f, cy - radius*0.7f}, (Vector2){px + radius*0.5f, cy - radius*0.7f}, accentCol);
    } else {
        DrawCircle((int)px, (int)(cy - radius*0.9f), radius*0.35f, accentCol);
    }

    // Attack swing effect
    if (playerAttacking) {
        float t = 1.0f - (attackAnimTime / 0.25f);
        float angle;
        if (playerDir == 0) angle = -PI/2;
        else if (playerDir == 1) angle = PI/2;
        else if (playerDir == 2) angle = PI;
        else angle = 0;
        angle += (t - 0.5f) * 1.5f;
        float sx = px + cosf(angle) * radius * 1.8f;
        float sy = cy + sinf(angle) * radius * 1.8f;
        DrawLine((int)px, (int)cy, (int)sx, (int)sy, (Color){255,255,220,(unsigned char)(200*(1-t))});
        DrawCircle((int)sx, (int)sy, radius*0.3f, (Color){255,255,200,(unsigned char)(150*(1-t))});
    }
}

// ========== Combat ==========
void PlayerAttack(void)
{
    if (attackCooldown > 0 || playerAttacking) return;
    playerAttacking = 1;
    attackAnimTime = 0.25f;
    attackCooldown = 0.35f;
    PlayItemSound();

    // Attack direction: in FP mode derive facing from the look vector
    int dir = playerDir;
    if (fpMode) {
        if (fabsf(fpDirY) > fabsf(fpDirX)) dir = (fpDirY > 0) ? 1 : 0;
        else dir = (fpDirX > 0) ? 3 : 2;
        playerDir = dir;
    }
    int dx = 0, dy = 0;
    if (dir == 0) dx = -1;
    else if (dir == 1) dx = 1;
    else if (dir == 2) dy = -1;
    else dy = 1;

    // Check enemies in front (1-2 cells)
    for (int range = 1; range <= 2; range++) {
        int tx = X + dx * range;
        int ty = Y + dy * range;
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].active) continue;
            if (enemies[i].x == tx && enemies[i].y == ty) {
                DamageEnemy(i, playerAttack + rand() % 5);
            }
        }
    }
    TriggerShake(2.0f, 0.08f);
}

void DamageEnemy(int idx, int dmg)
{
    if (idx < 0 || idx >= enemyCount || !enemies[idx].active) return;
    enemies[idx].hp -= dmg;
    enemies[idx].isHurt = 1;
    enemies[idx].hurtTime = 0.2f;
    PlayHurtSound();
    TriggerShake(3.0f, 0.1f);

    // Spawn hit particles
    double ex = gridOffsetX + (enemies[idx].y - 0.5) * cellSize;
    double ey = gridOffsetY + (enemies[idx].x - 0.5) * cellSize;
    for (int i = 0; i < 6; i++) {
        SpawnParticle(ex, ey, (rand()%100-50)*0.1f, (rand()%100-50)*0.1f, 0.4f, 3, (Color){255,100,50,255});
    }

    if (enemies[idx].hp <= 0) {
        if (enemies[idx].type == ENEMY_GHOST) UnlockAchievement(12); // Ghost Buster
        enemies[idx].active = 0;
        kills++;
        AddScore(50);
        PlayCoinSound();
        DropEnemyLoot(idx);
        // Death particles
        for (int i = 0; i < 12; i++) {
            SpawnParticle(ex, ey, (rand()%100-50)*0.15f, (rand()%100-50)*0.15f, 0.6f, 4, (Color){200,50,50,255});
        }
    }
}

void EnemyAttackPlayer(int idx)
{
    if (idx < 0 || idx >= enemyCount || !enemies[idx].active) return;
    if (enemies[idx].attackCooldown > 0) return;
    enemies[idx].attackCooldown = 1.0f;

    // Shield absorbs one hit (consistent everywhere)
    if (shieldActive) {
        shieldActive = 0;
        PlayItemSound();
        return;
    }

    int dmg = enemies[idx].attack - playerDefense;
    if (dmg < 1) dmg = 1;
    Hp -= dmg;
    tookDamageThisLevel = 1;
    PlayHurtSound();
    TriggerFlash((Color){200,30,30,255}, 0.15f);
    TriggerShake(5.0f, 0.15f);
    gameOverReason = 2;

    // Grid knockback (top-down only; FP position is float-driven and would override it)
    if (!fpMode) {
        int kx = 0, ky = 0;
        if (X > enemies[idx].x) kx = 1;
        else if (X < enemies[idx].x) kx = -1;
        if (Y > enemies[idx].y) ky = 1;
        else if (Y < enemies[idx].y) ky = -1;
        if (kx != 0 && ky != 0 && CanMoveTo(X + kx, Y + ky) == 1) { X += kx; Y += ky; }
        else if (kx != 0 && CanMoveTo(X + kx, Y) == 1) X += kx;
        else if (ky != 0 && CanMoveTo(X, Y + ky) == 1) Y += ky;
    }

    if (Hp <= 0) { gameOverReason = 2; gameState = STATE_GAMEOVER; PlayLoseSound(); }
}

void DropEnemyLoot(int idx)
{
    (void)idx;
    // 50% drop coin, 20% drop gem, 10% drop item
    int r = rand() % 100;
    if (r < 50) {
        // Add score directly (coin)
        AddScore(10);
    } else if (r < 70) {
        AddScore(50);
    } else if (r < 80) {
        inventory[rand() % 4]++;
    }
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Maze Adventure - Dungeon Edition");
    SetTargetFPS(60);

    InitGame();
    gameState = STATE_MENU;

    while (!WindowShouldClose()) {
        HandleInput();

        // Per-state ambient updates (kept out of the draw functions)
        if (gameState == STATE_MENU) {
            UpdateMenuAmbient();
            UpdateParticles();
        }

        // Game logic updates (only when playing)
        if (gameState == STATE_MAZE) {
            float dt = GetFrameTime();
            UpdateEnemies();
            UpdateTraps();
            UpdateFog();
            UpdateParticles();
            UpdateCollectibles(dt);
            UpdateItems(dt);
            SpawnAmbientParticles();
            UpdateEffects();
            UpdateWalkBob();
            UpdateAnimation(dt);
            UpdateFPView(dt);

            // Stamina regen
            if (!isSprinting && stamina < maxStamina) stamina += 20.0f * dt;
            if (stamina > maxStamina) stamina = maxStamina;

            // Time limit
            if (timedMode) {
                timeRemaining -= dt;
                if (timeRemaining <= 0) {
                    timeRemaining = 0;
                    gameOverReason = 1;
                    gameState = STATE_GAMEOVER;
                    PlayLoseSound();
                }
            }

            // Check win condition -- single authority for victory
            if (map_change[X][Y] == CELL_EXIT && is_key) {
                AddScore(200 + Hp * 2);
                CheckAchievements();
                PlayWinSound();
                AddHighScore(score, step, currentLevel, "Player");
                gameState = STATE_WIN;
            }
            // Enemy contact is handled by UpdateEnemies -> EnemyAttackPlayer:
            // one damage path with attack cooldown, shield and safe knockback.
        }

        // Modal file dialogs run at a clean frame boundary, not mid-frame
        if (pendingAction != PENDING_NONE) {
            if (pendingAction == PENDING_SAVE_MAP) {
                Savefile();
            } else if (pendingAction == PENDING_OPEN_MAP && gameState == STATE_MAZE) {
                Openfile();
                LoadMapAndReset();
            }
            pendingAction = PENDING_NONE;
        }

        BeginDrawing();

        switch (gameState) {
            case STATE_MENU:        DrawInterface(); break;
            case STATE_CHARSELECT:  DrawCharSelect(); break;
            case STATE_DIFFICULTY:  DrawDifficultySelect(); break;
            case STATE_MAZE:        DrawGame(); break;
            case STATE_PAUSE:       DrawGame(); DrawPauseMenu(); break;
            case STATE_GAMEOVER:    DrawGameOver(); break;
            case STATE_WIN:         DrawWin(); break;
            case STATE_HELP:        DrawHelp(); break;
            case STATE_ABOUT:       DrawAbout(); break;
            case STATE_LEADERBOARD: DrawLeaderboard(); break;
            case STATE_ACHIEVEMENTS: DrawAchievements(); break;
            default: ClearBackground(RAYWHITE); break;
        }

        // Transient save/load feedback (drawn over any state)
        if (hudMsgTime > 0) {
            hudMsgTime -= GetFrameTime();
            int msgW = MeasureText(hudMsg, 20);
            DrawText(hudMsg, (screenWidth - msgW) / 2, screenHeight - 100, 20, (Color){255, 255, 180, 230});
        }

        EndDrawing();
    }

    CloseAudioSys();
    CloseWindow();
    return 0;
}
