#include "event.h"
#include "file.h"
#include "inc.h"
#include "particle.h"

static int is_mousedown = 0;
static int last_gx = -1, last_gy = -1;
static int iceSlideDir = -1;
static float iceSlideTimer = 0;
static float moveCooldown = 0;

void MovePlayer(int dx, int dy)
{
    if (gameState != STATE_MAZE) return;

    int nx = X + dx, ny = Y + dy;

    // Check movement
    int canMove = CanMoveTo(nx, ny);

    if (canMove == 0) {
        // Hit wall - small shake
        TriggerShake(1.0f, 0.05f);
        return;
    }

    if (canMove == 2) {
        // Box - try push
        if (!TryPushBox(nx, ny, dx, dy)) return;
    }

    // Door check
    if (map_change[nx][ny] >= 10 && map_change[nx][ny] <= 12) {
        int doorIdx = map_change[nx][ny] - 10;
        if (keysCollected[doorIdx]) {
            map_change[nx][ny] = 0;
            PlayDoorSound();
        } else return;
    }

    // Reached end without key
    if (map_change[nx][ny] == 5 && is_key == 0) return;

    // Start timer
    if (!is_start) { start = (int)clock(); is_start = 1; }

    // Valid move
    X = nx;
    Y = ny;

    // HP cost
    int hpCost = 1;
    if (speedBoostTime > 0) hpCost = 0;
    if (selectedCharacter == CHAR_ROGUE && rogueFreeSteps > 0) { hpCost = 0; rogueFreeSteps--; }
    Hp -= hpCost;

    step++;
    is_showtip = 0;

    // Effects
    double px = gridOffsetX + (Y - 0.5) * cellSize;
    double py = gridOffsetY + (X - 0.5) * cellSize + cellSize * 0.2;
    SpawnDust((float)px, (float)py);
    AddWalkBob();
    PlayMoveSound();

    // Shared cell interactions (key/collectibles/items/traps/portals/colored keys)
    OnEnterCell();

    // Ice slide
    if (iceMap[X][Y]) {
        iceSlideDir = dx * 2 + (dy > 0 ? 1 : (dy < 0 ? 0 : -1));
        iceSlideTimer = 0.08f;
        iceSlideCount++;
        if (iceSlideCount >= 10) UnlockAchievement(13); // Ice Skater
    }

    if (Hp <= 0) { gameOverReason = 0; gameState = STATE_GAMEOVER; PlayLoseSound(); }
}

