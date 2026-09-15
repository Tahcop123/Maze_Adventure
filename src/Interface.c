#include "inc.h"
#include "interface.h"
#include "file.h"
#include <string.h>
#include <stdio.h>

int is_edit = 0;
int is_file = 0;
int is_e = 0;
int is_help = 0;

static bool PointInRect(int px, int py, int rx, int ry, int rw, int rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

int Button(int x, int y, int w, int h, const char *label, Color bg, Color textColor)
{
    Vector2 mouse = GetMousePosition();
    bool hover = PointInRect((int)mouse.x, (int)mouse.y, x, y, w, h);
    bool pressed = hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Color drawBg = hover ? (Color){(unsigned char)(bg.r+25<255?bg.r+25:255), (unsigned char)(bg.g+25<255?bg.g+25:255), (unsigned char)(bg.b+25<255?bg.b+25:255), bg.a} : bg;
    DrawRectangleRounded((Rectangle){(float)x, (float)y, (float)w, (float)h}, 0.15, 4, drawBg);
    DrawRectangleRoundedLines((Rectangle){(float)x, (float)y, (float)w, (float)h}, 0.15, 4, DARKGRAY);
    int textW = MeasureText(label, 16);
    DrawText(label, x + (w - textW) / 2, y + (h - 16) / 2, 16, textColor);
    return pressed ? 1 : 0;
}

int MenuButton(int x, int y, int w, int h, const char *label)
{
    return Button(x, y, w, h, label, (Color){70, 130, 180, 255}, WHITE);
}

void DrawHp(void)
{
    int hx = 15, hy = 10;
    // Heart
    DrawCircle(hx + 6, hy + 8, 6, RED);
    DrawCircle(hx + 15, hy + 8, 6, RED);
    DrawTriangle((Vector2){(float)(hx-1), (float)(hy+10)}, (Vector2){(float)(hx+22), (float)(hy+10)}, (Vector2){(float)(hx+10), (float)(hy+22)}, RED);
    char hpStr[32];
    sprintf(hpStr, "HP:%d", Hp);
    DrawText(hpStr, hx + 30, hy + 4, 16, MAROON);

    // Stamina bar
    DrawRectangle(hx + 100, hy + 4, 80, 14, (Color){40, 40, 50, 200});
    DrawRectangle(hx + 100, hy + 4, (int)(80 * stamina / maxStamina), 14, (Color){80, 200, 120, 255});
    DrawText("STA", hx + 102, hy + 6, 10, WHITE);

    // Score
    char scoreStr[32];
    sprintf(scoreStr, "Score:%d", score);
    DrawText(scoreStr, hx + 195, hy + 4, 16, (Color){255, 215, 0, 255});

    // Level
    char lvlStr[16];
    sprintf(lvlStr, "Lvl:%d", currentLevel + 1);
    DrawText(lvlStr, hx + 300, hy + 4, 16, (Color){150, 200, 255, 255});

    // Steps
    char stepStr[16];
    sprintf(stepStr, "Steps:%d", step);
    DrawText(stepStr, hx + 365, hy + 4, 16, DARKGRAY);

    // Key status
    if (is_key) DrawText("Key:YES", hx + 450, hy + 4, 16, (Color){0, 150, 0, 255});
    else DrawText("Key:NO", hx + 450, hy + 4, 16, GRAY);
}

void DrawTopBar(void)
{
    DrawRectangle(0, 0, screenWidth, 42, (Color){240, 240, 245, 255});
    DrawLine(0, 42, screenWidth, 42, LIGHTGRAY);
    int by = 7, bh = 28, rx = screenWidth - 10;

    rx -= 90;
    if (Button(rx, by, 90, bh, "Exit", (Color){200,80,80,255}, WHITE)) {
        gameState = STATE_MENU; is_edit = 0; is_showsolution = 0; is_showtip = 0;
    }
    rx -= 8;
    rx -= 70;
    if (Button(rx, by, 70, bh, "Open", (Color){100,150,220,255}, WHITE)) {
        Openfile();
        InitMapEntities(); InitEnemies(); InitItems();
        CreatWalllist(); InitFog();
        X = Y = 2; step = 0; is_key = 0;
        is_start = 0; OptimalSolution();
        if (shortstep <= 0) shortstep = 50;
        Hp = shortstep * 2;
        UpdateFog();
    }
    rx -= 8;
    rx -= 70;
    if (Button(rx, by, 70, bh, "Save", (Color){100,180,100,255}, WHITE)) Savefile();
    rx -= 8;
    rx -= 90;
    if (Button(rx, by, 90, bh, is_edit ? "Editing..." : "Edit(F3)", is_edit ? (Color){220,100,100,255} : (Color){200,200,200,255}, BLACK)) is_edit = !is_edit;
    rx -= 8;
    rx -= 90;
    if (Button(rx, by, 90, bh, "Tip", is_showtip ? (Color){0,150,150,255} : (Color){200,200,200,255}, BLACK)) {
        is_showtip = !is_showtip;
        if (is_showtip) { is_showsolution = 0; hintsUsed = 1; }
    }
    rx -= 8;
    rx -= 90;
    if (Button(rx, by, 90, bh, "Solution", is_showsolution ? (Color){0,150,150,255} : (Color){200,200,200,255}, BLACK)) {
        is_showsolution = !is_showsolution;
        if (is_showsolution) { is_showtip = 0; hintsUsed = 1; }
    }
    DrawHp();
}

void DrawInterface(void)
{
    // Background gradient
    for (int y = 0; y < screenHeight; y++) {
        float t = (float)y / screenHeight;
        DrawLine(0, y, screenWidth, y, (Color){(unsigned char)(20+t*30), (unsigned char)(15+t*25), (unsigned char)(40+t*40), 255});
    }
    // Torch flicker particles in menu
    if (rand() % 3 == 0) {
        SpawnParticle(rand() % screenWidth, screenHeight * 0.3f + rand() % (int)(screenHeight*0.4f),
                      (float)(rand()%100-50)/50.0f, -1.0f - (float)(rand()%50)/50.0f,
                      1.5f, 3.0f, (Color){255, 150+rand()%80, 50, 200});
    }
    UpdateParticles();
    DrawParticles();

    const char *title = "MAZE ADVENTURE";
    DrawText(title, (screenWidth - MeasureText(title, 52)) / 2, 60, 52, (Color){255, 220, 100, 255});
    DrawText("Dungeon Edition", (screenWidth - MeasureText("Dungeon Edition", 22)) / 2, 125, 22, (Color){200, 180, 140, 255});

    int bw = 240, bh = 48, bx = (screenWidth - bw) / 2, by = 180, gap = 14;
    // NOTE: return after any menu click. Buttons are immediate-mode: a slow
    // callback (e.g. StartLevel) plus mouse movement in the same frame can
    // make a later button also register as pressed and overwrite gameState.
    if (Button(bx, by, bw, bh, "Start Game", (Color){50, 150, 80, 255}, WHITE)) {
        mapSeed = 0; // roll a fresh seed; selected difficulty picks the start level
        StartLevel(difficulty);
        gameState = STATE_MAZE;
        return;
    }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "Select Character", (Color){150, 80, 180, 255}, WHITE)) { gameState = STATE_CHARSELECT; return; }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "Difficulty", (Color){50, 120, 180, 255}, WHITE)) { gameState = STATE_DIFFICULTY; return; }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "Leaderboard", (Color){180, 140, 50, 255}, WHITE)) { gameState = STATE_LEADERBOARD; return; }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "Achievements", (Color){120, 100, 160, 255}, WHITE)) { gameState = STATE_ACHIEVEMENTS; return; }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "Help", (Color){100, 100, 120, 255}, WHITE)) { gameState = STATE_HELP; return; }
    by += bh + gap;
    if (Button(bx, by, bw, bh, "About", (Color){80, 80, 100, 255}, WHITE)) { gameState = STATE_ABOUT; return; }

    DrawText("Arrows/WASD:Move | Shift:Sprint | Space:Attack | V:First-Person | 1-4:Items | P/Esc:Pause | F3:Edit",
             20, screenHeight - 30, 14, (Color){180, 170, 150, 200});
}

