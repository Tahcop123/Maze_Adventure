#include "inc.h"
#include "save.h"

ScoreEntry highScores[MAX_SCORES];
int achievements[MAX_ACHIEVEMENTS];

const char *achievementNames[MAX_ACHIEVEMENTS] = {
    "First Steps - Complete level 1",
    "Explorer - Collect all coins in a level",
    "Speedrunner - Finish in under 100 steps",
    "Pacifist - Complete without taking damage",
    "Treasure Hunter - Collect a gem",
    "Bomb Master - Use 5 bombs",
    "Key Master - Collect all keys",
    "Survivor - Finish with >50% HP",
    "Level 2 - Complete level 2",
    "Champion - Complete all 3 levels",
    "No Hints - Complete without using Solution/Tip",
    "Perfectionist - Get 1000+ points",
    "Ghost Buster - Defeat a ghost enemy",
    "Ice Skater - Slide on ice 10 times",
    "Portal Master - Use all portals"
};

static const char *SCORE_FILE = "highscores.dat";
static const char *ACH_FILE = "achievements.dat";
static const char *SAVE_FILE = "savegame.dat";

#define SAVE_MAGIC "MAZS"
#define SAVE_VERSION 1

// Read exactly n bytes; returns 1 on success
static int rd(FILE *f, void *buf, size_t n)
{
    return fread(buf, 1, n, f) == n;
}

// Write exactly n bytes; returns 1 on success
static int wr(FILE *f, const void *buf, size_t n)
{
    return fwrite(buf, 1, n, f) == n;
}

void LoadHighScores(void)
{
    for (int i = 0; i < MAX_SCORES; i++) {
        highScores[i].score = 0;
        highScores[i].steps = 0;
        highScores[i].level = 0;
        strcpy(highScores[i].name, "---");
    }
    FILE *f = fopen(SCORE_FILE, "rb");
    if (f) {
        size_t got = fread(highScores, sizeof(ScoreEntry), MAX_SCORES, f);
        fclose(f);
        for (size_t i = 0; i < got; i++) highScores[i].name[15] = 0;
    }
}

void SaveHighScores(void)
{
    FILE *f = fopen(SCORE_FILE, "wb");
    if (f) {
        fwrite(highScores, sizeof(ScoreEntry), MAX_SCORES, f);
        fclose(f);
    }
}

void AddHighScore(int score, int steps, int level, const char *name)
{
    ScoreEntry entry;
    entry.score = score;
    entry.steps = steps;
    entry.level = level;
    strncpy(entry.name, name, 15);
    entry.name[15] = 0;

    for (int i = 0; i < MAX_SCORES; i++) {
        if (entry.score > highScores[i].score) {
            for (int j = MAX_SCORES - 1; j > i; j--) {
                highScores[j] = highScores[j - 1];
            }
            highScores[i] = entry;
            break;
        }
    }
    SaveHighScores();
}

void LoadAchievements(void)
{
    for (int i = 0; i < MAX_ACHIEVEMENTS; i++) achievements[i] = 0;
    FILE *f = fopen(ACH_FILE, "rb");
    if (f) {
        if (fread(achievements, sizeof(int), MAX_ACHIEVEMENTS, f) != MAX_ACHIEVEMENTS) {
            for (int i = 0; i < MAX_ACHIEVEMENTS; i++) achievements[i] = 0;
        }
        fclose(f);
    }
}

void SaveAchievements(void)
{
    FILE *f = fopen(ACH_FILE, "wb");
    if (f) {
        fwrite(achievements, sizeof(int), MAX_ACHIEVEMENTS, f);
        fclose(f);
    }
}

void UnlockAchievement(int id)
{
    if (id >= 0 && id < MAX_ACHIEVEMENTS && !achievements[id]) {
        achievements[id] = 1;
        SaveAchievements();
    }
}

int IsAchievementUnlocked(int id)
{
    if (id < 0 || id >= MAX_ACHIEVEMENTS) return 0;
    return achievements[id];
}

void CheckAchievements(void)
{
    if (currentLevel >= 0) UnlockAchievement(0);
    if (currentLevel >= 1) UnlockAchievement(8);
    if (currentLevel >= 2) UnlockAchievement(9);
    if (coinsCollected >= totalCoins && totalCoins > 0) UnlockAchievement(1);
    if (step < 100 && step > 0) UnlockAchievement(2);
    if (!tookDamageThisLevel) UnlockAchievement(3);                 // Pacifist
    if (gemsCollected > 0) UnlockAchievement(4);
    if (keysCollected[0] && keysCollected[1] && keysCollected[2])
        UnlockAchievement(6);                                       // Key Master
    if (Hp > shortstep) UnlockAchievement(7);
    if (!hintsUsed) UnlockAchievement(10);                          // No Hints
    if (score >= 1000) UnlockAchievement(11);
}

