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

// Returns 1 on success so the UI can give feedback (disk full, no permission...)
int SaveGame(void)
{
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return 0;

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
    return ok;
}

// Staging buffers: the file is fully read and validated before ANY game state
// is committed, so a corrupt/truncated/hostile save cannot leave the game in a
// half-loaded state or push entity loops out of bounds.
static int stageMap[100][100];
static int stageFog[100][100];
static int stageIce[100][100];
static Coin stageCoins[MAX_COINS];
static Trap stageTraps[MAX_TRAPS];
static Portal stagePortals[MAX_PORTALS];
static Box stageBoxes[MAX_BOXES];
static Plate stagePlates[MAX_PLATES];
static Enemy stageEnemies[MAX_ENEMIES];
static Item stageItems[MAX_ITEMS];

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

    // --- Staged scalars (read in SaveGame's exact field order) ---
    int svRow, svCol, svX, svY, svHp, svStep, svScore, svLevel, svChar, svIsKey;
    int svCoins, svGems, svTotalCoins, svTotalGems, svKills, svTimed;
    float svTimeRemaining, svStamina, svSpeedBoost, svTorchBoost;
    int svShield, svRogue, svInventory[4], svKeys[3], svDoorPos[3][2], svDoorExists[3];
    int svPlayerDir, svViewRadius;
    unsigned int svSeed;

    ok = ok && rd(f, &svRow, sizeof(int));
    ok = ok && rd(f, &svCol, sizeof(int));
    ok = ok && rd(f, &svX, sizeof(int));
    ok = ok && rd(f, &svY, sizeof(int));
    ok = ok && rd(f, &svHp, sizeof(int));
    ok = ok && rd(f, &svStep, sizeof(int));
    ok = ok && rd(f, &svScore, sizeof(int));
    ok = ok && rd(f, &svLevel, sizeof(int));
    ok = ok && rd(f, &svChar, sizeof(int));
    ok = ok && rd(f, &svIsKey, sizeof(int));
    ok = ok && rd(f, &svCoins, sizeof(int));
    ok = ok && rd(f, &svGems, sizeof(int));
    ok = ok && rd(f, &svTotalCoins, sizeof(int));
    ok = ok && rd(f, &svTotalGems, sizeof(int));
    ok = ok && rd(f, &svKills, sizeof(int));
    ok = ok && rd(f, &svTimed, sizeof(int));
    ok = ok && rd(f, &svTimeRemaining, sizeof(float));
    ok = ok && rd(f, &svStamina, sizeof(float));
    ok = ok && rd(f, &svShield, sizeof(int));
    ok = ok && rd(f, &svSpeedBoost, sizeof(float));
    ok = ok && rd(f, &svTorchBoost, sizeof(float));
    ok = ok && rd(f, &svRogue, sizeof(int));
    ok = ok && rd(f, svInventory, sizeof(int) * 4);
    ok = ok && rd(f, svKeys, sizeof(int) * 3);
    ok = ok && rd(f, svDoorPos, sizeof(svDoorPos));
    ok = ok && rd(f, svDoorExists, sizeof(svDoorExists));
    ok = ok && rd(f, &svPlayerDir, sizeof(int));
    ok = ok && rd(f, &svViewRadius, sizeof(int));
    ok = ok && rd(f, &svSeed, sizeof(unsigned int));

    // --- Staged grids ---
    ok = ok && rd(f, stageMap, sizeof(stageMap));
    ok = ok && rd(f, stageFog, sizeof(stageFog));
    ok = ok && rd(f, stageIce, sizeof(stageIce));

    // --- Staged entities (counts read first, validated below) ---
    int svCoinCount, svTrapCount, svPortalCount, svBoxCount, svPlateCount;
    int svEnemyCount, svItemCount, svHiddenX, svHiddenY, svHiddenFound;
    ok = ok && rd(f, &svCoinCount, sizeof(int));
    ok = ok && rd(f, stageCoins, sizeof(stageCoins));
    ok = ok && rd(f, &svTrapCount, sizeof(int));
    ok = ok && rd(f, stageTraps, sizeof(stageTraps));
    ok = ok && rd(f, &svPortalCount, sizeof(int));
    ok = ok && rd(f, stagePortals, sizeof(stagePortals));
    ok = ok && rd(f, &svBoxCount, sizeof(int));
    ok = ok && rd(f, stageBoxes, sizeof(stageBoxes));
    ok = ok && rd(f, &svPlateCount, sizeof(int));
    ok = ok && rd(f, stagePlates, sizeof(stagePlates));
    ok = ok && rd(f, &svEnemyCount, sizeof(int));
    ok = ok && rd(f, stageEnemies, sizeof(stageEnemies));
    ok = ok && rd(f, &svItemCount, sizeof(int));
    ok = ok && rd(f, stageItems, sizeof(stageItems));
    ok = ok && rd(f, &svHiddenX, sizeof(int));
    ok = ok && rd(f, &svHiddenY, sizeof(int));
    ok = ok && rd(f, &svHiddenFound, sizeof(int));

    fclose(f);
    if (!ok) return 0;

    // --- Validation: reject anything that could crash the game later ---
    if (svRow < 5 || svRow > 98 || svCol < 5 || svCol > 98) return 0;
    if (svLevel < 0 || svLevel > 2) return 0;
    if (svChar < CHAR_KNIGHT || svChar > CHAR_ROGUE) return 0;
    if (svX < 1 || svX > svRow || svY < 1 || svY > svCol) return 0;
    if (svCoinCount < 0 || svCoinCount > MAX_COINS) return 0;
    if (svTrapCount < 0 || svTrapCount > MAX_TRAPS) return 0;
    if (svPortalCount < 0 || svPortalCount > MAX_PORTALS) return 0;
    if (svBoxCount < 0 || svBoxCount > MAX_BOXES) return 0;
    if (svPlateCount < 0 || svPlateCount > MAX_PLATES) return 0;
    if (svEnemyCount < 0 || svEnemyCount > MAX_ENEMIES) return 0;
    if (svItemCount < 0 || svItemCount > MAX_ITEMS) return 0;
    for (int d = 0; d < 3; d++) {
        if (svDoorExists[d] &&
            (svDoorPos[d][0] < 1 || svDoorPos[d][0] > svRow ||
             svDoorPos[d][1] < 1 || svDoorPos[d][1] > svCol)) return 0;
    }

    // --- Commit ---
    Row = svRow;
    Col = svCol;
    X = svX;
    Y = svY;
    Hp = (svHp < 1) ? 1 : svHp;
    step = svStep;
    score = svScore;
    currentLevel = svLevel;
    selectedCharacter = svChar;
    is_key = svIsKey ? 1 : 0;
    coinsCollected = svCoins;
    gemsCollected = svGems;
    totalCoins = svTotalCoins;
    totalGems = svTotalGems;
    kills = svKills;
    timedMode = svTimed ? 1 : 0;
    timeRemaining = (svTimeRemaining < 0) ? 0 : svTimeRemaining;
    stamina = svStamina;
    if (stamina < 0) stamina = 0;
    if (stamina > maxStamina) stamina = maxStamina;
    shieldActive = svShield ? 1 : 0;
    speedBoostTime = (svSpeedBoost < 0) ? 0 : svSpeedBoost;
    torchBoostTime = (svTorchBoost < 0) ? 0 : svTorchBoost;
    rogueFreeSteps = (svRogue < 0) ? 0 : svRogue;
    for (int i = 0; i < 4; i++) inventory[i] = (svInventory[i] > 0) ? svInventory[i] : 0;
    for (int i = 0; i < 3; i++) keysCollected[i] = svKeys[i] ? 1 : 0;
    for (int i = 0; i < 3; i++) {
        doorPositions[i][0] = svDoorPos[i][0];
        doorPositions[i][1] = svDoorPos[i][1];
        doorExists[i] = svDoorExists[i] ? 1 : 0;
    }
    playerDir = (svPlayerDir >= 0 && svPlayerDir <= 3) ? svPlayerDir : 0;
    viewRadius = (svViewRadius >= 1 && svViewRadius <= 10) ? svViewRadius : 4;
    mapSeed = svSeed;

    memcpy(map_change, stageMap, sizeof(stageMap));
    memcpy(fog, stageFog, sizeof(stageFog));
    memcpy(iceMap, stageIce, sizeof(stageIce));

    coinCount = svCoinCount;
    memcpy(coins, stageCoins, sizeof(stageCoins));
    trapCount = svTrapCount;
    memcpy(traps, stageTraps, sizeof(stageTraps));
    portalCount = svPortalCount;
    memcpy(portals, stagePortals, sizeof(stagePortals));
    boxCount = svBoxCount;
    memcpy(boxes, stageBoxes, sizeof(stageBoxes));
    plateCount = svPlateCount;
    memcpy(plates, stagePlates, sizeof(stagePlates));
    enemyCount = svEnemyCount;
    memcpy(enemies, stageEnemies, sizeof(stageEnemies));
    itemCount = svItemCount;
    memcpy(items, stageItems, sizeof(stageItems));
    hiddenRoomX = svHiddenX;
    hiddenRoomY = svHiddenY;
    hiddenRoomFound = svHiddenFound ? 1 : 0;

    // Rebuild derived rendering/pathfinding state
    cellSize = (double)fmin((screenWidth - 220.0) / Col, (screenHeight - 120.0) / Row);
    gridOffsetX = (screenWidth - Col * cellSize) / 2.0 - 50.0;
    gridOffsetY = 60.0;
    for (int i = 0; i <= Row + 1; i++)
        for (int j = 0; j <= Col + 1; j++)
            map[i][j] = map_change[i][j];
    MarkMazeDirty();
    MarkFogDirty();
    OptimalSolution();

    // Loading always returns to top-down view with a usable mouse
    fpMode = 0;
    fpMouseLook = 0;
    EnableCursor();
    gameState = STATE_MAZE;
    return 1;
}