void DrawCharSelect(void)
{
    ClearBackground((Color){25, 20, 35, 255});
    const char *title = "SELECT CHARACTER";
    DrawText(title, (screenWidth - MeasureText(title, 36)) / 2, 50, 36, WHITE);

    const char *names[3] = {"Knight", "Mage", "Rogue"};
    const char *desc[3] = {"HP +20", "View +1 tile", "First 10 steps free"};
    Color colors[3] = {{200, 150, 50, 255}, {80, 120, 220, 255}, {50, 180, 100, 255}};
    int cw = 250, ch = 320, gap = 40;
    int startX = (screenWidth - (cw * 3 + gap * 2)) / 2;

    for (int i = 0; i < 3; i++) {
        int cx = startX + i * (cw + gap);
        int cy = 120;
        bool selected = (selectedCharacter == i);
        Color bg = selected ? colors[i] : (Color){50, 50, 60, 255};
        DrawRectangleRounded((Rectangle){cx, cy, cw, ch}, 0.1, 4, bg);
        DrawRectangleRoundedLines((Rectangle){cx, cy, cw, ch}, 0.1, 4, selected ? WHITE : DARKGRAY);

        // Character icon
        float iconX = cx + cw / 2, iconY = cy + 100;
        DrawCircle(iconX, iconY, 50, (Color){30, 30, 40, 255});
        DrawCircle(iconX - 15, iconY - 10, 8, (Color){100, 200, 255, 255});
        DrawCircle(iconX + 15, iconY - 10, 8, (Color){100, 200, 255, 255});

        DrawText(names[i], cx + (cw - MeasureText(names[i], 24)) / 2, cy + 180, 24, WHITE);
        DrawText(desc[i], cx + (cw - MeasureText(desc[i], 16)) / 2, cy + 220, 16, (Color){220, 220, 230, 255});

        if (Button(cx + 40, cy + 260, cw - 80, 40, selected ? "SELECTED" : "Select",
                    selected ? (Color){100, 200, 100, 255} : (Color){100, 100, 120, 255}, WHITE)) {
            selectedCharacter = i;
        }
    }

    if (Button(screenWidth / 2 - 80, screenHeight - 70, 160, 40, "Back", (Color){100, 100, 120, 255}, WHITE))
        gameState = STATE_MENU;
}

