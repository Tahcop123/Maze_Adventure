#ifndef _LEVEL_H
#define _LEVEL_H

typedef struct {
    int rows;
    int cols;
    float hpMultiplier;
    int enemyCount;
    int itemCount;
    int coinCount;
    int trapCount;
    int timeLimit; // 0 = no limit
    const char *name;
} LevelConfig;

extern LevelConfig levelConfigs[3];

void StartLevel(int level);
void ResetRunState(void);
void NextLevel(void);
void GenerateLevelContent(void);
void LoadMapAndReset(void);

#endif
