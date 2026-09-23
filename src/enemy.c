#include "inc.h"
#include "enemy.h"

Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;
static float moveTickFraction = 0;

void InitEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }
    enemyCount = 0;
    moveTickFraction = 0;
}

void SpawnEnemy(int x, int y, int type)
{
    if (enemyCount >= MAX_ENEMIES) return;
    Enemy *e = &enemies[enemyCount];
    e->x = x;
    e->y = y;
    e->type = type;
    e->dir = rand() % 4;
    e->active = 1;
    e->animTime = 0;
    switch (type) {
        case ENEMY_SLIME:    e->speed = 40; break;
        case ENEMY_SKELETON: e->speed = 25; break;
        case ENEMY_GHOST:    e->speed = 30; break;
    }
    e->moveCounter = 0;
    e->isHurt = 0;
    e->hurtTime = 0;
    e->attackCooldown = 0;
    switch (type) {
        case ENEMY_SLIME:    e->hp = 20; e->maxHp = 20; e->attack = 5; break;
        case ENEMY_SKELETON: e->hp = 35; e->maxHp = 35; e->attack = 10; break;
        case ENEMY_GHOST:    e->hp = 25; e->maxHp = 25; e->attack = 8; break;
    }
    enemyCount++;
}

static int EnemyCanMove(int x, int y, int type)
{
    if (x < 1 || x > Row || y < 1 || y > Col) return 0;
    if (map_change[x][y] >= CELL_DOOR_RED && map_change[x][y] <= CELL_DOOR_GREEN) return 0; // doors block all
    if (type == ENEMY_GHOST) return 1; // ghost passes ordinary walls only
    if (map_change[x][y] == CELL_WALL) return 0;
    return 1;
}

void UpdateEnemies(float dt)
{
    // Keep the existing speed values (ticks at 60 Hz) while using elapsed time.
    moveTickFraction += dt * 60.0f;
    int elapsedTicks = (int)moveTickFraction;
    moveTickFraction -= elapsedTicks;
    for (int i = 0; i < enemyCount; i++) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        e->animTime += dt;
        if (e->attackCooldown > 0) e->attackCooldown -= dt;
        if (e->isHurt) {
            e->hurtTime -= dt;
            if (e->hurtTime <= 0) e->isHurt = 0;
        }
        // Attack player if adjacent
        if (abs(e->x - X) + abs(e->y - Y) <= 1) {
            EnemyAttackPlayer(i);
        }
        if (e->speed <= 0) continue; // malformed save data must not divide by zero
        if (e->moveCounter < 0 || e->moveCounter >= e->speed)
            e->moveCounter = 0;
        e->moveCounter += elapsedTicks;
        while (e->moveCounter >= e->speed) {
            e->moveCounter -= e->speed;

            int dx = 0, dy = 0;
            if (e->type == ENEMY_GHOST) {
                // Chase player along the more distant axis (no jitter)
                int adx = abs(X - e->x), ady = abs(Y - e->y);
                if (adx >= ady) dx = (X > e->x) ? 1 : (X < e->x ? -1 : 0);
                else dy = (Y > e->y) ? 1 : (Y < e->y ? -1 : 0);
            } else if (e->type == ENEMY_SKELETON) {
                // Patrol: keep direction, try several turns when blocked
                dx = direction[e->dir][0];
                dy = direction[e->dir][1];
                if (!EnemyCanMove(e->x + dx, e->y + dy, e->type)) {
                    for (int t = 0; t < 4; t++) {
                        e->dir = rand() % 4;
                        dx = direction[e->dir][0];
                        dy = direction[e->dir][1];
                        if (EnemyCanMove(e->x + dx, e->y + dy, e->type)) break;
                    }
                }
            } else {
                // Slime: random
                e->dir = rand() % 4;
                dx = direction[e->dir][0];
                dy = direction[e->dir][1];
            }

            if (EnemyCanMove(e->x + dx, e->y + dy, e->type)) {
                e->x += dx;
                e->y += dy;
            }
        }
    }
}