void DrawDifficultySelect(void)
{
    ClearBackground((Color){20, 25, 40, 255});
    const char *title = "SELECT DIFFICULTY";
    DrawText(title, (screenWidth - MeasureText(title, 36)) / 2, 60, 36, WHITE);

    const char *names[3] = {"Easy", "Normal", "Hard"};
    const char *desc[3] = {"15x20 | HP x3 | 1 enemy", "20x30 | HP x2 | 3 enemies", "30x40 | HP x1.5 | 5 enemies | 120s"};
    Color colors[3] = {{80, 180, 80, 255}, {200, 180, 50, 255}, {200, 80, 80, 255}};

    for (int i = 0; i < 3; i++) {
        int bx = (screenWidth - 400) / 2;
        int by = 140 + i * 110;
        bool selected = (difficulty == i);
        DrawRectangleRounded((Rectangle){bx, by, 400, 90}, 0.1, 4, selected ? colors[i] : (Color){50, 55, 70, 255});
        DrawText(names[i], bx + 20, by + 15, 24, WHITE);
        DrawText(desc[i], bx + 20, by + 50, 14, (Color){200, 200, 210, 255});
        if (Button(bx + 300, by + 25, 80, 40, selected ? "OK" : "Pick",
                    selected ? (Color){100, 200, 100, 255} : (Color){100, 100, 120, 255}, WHITE)) {
            difficulty = i;
        }
    }

    if (Button(screenWidth / 2 - 80, screenHeight - 70, 160, 40, "Back", (Color){100, 100, 120, 255}, WHITE))
        gameState = STATE_MENU;
}

void DrawLeaderboard(void)
{
    ClearBackground((Color){25, 20, 15, 255});
    const char *title = "LEADERBOARD";
    DrawText(title, (screenWidth - MeasureText(title, 36)) / 2, 40, 36, (Color){255, 215, 0, 255});

    int bx = (screenWidth - 600) / 2;
    DrawText("Rank  Score  Steps  Level  Name", bx, 100, 18, (Color){200, 200, 200, 255});
    DrawLine(bx, 125, bx + 600, 125, (Color){100, 100, 100, 255});

    for (int i = 0; i < MAX_SCORES; i++) {
        if (highScores[i].score <= 0) continue;
        int y = 135 + i * 32;
        char line[128];
        sprintf(line, "%-6d %-7d %-7d %-7d %s", i+1, highScores[i].score, highScores[i].steps, highScores[i].level+1, highScores[i].name);
        Color c = (i == 0) ? GOLD : (i == 1) ? (Color){200,200,200,255} : (i == 2) ? (Color){205,127,50,255} : WHITE;
        DrawText(line, bx, y, 16, c);
    }

    if (Button(screenWidth / 2 - 80, screenHeight - 70, 160, 40, "Back", (Color){100, 100, 120, 255}, WHITE))
        gameState = STATE_MENU;
}