static void HandleKeyboard(void)
{
    // Pause toggle (both top-down and FP; release/recapture the mouse in FP)
    if (gameState == STATE_MAZE || gameState == STATE_PAUSE) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
            if (gameState == STATE_MAZE) {
                gameState = STATE_PAUSE;
                if (fpMode) EnableCursor();
            } else {
                gameState = STATE_MAZE;
                if (fpMode) DisableCursor();
            }
            return;
        }
    }

    if (gameState != STATE_MAZE) return;

    // Sprint stamina (consumed in both modes)
    isSprinting = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (isSprinting && stamina < 5) isSprinting = 0;
    if (isSprinting) stamina -= 30.0f * GetFrameTime();

    // Ice sliding (grid movement only)
    if (!fpMode && iceSlideTimer > 0) {
        iceSlideTimer -= GetFrameTime();
        if (iceSlideTimer <= 0 && iceSlideDir >= 0) {
            int sdx = 0, sdy = 0;
            if (iceSlideDir == 0) sdx = -1;
            else if (iceSlideDir == 1) sdx = 1;
            else if (iceSlideDir == 2) sdy = -1;
            else if (iceSlideDir == 3) sdy = 1;
            if (sdx != 0 || sdy != 0) {
                int nx = X + sdx, ny = Y + sdy;
                if (CanMoveTo(nx, ny) > 0 && iceMap[nx][ny]) {
                    MovePlayer(sdx, sdy);
                } else {
                    iceSlideDir = -1;
                }
            }
        }
    }

    // Top-down grid movement (FP smooth movement lives in fpview.c)
    if (!fpMode) {
        if (moveCooldown > 0) moveCooldown -= GetFrameTime();
        float moveDelay = (isSprinting || speedBoostTime > 0) ? 0.07f : 0.13f;

        int dx = 0, dy = 0;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) dx = -1;
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) dx = 1;
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) dy = -1;
        else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) dy = 1;

        if ((dx != 0 || dy != 0) && moveCooldown <= 0 && iceSlideTimer <= 0) {
            MovePlayer(dx, dy);
            moveCooldown = moveDelay;
            if (dx == -1) playerDir = 0;
            else if (dx == 1) playerDir = 1;
            else if (dy == -1) playerDir = 2;
            else if (dy == 1) playerDir = 3;
        }
    }

    // Attack (space) -- available in both modes
    if (IsKeyPressed(KEY_SPACE)) PlayerAttack();

    // Toggle first person view (V) -- must work while in FP
    if (IsKeyPressed(KEY_V)) ToggleFPMode();

    // Item shortcuts (both modes)
    if (IsKeyPressed(KEY_ONE)) UseItem(ITEM_BOMB);
    if (IsKeyPressed(KEY_TWO)) UseItem(ITEM_SPEED);
    if (IsKeyPressed(KEY_THREE)) UseItem(ITEM_SHIELD);
    if (IsKeyPressed(KEY_FOUR)) UseItem(ITEM_TORCH);

    // Function keys (F-series to avoid WASD conflicts; work in both modes)
    if (IsKeyPressed(KEY_F2)) { mapSeed = 0; StartLevel(currentLevel); }
    if (IsKeyPressed(KEY_F3) && !fpMode) { is_edit = !is_edit; is_showsolution = 0; is_showtip = 0; }
    if (IsKeyPressed(KEY_F5)) Savefile();
    if (IsKeyPressed(KEY_F9)) {
        Openfile();
        InitMapEntities(); InitEnemies(); InitItems();
        CreatWalllist();
        InitFog();
        X = Y = 2; step = 0; is_key = 0;
        is_start = 0; OptimalSolution();
        if (shortstep <= 0) shortstep = 50;
        Hp = shortstep * 2;
        UpdateFog();
    }

    // Edit mode (top-down only)
    if (!fpMode && is_edit) {
        Vector2 mouse = GetMousePosition();
        WallT ptr = SelectNearestNode(Wall, mouse.x, mouse.y);
        if (ptr != NULL) {
            if (IsKeyPressed(KEY_Q)) map_change[ptr->x0][ptr->y0] = 3;
            if (IsKeyPressed(KEY_W)) map_change[ptr->x0][ptr->y0] = 0;
        }
    }
}

static void HandleMouseEdit(void)
{
    if (!is_edit || gameState != STATE_MAZE) return;
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        is_mousedown = 1;
        WallT ptr = SelectNearestNode(Wall, mouse.x, mouse.y);
        if (ptr != NULL) { last_gx = ptr->x0; last_gy = ptr->y0; }
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && is_mousedown) {
        WallT ptr = SelectNearestNode(Wall, mouse.x, mouse.y);
        if (ptr != NULL && (ptr->x0 != last_gx || ptr->y0 != last_gy)) {
            int tmp = map_change[last_gx][last_gy];
            map_change[last_gx][last_gy] = map_change[ptr->x0][ptr->y0];
            map_change[ptr->x0][ptr->y0] = tmp;
            if (last_gx == X && last_gy == Y) { X = ptr->x0; Y = ptr->y0; }
            last_gx = ptr->x0; last_gy = ptr->y0;
        }
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        is_mousedown = 0; last_gx = -1; last_gy = -1;
    }
}

void HandleInput(void)
{
    HandleKeyboard();
    HandleMouseEdit();
}
