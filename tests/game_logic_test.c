#include "inc.h"
#include <assert.h>
#include <stdio.h>

static void TestMapLoadIsTransactional(void)
{
    Row = 15;
    Col = 20;
    map_change[1][1] = CELL_KEY;
    assert(!LoadMapFile("tests/fixtures/truncated_map.txt"));
    assert(Row == 15 && Col == 20 && map_change[1][1] == CELL_KEY);
    assert(LoadMapFile("tests/fixtures/valid_map.txt"));
    assert(Row == 5 && Col == 5);
    assert(map_change[1][1] == CELL_START);
    assert(map_change[5][5] == CELL_EXIT);
    assert(map_change[0][0] == CELL_BORDER);
    assert(LoadMapFile("tests/fixtures/edit_map.txt"));
    X = Y = 1;
    OptimalSolution();
    assert(shortstep == 0);
}

static void TestDoorUsesCollectedKey(void)
{
    Row = Col = 5;
    map_change[2][2] = CELL_DOOR_RED;
    keysCollected[0] = 0;
    assert(!OpenDoorIfUnlocked(2, 2));
    assert(map_change[2][2] == CELL_DOOR_RED);
    keysCollected[0] = 1;
    assert(OpenDoorIfUnlocked(2, 2));
    assert(map_change[2][2] == CELL_ROAD);
}

static void TestHiddenRoomPreservesObjectives(void)
{
    Row = 15;
    Col = 20;
    for (unsigned seed = 1; seed <= 300; seed++) {
        for (int i = 1; i <= Row; i++)
            for (int j = 1; j <= Col; j++) map_change[i][j] = CELL_ROAD;
        map_change[2][2] = CELL_START;
        map_change[14][19] = CELL_EXIT;
        map_change[8][12] = CELL_KEY;
        coinCount = 0;
        InitItems();
        srand(seed);
        CreateHiddenRoom();
        assert(map_change[2][2] == CELL_START);
        assert(map_change[14][19] == CELL_EXIT);
        assert(map_change[8][12] == CELL_KEY);
    }
}

static void TestRunResetAndTimers(void)
{
    score = 99;
    kills = 2;
    inventory[ITEM_BOMB] = 3;
    bombsUsedTotal = 4;
    hintsUsed = 1;
    ResetRunState();
    assert(score == 0 && kills == 0 && inventory[ITEM_BOMB] == 0);
    assert(bombsUsedTotal == 0 && hintsUsed == 0);

    InitMapEntities();
    trapCount = 1;
    traps[0].active = 1;
    traps[0].cycle = 0;
    UpdateTraps(0.5f);
    assert(traps[0].cycle == 30);
    UpdateTraps(0.5f);
    assert(traps[0].cycle == 60);

    Row = 15;
    Col = 20;
    X = Y = 1;
    for (int i = 1; i <= Row; i++)
        for (int j = 1; j <= Col; j++) map_change[i][j] = CELL_WALL;
    InitEnemies();
    srand(1);
    SpawnEnemy(5, 5, ENEMY_SLIME);
    UpdateEnemies(0.5f);
    assert(enemies[0].moveCounter == 30);
    UpdateEnemies(1.0f / 6.0f);
    assert(enemies[0].moveCounter == 0);

    for (int i = 1; i <= Row; i++)
        for (int j = 1; j <= Col; j++) map_change[i][j] = CELL_ROAD;
    InitEnemies();
    SpawnEnemy(10, 10, ENEMY_GHOST);
    UpdateEnemies(1.0f);
    assert(enemies[0].x == 9 && enemies[0].y == 9);

    mapSeed = 1;
    selectedCharacter = CHAR_KNIGHT;
    rogueFreeSteps = 7;
    StartLevel(0);
    assert(rogueFreeSteps == 0);
    selectedCharacter = CHAR_ROGUE;
    StartLevel(0);
    assert(rogueFreeSteps == 10);
}

static void TestGeneratedObjectivesRemainReachable(void)
{
    selectedCharacter = CHAR_KNIGHT;
    for (int level = 0; level < 3; level++) {
        for (unsigned seed = 1; seed <= 50; seed++) {
            mapSeed = seed;
            StartLevel(level);
            int starts = 0, keys = 0, exits = 0;
            for (int i = 1; i <= Row; i++)
                for (int j = 1; j <= Col; j++) {
                    starts += map_change[i][j] == CELL_START;
                    keys += map_change[i][j] == CELL_KEY;
                    exits += map_change[i][j] == CELL_EXIT;
                }
            assert(starts == 1 && keys == 1 && exits == 1);
            assert(shortstep > 0);
        }
    }
}

int main(void)
{
    TestMapLoadIsTransactional();
    TestDoorUsesCollectedKey();
    TestHiddenRoomPreservesObjectives();
    TestRunResetAndTimers();
    TestGeneratedObjectivesRemainReachable();
    puts("game logic tests passed");
    return 0;
}