void DrawAchievements(void)
{
    ClearBackground((Color){20, 15, 30, 255});
    const char *title = "ACHIEVEMENTS";
    DrawText(title, (screenWidth - MeasureText(title, 36)) / 2, 40, 36, (Color){180, 140, 255, 255});

    int cols = 2, cw = 500, ch = 50, gap = 10;
    int startX = (screenWidth - (cw * cols + gap)) / 2;

    for (int i = 0; i < MAX_ACHIEVEMENTS; i++) {
        int col = i % cols, row = i / cols;
        int x = startX + col * (cw + gap);
        int y = 100 + row * (ch + gap);
        bool unlocked = IsAchievementUnlocked(i);
        DrawRectangleRounded((Rectangle){x, y, cw, ch}, 0.1, 4, unlocked ? (Color){60, 50, 90, 255} : (Color){40, 40, 50, 200});
        DrawText(unlocked ? "[X]" : "[ ]", x + 10, y + 15, 18, unlocked ? (Color){100, 255, 100, 255} : GRAY);
        DrawText(achievementNames[i], x + 50, y + 15, 14, unlocked ? WHITE : (Color){150, 150, 150, 255});
    }

    if (Button(screenWidth / 2 - 80, screenHeight - 70, 160, 40, "Back", (Color){100, 100, 120, 255}, WHITE))
        gameState = STATE_MENU;
}

void DrawHelp(void)
{
    ClearBackground((Color){15, 20, 30, 255});
    DrawText("HOW TO PLAY", (screenWidth - MeasureText("HOW TO PLAY", 36)) / 2, 30, 36, WHITE);
    const char *lines[] = {
        "Arrows/WASD - Move player",
        "Shift - Sprint (uses stamina)",
        "Space - Attack enemies in front",
        "V - Toggle first-person 3D view",
        "1 - Use Bomb (destroy 3x3 walls)",
        "2 - Use Speed Boots (10s no HP cost)",
        "3 - Use Shield (block next damage)",
        "4 - Use Torch (15s expanded vision)",
        "P / Esc - Pause game",
        "F3 - Toggle map edit mode",
        "F2 - New map | F5 - Save map | F9 - Open map",
        "",
        "Goal: Grab the golden KEY, reach the red FLAG!",
        "Avoid enemies (slimes/skeletons/ghosts) and spike traps.",
        "Collect coins and gems for score. Find hidden rooms for bonuses!",
        "Portals teleport you. Push boxes onto pressure plates to open doors.",
        "Ice tiles make you slide until you hit a wall.",
    };
    int y = 80;
    for (int i = 0; i < 17; i++) {
        DrawText(lines[i], 80, y, 17, (Color){220, 225, 240, 255});
        y += 28;
    }
    if (Button(screenWidth / 2 - 80, screenHeight - 60, 160, 40, "Back", (Color){100,100,120,255}, WHITE)) gameState = STATE_MENU;
}

void DrawAbout(void)
{
    ClearBackground((Color){20, 15, 25, 255});
    DrawText("ABOUT", (screenWidth - MeasureText("ABOUT", 36)) / 2, 50, 36, WHITE);
    const char *lines[] = {
        "Maze Adventure - Dungeon Edition",
        "",
        "Graphics: raylib 5.5 (OpenGL rendering)",
        "Algorithm: Prim maze generation + BFS pathfinding",
        "",
        "Features:",
        "  - 3 difficulty levels with progressive maze sizes",
        "  - 3 playable characters with unique abilities",
        "  - Enemy AI (slimes, skeletons, ghosts)",
        "  - Item system (bombs, speed boots, shield, torch)",
        "  - Collectibles (coins, gems) and scoring",
        "  - Traps (spikes), portals, pushable boxes",
        "  - Pressure plates and multi-key doors",
        "  - Ice sliding mechanics",
        "  - Hidden rooms with bonus rewards",
        "  - War fog of war + minimap",
        "  - Dynamic lighting + particle effects",
        "  - Synthesized audio (SFX + BGM)",
        "  - Save/load, leaderboard, achievements",
        "",
        "Original libgraphics replaced with raylib for modern graphics.",
    };
    int y = 110;
    for (int i = 0; i < 21; i++) {
        DrawText(lines[i], 100, y, 16, (Color){220, 215, 230, 255});
        y += 24;
    }
    if (Button(screenWidth / 2 - 80, screenHeight - 60, 160, 40, "Back", (Color){100,100,120,255}, WHITE)) gameState = STATE_MENU;
}
