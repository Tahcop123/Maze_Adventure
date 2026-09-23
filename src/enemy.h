#ifndef _ENEMY_H
#define _ENEMY_H

#define MAX_ENEMIES 20

typedef enum {
    ENEMY_SLIME = 0,
    ENEMY_SKELETON = 1,
    ENEMY_GHOST = 2
} EnemyType;

typedef struct {
    int x, y;
    int type;
    int dir;
    int speed;
    int moveCounter;
    int active;
    float animTime;
    // Combat
    int hp;
    int maxHp;
    int attack;
    int isHurt;
    float hurtTime;
    float attackCooldown;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];
extern int enemyCount;

void InitEnemies(void);
void SpawnEnemy(int x, int y, int type);
void UpdateEnemies(void);
void DrawEnemies(void);

// Combat
void DamageEnemy(int idx, int dmg);
void EnemyAttackPlayer(int idx);
void DropEnemyLoot(int idx);

#endif
