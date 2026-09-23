#include "inc.h"
#include "item.h"

Item items[MAX_ITEMS];
int itemCount = 0;

void InitItems(void)
{
    for (int i = 0; i < MAX_ITEMS; i++) items[i].active = 0;
    itemCount = 0;
}

void SpawnItem(int x, int y, int type)
{
    if (itemCount >= MAX_ITEMS) return;
    items[itemCount].x = x;
    items[itemCount].y = y;
    items[itemCount].type = type;
    items[itemCount].active = 1;
    items[itemCount].animTime = (float)(rand() % 100) / 10.0f;
    itemCount++;
}

void CheckItemPickup(void)
{
    for (int i = 0; i < itemCount; i++) {
        if (items[i].active && items[i].x == X && items[i].y == Y) {
            inventory[items[i].type]++;
            items[i].active = 0;
            PlayItemSound();
            AddScore(20);
        }
    }
}

void UseItem(int type)
{
    if (inventory[type] <= 0) return;
    switch (type) {
        case ITEM_BOMB:
            ExplodeBomb(X, Y);
            inventory[type]--;
            bombsUsedTotal++;
            if (bombsUsedTotal >= 5) UnlockAchievement(5); // Bomb Master
            PlayBombSound();
            TriggerShake(8.0f, 0.3f);
            break;
        case ITEM_SPEED:
            speedBoostTime = 10.0f;
            inventory[type]--;
            PlayItemSound();
            break;
        case ITEM_SHIELD:
            shieldActive = 1;
            inventory[type]--;
            PlayItemSound();
            break;
        case ITEM_TORCH:
            torchBoostTime = 15.0f;
            inventory[type]--;
            PlayItemSound();
            break;
    }
}

static void DrawBomb(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2 + sinf(t * 2.5f) * cs * 0.06f;
    DrawCircle((int)cx, (int)cy, cs * 0.22f, (Color){30, 30, 35, 255});
    DrawCircle((int)(cx - cs*0.07), (int)(cy - cs*0.07), cs * 0.07f, (Color){70, 70, 80, 255});
    // Fuse
    DrawLine((int)cx, (int)(cy - cs*0.2), (int)(cx + cs*0.1), (int)(cy - cs*0.3), (Color){139, 90, 43, 255});
    // Spark
    float spark = sinf(t * 15.0f) * 0.5f + 0.5f;
    DrawCircle((int)(cx + cs*0.1), (int)(cy - cs*0.3), cs * 0.04f * spark, (Color){255, 200, 50, 255});
}

static void DrawSpeedBoots(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2 + sinf(t * 2.5f) * cs * 0.06f;
    DrawRectangle((int)(cx - cs*0.2), (int)(cy - cs*0.1), (int)(cs*0.4), (int)(cs*0.2), (Color){80, 150, 255, 255});
    DrawRectangle((int)(cx - cs*0.2), (int)(cy + cs*0.05), (int)(cs*0.45), (int)(cs*0.08), (Color){60, 100, 200, 255});
    // Wing
    DrawTriangle((Vector2){cx - cs*0.2f, cy}, (Vector2){cx - cs*0.35f, cy - cs*0.15f}, (Vector2){cx - cs*0.2f, cy - cs*0.05f}, (Color){200, 220, 255, 255});
}

static void DrawShield(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2 + sinf(t * 2.5f) * cs * 0.06f;
    // Shield shape
    DrawTriangle((Vector2){cx, cy - cs*0.25f}, (Vector2){cx - cs*0.2f, cy - cs*0.1f}, (Vector2){cx - cs*0.2f, cy + cs*0.1f}, (Color){200, 170, 50, 255});
    DrawTriangle((Vector2){cx, cy - cs*0.25f}, (Vector2){cx + cs*0.2f, cy - cs*0.1f}, (Vector2){cx + cs*0.2f, cy + cs*0.1f}, (Color){180, 150, 40, 255});
    DrawTriangle((Vector2){cx - cs*0.2f, cy + cs*0.1f}, (Vector2){cx + cs*0.2f, cy + cs*0.1f}, (Vector2){cx, cy + cs*0.25f}, (Color){160, 130, 30, 255});
    DrawCircle((int)cx, (int)cy, cs * 0.08f, (Color){255, 220, 100, 255});
}

static void DrawTorch(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2 + sinf(t * 2.5f) * cs * 0.06f;
    // Handle
    DrawRectangle((int)(cx - cs*0.04), (int)(cy - cs*0.05), (int)(cs*0.08), (int)(cs*0.25), (Color){100, 60, 30, 255});
    // Flame
    float flicker = sinf(t * 12.0f) * 0.15f + 0.85f;
    DrawCircle((int)cx, (int)(cy - cs*0.15), cs * 0.12f * flicker, (Color){255, 150, 30, 255});
    DrawCircle((int)cx, (int)(cy - cs*0.12), cs * 0.07f * flicker, (Color){255, 220, 80, 255});
    // Glow
    DrawCircle((int)cx, (int)(cy - cs*0.1), cs * 0.25f, (Color){255, 150, 30, 40});
}

void UpdateItems(float dt)
{
    for (int i = 0; i < itemCount; i++) {
        if (items[i].active) items[i].animTime += dt;
    }
}

void DrawItems(void)
{
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].active) continue;
        if (!IsExplored(items[i].x, items[i].y)) continue;
        double px = gridOffsetX + (items[i].y - 1) * cellSize;
        double py = gridOffsetY + (items[i].x - 1) * cellSize;
        float cs = (float)cellSize;
        switch (items[i].type) {
            case ITEM_BOMB:   DrawBomb((int)px, (int)py, cs, items[i].animTime); break;
            case ITEM_SPEED:  DrawSpeedBoots((int)px, (int)py, cs, items[i].animTime); break;
            case ITEM_SHIELD: DrawShield((int)px, (int)py, cs, items[i].animTime); break;
            case ITEM_TORCH:  DrawTorch((int)px, (int)py, cs, items[i].animTime); break;
        }
    }
}
