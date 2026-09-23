#include "event.h"
#include "file.h"
#include "inc.h"
#include "particle.h"

static int is_mousedown = 0;
static int last_gx = -1, last_gy = -1;
// Ice slide: keep the raw direction (the old packed-dir encoding decoded
// left/up/right onto the wrong axes)
static int iceSliding = 0;
static int iceSlideDx = 0, iceSlideDy = 0;
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

    if (!OpenDoorIfUnlocked(nx, ny)) return;

    // Reached end without key
    if (map_change[nx][ny] == CELL_EXIT && is_key == 0) return;

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
        iceSlideDx = dx;
        iceSlideDy = dy;
        iceSliding = 1;
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

    // Music toggle (works in every state, like a media key)
    if (IsKeyPressed(KEY_M)) ToggleBGM();

    if (gameState != STATE_MAZE) return;

    // Sprint stamina (consumed in both modes)
    isSprinting = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (isSprinting && stamina < 5) isSprinting = 0;
    if (isSprinting) stamina -= 30.0f * GetFrameTime();

    // Ice sliding (grid movement only)
    if (!fpMode && iceSliding) {
        iceSlideTimer -= GetFrameTime();
        if (iceSlideTimer <= 0) {
            int nx = X + iceSlideDx, ny = Y + iceSlideDy;
            if (CanMoveTo(nx, ny) > 0 && iceMap[nx][ny]) {
                MovePlayer(iceSlideDx, iceSlideDy);
            } else {
                iceSliding = 0;
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

        if ((dx != 0 || dy != 0) && moveCooldown <= 0 && !iceSliding) {
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
    if (IsKeyPressed(KEY_F2)) { mapSeed = 0; ResetRunState(); StartLevel(currentLevel); }
    if (IsKeyPressed(KEY_F3) && !fpMode) { is_edit = !is_edit; is_showsolution = 0; is_showtip = 0; }
    if (IsKeyPressed(KEY_F5)) pendingAction = PENDING_SAVE_MAP;
    if (IsKeyPressed(KEY_F9)) pendingAction = PENDING_OPEN_MAP;

    // Edit mode (top-down only)
    if (!fpMode && is_edit) {
        Vector2 mouse = GetMousePosition();
        int gx, gy;
        if (ScreenToCell(mouse.x, mouse.y, &gx, &gy)) {
            if (IsKeyPressed(KEY_Q)) { map_change[gx][gy] = CELL_WALL; MarkMazeDirty(); }
            if (IsKeyPressed(KEY_W)) { map_change[gx][gy] = CELL_ROAD; MarkMazeDirty(); }
        }
    }
}

static void HandleMouseEdit(void)
{
    if (!is_edit || gameState != STATE_MAZE) return;
    Vector2 mouse = GetMousePosition();
    int gx, gy;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        is_mousedown = 1;
        if (ScreenToCell(mouse.x, mouse.y, &gx, &gy)) { last_gx = gx; last_gy = gy; }
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && is_mousedown &&
        ScreenToCell(mouse.x, mouse.y, &gx, &gy)) {
        if (gx != last_gx || gy != last_gy) {
            int tmp = map_change[last_gx][last_gy];
            map_change[last_gx][last_gy] = map_change[gx][gy];
            map_change[gx][gy] = tmp;
            if (last_gx == X && last_gy == Y) { X = gx; Y = gy; }
            last_gx = gx; last_gy = gy;
            MarkMazeDirty();
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