static void DrawSlime(int px, int py, float cs, float t)
{
    float bob = sinf(t * 3.0f) * cs * 0.05f;
    float cx = px + cs / 2;
    float cy = py + cs / 2 + bob;
    DrawCircle((int)cx, (int)(cy + cs*0.1), cs * 0.32f, (Color){40, 140, 50, 255});
    DrawCircle((int)cx, (int)cy, cs * 0.3f, (Color){60, 180, 70, 255});
    DrawCircle((int)(cx - cs*0.1), (int)(cy - cs*0.05), cs * 0.12f, (Color){100, 220, 110, 255});
    // Eyes
    DrawCircle((int)(cx - cs*0.1), (int)cy, cs * 0.05f, WHITE);
    DrawCircle((int)(cx + cs*0.1), (int)cy, cs * 0.05f, WHITE);
    DrawCircle((int)(cx - cs*0.1), (int)cy, cs * 0.025f, BLACK);
    DrawCircle((int)(cx + cs*0.1), (int)cy, cs * 0.025f, BLACK);
}

static void DrawSkeleton(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2;
    // Body
    DrawRectangle((int)(cx - cs*0.15), (int)(cy - cs*0.1), (int)(cs*0.3), (int)(cs*0.35), (Color){220, 220, 210, 255});
    // Head
    DrawCircle((int)cx, (int)(cy - cs*0.2), cs * 0.18f, (Color){230, 230, 220, 255});
    // Eyes
    DrawCircle((int)(cx - cs*0.07), (int)(cy - cs*0.22), cs * 0.04f, BLACK);
    DrawCircle((int)(cx + cs*0.07), (int)(cy - cs*0.22), cs * 0.04f, BLACK);
    // Sword
    float swing = sinf(t * 4.0f) * cs * 0.1f;
    DrawLine((int)(cx + cs*0.15), (int)(cy - cs*0.15), (int)(cx + cs*0.3 + swing), (int)(cy + cs*0.1), (Color){180, 180, 190, 255});
}

static void DrawGhost(int px, int py, float cs, float t)
{
    float cx = px + cs / 2;
    float cy = py + cs / 2 + sinf(t * 2.0f) * cs * 0.08f;
    float alpha = 180 + sinf(t * 3.0f) * 40;
    // Body (wavy bottom)
    DrawCircle((int)cx, (int)(cy - cs*0.05), cs * 0.28f, (Color){180, 180, 220, (unsigned char)alpha});
    for (int i = 0; i < 4; i++) {
        float wx = cx - cs*0.2 + i * cs*0.13f;
        float wy = cy + cs*0.15 + sinf(t * 5.0f + i) * cs*0.05f;
        DrawCircle((int)wx, (int)wy, cs * 0.08f, (Color){180, 180, 220, (unsigned char)alpha});
    }
    // Eyes
    DrawCircle((int)(cx - cs*0.1), (int)(cy - cs*0.08), cs * 0.05f, (Color){40, 40, 80, 255});
    DrawCircle((int)(cx + cs*0.1), (int)(cy - cs*0.08), cs * 0.05f, (Color){40, 40, 80, 255});
}

void DrawEnemies(void)
{
    for (int i = 0; i < enemyCount; i++) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        if (!IsExplored(e->x, e->y)) continue;
        double px = gridOffsetX + (e->y - 1) * cellSize;
        double py = gridOffsetY + (e->x - 1) * cellSize;
        float cs = (float)cellSize;

        // Hurt flash overlay
        if (e->isHurt) {
            BeginBlendMode(BLEND_ADDITIVE);
            DrawRectangle((int)px, (int)py, (int)cs, (int)cs, (Color){255,100,100,120});
            EndBlendMode();
        }

        switch (e->type) {
            case ENEMY_SLIME:    DrawSlime((int)px, (int)py, cs, e->animTime); break;
            case ENEMY_SKELETON: DrawSkeleton((int)px, (int)py, cs, e->animTime); break;
            case ENEMY_GHOST:    DrawGhost((int)px, (int)py, cs, e->animTime); break;
        }

        // HP bar
        if (e->hp < e->maxHp) {
            float bw = cs * 0.7f;
            float bx = px + cs * 0.15f;
            float by = py - 6;
            DrawRectangle((int)bx, (int)by, (int)bw, 4, (Color){60,20,20,200});
            DrawRectangle((int)bx, (int)by, (int)(bw * e->hp / e->maxHp), 4, (Color){220,50,50,255});
        }
    }
}