void SaveGame(void)
{
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;

    int ok = 1;
    char magic[4] = {'M','A','Z','S'};
    int version = SAVE_VERSION;
    ok = ok && wr(f, magic, 4);
    ok = ok && wr(f, &version, sizeof(int));

    // Core scalars
    ok = ok && wr(f, &Row, sizeof(int));
    ok = ok && wr(f, &Col, sizeof(int));
    ok = ok && wr(f, &X, sizeof(int));
    ok = ok && wr(f, &Y, sizeof(int));
    ok = ok && wr(f, &Hp, sizeof(int));
    ok = ok && wr(f, &step, sizeof(int));
    ok = ok && wr(f, &score, sizeof(int));
    ok = ok && wr(f, &currentLevel, sizeof(int));
    ok = ok && wr(f, &selectedCharacter, sizeof(int));
    ok = ok && wr(f, &is_key, sizeof(int));
    ok = ok && wr(f, &coinsCollected, sizeof(int));
    ok = ok && wr(f, &gemsCollected, sizeof(int));
    ok = ok && wr(f, &totalCoins, sizeof(int));
    ok = ok && wr(f, &totalGems, sizeof(int));
    ok = ok && wr(f, &kills, sizeof(int));
    ok = ok && wr(f, &timedMode, sizeof(int));
    ok = ok && wr(f, &timeRemaining, sizeof(float));
    ok = ok && wr(f, &stamina, sizeof(float));
    ok = ok && wr(f, &shieldActive, sizeof(int));
    ok = ok && wr(f, &speedBoostTime, sizeof(float));
    ok = ok && wr(f, &torchBoostTime, sizeof(float));
    ok = ok && wr(f, &rogueFreeSteps, sizeof(int));
    ok = ok && wr(f, inventory, sizeof(int) * 4);
    ok = ok && wr(f, keysCollected, sizeof(int) * 3);
    ok = ok && wr(f, doorPositions, sizeof(doorPositions));
    ok = ok && wr(f, doorExists, sizeof(doorExists));
    ok = ok && wr(f, &playerDir, sizeof(int));
    ok = ok && wr(f, &viewRadius, sizeof(int));
    ok = ok && wr(f, &mapSeed, sizeof(unsigned int));

    // Grids
    ok = ok && wr(f, map_change, sizeof(int) * 100 * 100);
    ok = ok && wr(f, fog, sizeof(int) * 100 * 100);
    ok = ok && wr(f, iceMap, sizeof(int) * 100 * 100);

    // Entities (fixed-size arrays plus counts)
    ok = ok && wr(f, &coinCount, sizeof(int));
    ok = ok && wr(f, coins, sizeof(coins));
    ok = ok && wr(f, &trapCount, sizeof(int));
    ok = ok && wr(f, traps, sizeof(traps));
    ok = ok && wr(f, &portalCount, sizeof(int));
    ok = ok && wr(f, portals, sizeof(portals));
    ok = ok && wr(f, &boxCount, sizeof(int));
    ok = ok && wr(f, boxes, sizeof(boxes));
    ok = ok && wr(f, &plateCount, sizeof(int));
    ok = ok && wr(f, plates, sizeof(plates));
    ok = ok && wr(f, &enemyCount, sizeof(int));
    ok = ok && wr(f, enemies, sizeof(enemies));
    ok = ok && wr(f, &itemCount, sizeof(int));
    ok = ok && wr(f, items, sizeof(items));
    ok = ok && wr(f, &hiddenRoomX, sizeof(int));
    ok = ok && wr(f, &hiddenRoomY, sizeof(int));
    ok = ok && wr(f, &hiddenRoomFound, sizeof(int));

    fclose(f);
    (void)ok;
}

