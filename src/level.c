#include "inc.h"
#include "level.h"

LevelConfig levelConfigs[3] = {
    {15, 20, 3.0f, 1, 3, 10, 0, 0, "Easy"},
    {20, 30, 2.0f, 3, 4, 20, 2, 0, "Normal"},
    {30, 40, 1.5f, 5, 5, 35, 4, 120, "Hard"}
};

void ResetRunState(void)
{
    score = 0;
    kills = 0;
    bombsUsedTotal = 0;
    hintsUsed = 0;
    for (int i = 0; i < 4; i++) inventory[i] = 0;
}

void StartLevel(int level)
{
    if (level < 0 || level >= 3) level = 0;
    currentLevel = level;
    LevelConfig *cfg = &levelConfigs[level];

    // Seed RNG once per run: a fixed mapSeed is reproducible; mapSeed==0
    // (new game / F2) rolls a fresh seed so quick restarts differ.
    if (mapSeed == 0) mapSeed = (unsigned int)time(NULL) ^ (unsigned int)clock();
    srand(mapSeed);

    Row = cfg->rows;
    Col = cfg->cols;
    cellSize = (double)fmin((screenWidth - 220.0) / Col, (screenHeight - 120.0) / Row);
    gridOffsetX = (screenWidth - Col * cellSize) / 2.0 - 50.0;
    gridOffsetY = 60.0;

    CreatMap();
    InitMapEntities();
    InitEnemies();
    InitItems();
    Randomkey();
    GenerateLevelContent();

    X = Y = 2;
    OptimalSolution();
    if (shortstep <= 0) shortstep = 50;
    Hp = (int)(shortstep * cfg->hpMultiplier);
    if (selectedCharacter == CHAR_KNIGHT) Hp += 20;
    rogueFreeSteps = (selectedCharacter == CHAR_ROGUE) ? 10 : 0;

    stamina = maxStamina;
    isSprinting = 0;
    // NOTE: score is intentionally NOT reset here - it accumulates across
    // NextLevel(). Fresh runs / retries reset it at their call sites.
    step = 0;
    coinsCollected = 0;
    gemsCollected = 0;
    is_key = 0;
    shieldActive = 0;
    speedBoostTime = 0;
    torchBoostTime = 0;
    keysCollected[0] = keysCollected[1] = keysCollected[2] = 0;
    doorExists[0] = doorExists[1] = doorExists[2] = 0;
    tookDamageThisLevel = 0;
    iceSlideCount = 0;
    hintsUsed = 0;
    viewRadius = 4;
    playerDir = 1;
    attackCooldown = 0;
    playerAttacking = 0;
    // Restarting from FP must not leave the cursor captured
    fpMode = 0;
    EnableCursor();
    timedMode = (cfg->timeLimit > 0) ? 1 : 0;
    timeRemaining = (float)cfg->timeLimit;
    gameOverReason = 0;

    InitFog();
    UpdateFog();
    InitParticles();
}

