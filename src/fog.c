#include "inc.h"
#include "fog.h"

int fog[100][100];

void InitFog(void)
{
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++)
            fog[i][j] = 0;
}

void RevealArea(int cx, int cy, int radius)
{
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int x = cx + dx;
            int y = cy + dy;
            if (x >= 0 && x < 100 && y >= 0 && y < 100) {
                if (dx * dx + dy * dy <= radius * radius) {
                    fog[x][y] = 1;
                }
            }
        }
    }
}

void UpdateFog(void)
{
    int vr = viewRadius;
    if (torchBoostTime > 0) vr += 2;
    if (selectedCharacter == CHAR_MAGE) vr += 1;
    RevealArea(X, Y, vr);
    // Reveal around items and enemies partially
    for (int i = 0; i < itemCount; i++) {
        if (items[i].active) RevealArea(items[i].x, items[i].y, 1);
    }
}

int IsExplored(int x, int y)
{
    if (x < 0 || x >= 100 || y < 0 || y >= 100) return 0;
    return fog[x][y];
}

int IsVisible(int x, int y)
{
    int vr = viewRadius;
    if (torchBoostTime > 0) vr += 2;
    if (selectedCharacter == CHAR_MAGE) vr += 1;
    int dx = x - X;
    int dy = y - Y;
    return (dx * dx + dy * dy <= vr * vr);
}

void DrawMinimap(int x, int y, int w, int h)
{
    // Background
    DrawRectangle(x - 2, y - 2, w + 4, h + 4, (Color){20, 20, 30, 220});
    DrawRectangleLines(x - 2, y - 2, w + 4, h + 4, (Color){100, 100, 120, 200});

    float cellW = (float)w / Col;
    float cellH = (float)h / Row;

    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            if (!IsExplored(i, j)) continue;
            float px = x + (j - 1) * cellW;
            float py = y + (i - 1) * cellH;
            int v = map_change[i][j];
            Color c;
            if (v == 3 || v == -1) c = (Color){80, 80, 90, 255};
            else if (v == 1) c = (Color){0, 200, 200, 200};
            else if (v == 2) c = GOLD;
            else if (v == 4) c = GREEN;
            else if (v == 5) c = RED;
            else c = (Color){180, 180, 170, 255};
            DrawRectangle((int)px, (int)py, (int)(cellW + 1), (int)(cellH + 1), c);
        }
    }

    // Player dot
    float ppx = x + (Y - 0.5f) * cellW;
    float ppy = y + (X - 0.5f) * cellH;
    DrawCircle((int)ppx, (int)ppy, (int)(cellW > cellH ? cellH : cellW) * 0.6f, (Color){100, 220, 255, 255});

    // Enemies
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].active) continue;
        if (!IsVisible(enemies[i].x, enemies[i].y)) continue;
        float ex = x + (enemies[i].y - 0.5f) * cellW;
        float ey = y + (enemies[i].x - 0.5f) * cellH;
        DrawCircle((int)ex, (int)ey, (int)(cellW > cellH ? cellH : cellW) * 0.5f, RED);
    }
}