int LoadGame(void)
{
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return 0;

    int ok = 1;
    char magic[4] = {0};
    int version = 0;
    ok = ok && rd(f, magic, 4);
    ok = ok && rd(f, &version, sizeof(int));
    if (!ok || memcmp(magic, SAVE_MAGIC, 4) != 0 || version != SAVE_VERSION) {
        fclose(f);
        return 0;
    }

    int svRow = Row, svCol = Col, svLevel = currentLevel;
    ok = ok && rd(f, &svRow, sizeof(int));
    ok = ok && rd(f, &svCol, sizeof(int));
    if (svRow < 5 || svRow > 98 || svCol < 5 || svCol > 98) ok = 0;
    Row = svRow; Col = svCol;

    ok = ok && rd(f, &X, sizeof(int));
    ok = ok && rd(f, &Y, sizeof(int));
    ok = ok && rd(f, &Hp, sizeof(int));
    ok = ok && rd(f, &step, sizeof(int));
    ok = ok && rd(f, &score, sizeof(int));
    ok = ok && rd(f, &svLevel, sizeof(int));
    ok = ok && rd(f, &selectedCharacter, sizeof(int));
    ok = ok && rd(f, &is_key, sizeof(int));
    ok = ok && rd(f, &coinsCollected, sizeof(int));
    ok = ok && rd(f, &gemsCollected, sizeof(int));
    ok = ok && rd(f, &totalCoins, sizeof(int));
    ok = ok && rd(f, &totalGems, sizeof(int));
    ok = ok && rd(f, &kills, sizeof(int));
    ok = ok && rd(f, &timedMode, sizeof(int));
    ok = ok && rd(f, &timeRemaining, sizeof(float));
    ok = ok && rd(f, &stamina, sizeof(float));
    ok = ok && rd(f, &shieldActive, sizeof(int));
    ok = ok && rd(f, &speedBoostTime, sizeof(float));
    ok = ok && rd(f, &torchBoostTime, sizeof(float));
    ok = ok && rd(f, &rogueFreeSteps, sizeof(int));
    ok = ok && rd(f, inventory, sizeof(int) * 4);
    ok = ok && rd(f, keysCollected, sizeof(int) * 3);
    ok = ok && rd(f, doorPositions, sizeof(doorPositions));
    ok = ok && rd(f, doorExists, sizeof(doorExists));
    ok = ok && rd(f, &playerDir, sizeof(int));
    ok = ok && rd(f, &viewRadius, sizeof(int));
    ok = ok && rd(f, &mapSeed, sizeof(unsigned int));

    ok = ok && rd(f, map_change, sizeof(int) * 100 * 100);
    ok = ok && rd(f, fog, sizeof(int) * 100 * 100);
    ok = ok && rd(f, iceMap, sizeof(int) * 100 * 100);

    ok = ok && rd(f, &coinCount, sizeof(int));
    ok = ok && rd(f, coins, sizeof(coins));
    ok = ok && rd(f, &trapCount, sizeof(int));
    ok = ok && rd(f, traps, sizeof(traps));
    ok = ok && rd(f, &portalCount, sizeof(int));
    ok = ok && rd(f, portals, sizeof(portals));
    ok = ok && rd(f, &boxCount, sizeof(int));
    ok = ok && rd(f, boxes, sizeof(boxes));
    ok = ok && rd(f, &plateCount, sizeof(int));
    ok = ok && rd(f, plates, sizeof(plates));
    ok = ok && rd(f, &enemyCount, sizeof(int));
    ok = ok && rd(f, enemies, sizeof(enemies));
    ok = ok && rd(f, &itemCount, sizeof(int));
    ok = ok && rd(f, items, sizeof(items));
    ok = ok && rd(f, &hiddenRoomX, sizeof(int));
    ok = ok && rd(f, &hiddenRoomY, sizeof(int));
    ok = ok && rd(f, &hiddenRoomFound, sizeof(int));

    fclose(f);
    if (!ok) return 0;
    if (svLevel < 0 || svLevel > 2) return 0;
    currentLevel = svLevel;

    // Rebuild derived rendering/pathfinding state
    cellSize = (double)fmin((screenWidth - 220.0) / Col, (screenHeight - 120.0) / Row);
    gridOffsetX = (screenWidth - Col * cellSize) / 2.0 - 50.0;
    gridOffsetY = 60.0;
    for (int i = 0; i <= Row + 1; i++)
        for (int j = 0; j <= Col + 1; j++)
            map[i][j] = map_change[i][j];
    CreatWalllist();
    OptimalSolution();

    // Loading always returns to top-down view with a usable mouse
    fpMode = 0;
    fpMouseLook = 0;
    EnableCursor();
    gameState = STATE_MAZE;
    return 1;
}