// Reset gameplay state after loading a map file (Open button / F9).
// Shared by all map-load entry points so they cannot drift apart.
void LoadMapAndReset(void)
{
    ResetRunState();
    cellSize = (double)fmin((screenWidth - 220.0) / Col, (screenHeight - 120.0) / Row);
    gridOffsetX = (screenWidth - Col * cellSize) / 2.0 - 50.0;
    gridOffsetY = 60.0;
    InitMapEntities();
    InitEnemies();
    InitItems();
    InitFog();
    coinsCollected = gemsCollected = totalCoins = totalGems = 0;
    for (int d = 0; d < 3; d++) {
        keysCollected[d] = 0;
        doorExists[d] = 0;
        doorPositions[d][0] = doorPositions[d][1] = 0;
    }
    for (int i = 1; i <= Row; i++)
        for (int j = 1; j <= Col; j++) {
            int cell = map_change[i][j];
            if (cell == CELL_COIN || cell == CELL_GEM) {
                Coin *coin = &coins[coinCount++];
                coin->x = i;
                coin->y = j;
                coin->value = (cell == CELL_GEM) ? 50 : 10;
                coin->active = 1;
                coin->animTime = 0;
                if (cell == CELL_GEM) totalGems++;
                else totalCoins++;
                map_change[i][j] = CELL_ROAD;
            }
            if (cell >= CELL_DOOR_RED && cell <= CELL_DOOR_GREEN) {
                int d = cell - CELL_DOOR_RED;
                doorExists[d] = 1;
                doorPositions[d][0] = i;
                doorPositions[d][1] = j;
            }
        }
    // Prefer the map's start marker, then the first traversable cell.
    X = Y = 0;
    for (int i = 1; i <= Row; i++)
        for (int j = 1; j <= Col; j++)
            if (map_change[i][j] == CELL_START) { X = i; Y = j; }
    if (X == 0)
        for (int i = 1; i <= Row && X == 0; i++)
            for (int j = 1; j <= Col; j++)
                if (map_change[i][j] == CELL_ROAD) { X = i; Y = j; break; }
    step = 0;
    is_key = 0;
    timedMode = 0;
    timeRemaining = 0;
    mapSeed = 0;
    viewRadius = 4;
    playerDir = 1;
    fpMode = 0;
    fpMouseLook = 0;
    EnableCursor();
    MarkMazeDirty();
    OptimalSolution();
    if (shortstep <= 0) shortstep = 50;
    Hp = shortstep * 2;
    rogueFreeSteps = (selectedCharacter == CHAR_ROGUE) ? 10 : 0;
    stamina = maxStamina;
    isSprinting = 0;
    shieldActive = 0;
    speedBoostTime = 0;
    torchBoostTime = 0;
    tookDamageThisLevel = 0;
    iceSlideCount = 0;
    UpdateFog();
}

void NextLevel(void)
{
    if (currentLevel < 2) {
        StartLevel(currentLevel + 1);
    } else {
        // Beat all levels
        gameState = STATE_WIN;
        CheckAchievements();
    }
}

void GenerateLevelContent(void)
{
    LevelConfig *cfg = &levelConfigs[currentLevel];

    // Spawn enemies on road cells (not near start)
    int spawned = 0;
    int attempts = 0;
    while (spawned < cfg->enemyCount && attempts < 200) {
        attempts++;
        int ex = rand() % (Row - 4) + 3;
        int ey = rand() % (Col - 4) + 3;
        if (map_change[ex][ey] == 0 && (abs(ex - 2) + abs(ey - 2)) > 5) {
            int type = (currentLevel == 2) ? (rand() % 3) : (rand() % 2);
            SpawnEnemy(ex, ey, type);
            spawned++;
        }
    }

    // Spawn items
    spawned = 0;
    attempts = 0;
    while (spawned < cfg->itemCount && attempts < 200) {
        attempts++;
        int ix = rand() % Row + 1;
        int iy = rand() % Col + 1;
        if (map_change[ix][iy] == 0 && !(ix == 2 && iy == 2)) {
            SpawnItem(ix, iy, rand() % 4);
            spawned++;
        }
    }

    // Spawn collectibles
    SpawnCollectibles(cfg->coinCount);

    // Spawn traps
    SpawnTraps(cfg->trapCount);

    // Spawn portals (1 pair)
    SpawnPortals(1);

    // Spawn boxes (2)
    SpawnBoxes(2);

    // Spawn pressure plates (1)
    SpawnPlates(1);

    // Hidden room
    CreateHiddenRoom();

    // Ice patches
    for (int i = 0; i < 5; i++) {
        int ix = rand() % Row + 1;
        int iy = rand() % Col + 1;
        if (map_change[ix][iy] == 0) {
            iceMap[ix][iy] = 1;
        }
    }

    // Colored keys (6=red,7=blue,8=green): Easy 1, Normal 2, Hard 3
    int coloredKeyCount = currentLevel + 1;
    for (int k = 0; k < coloredKeyCount; k++) {
        int placed = 0, attempts = 0;
        while (!placed && attempts++ < 200) {
            int kx = rand() % Row + 1;
            int ky = rand() % Col + 1;
            if (map_change[kx][ky] == 0 && !(kx == 2 && ky == 2) &&
                (abs(kx - 2) + abs(ky - 2)) > 4) {
                map_change[kx][ky] = 6 + k;
                placed = 1;
            }
        }
    }

    totalCoins = 0;
    totalGems = 0;
    for (int i = 0; i < coinCount; i++) {
        if (coins[i].active) {
            if (coins[i].value >= 50) totalGems++;
            else totalCoins++;
        }
    }
}


