#ifndef _SAVE_H
#define _SAVE_H

#define MAX_SCORES 10
#define MAX_ACHIEVEMENTS 15

typedef struct {
    int score;
    int steps;
    int level;
    char name[16];
} ScoreEntry;

extern ScoreEntry highScores[MAX_SCORES];
extern int achievements[MAX_ACHIEVEMENTS];
extern const char *achievementNames[MAX_ACHIEVEMENTS];

void LoadHighScores(void);
void SaveHighScores(void);
void AddHighScore(int score, int steps, int level, const char *name);
void LoadAchievements(void);
void SaveAchievements(void);
void UnlockAchievement(int id);
int IsAchievementUnlocked(int id);
void SaveGame(void);
int LoadGame(void);
void CheckAchievements(void);

#endif
