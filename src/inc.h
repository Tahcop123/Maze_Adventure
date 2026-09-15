#ifndef _INC_H
#define _INC_H

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "linkedlist.h"
#include "map.h"
#include "solution.h"
#include "interface.h"
#include "event.h"
#include "file.h"
#include "particle.h"
#include "enemy.h"
#include "item.h"
#include "fog.h"
#include "level.h"
#include "audio.h"
#include "save.h"
#include "fpview.h"

// ========== Game States ==========
typedef enum {
    STATE_MENU = 0,
    STATE_CHARSELECT = 1,
    STATE_DIFFICULTY = 2,
    STATE_MAZE = 3,
    STATE_PAUSE = 4,
    STATE_GAMEOVER = 5,
    STATE_WIN = 6,
    STATE_HELP = 7,
    STATE_ABOUT = 8,
    STATE_LEADERBOARD = 9,
    STATE_ACHIEVEMENTS = 10
} GameState;

// ========== Character Types ==========
typedef enum {
    CHAR_KNIGHT = 0,
    CHAR_MAGE = 1,
    CHAR_ROGUE = 2
} CharacterType;

// ========== Difficulty ==========
typedef enum {
    DIFF_EASY = 0,
    DIFF_NORMAL = 1,
    DIFF_HARD = 2
} Difficulty;

// ========== Global State ==========
extern GameState gameState;
extern int screenWidth;
extern int screenHeight;

// Character & Level
extern int selectedCharacter;
extern int currentLevel;
extern int difficulty;
extern unsigned int mapSeed;
extern int timedMode;
extern float timeRemaining;

// Score & Collectibles
extern int score;
extern int coinsCollected;
extern int gemsCollected;
extern int totalCoins;
extern int totalGems;

// Stamina
extern float stamina;
extern float maxStamina;
extern int isSprinting;

// Inventory (bomb/speed/shield/torch)
extern int inventory[4];
extern int shieldActive;
extern float speedBoostTime;
extern float torchBoostTime;
extern int rogueFreeSteps;

// Fog & View
extern int fog[100][100];
extern int viewRadius;

// Ice map
extern int iceMap[100][100];

// Multi-key multi-door (0=red,1=blue,2=green)
extern int keysCollected[3];
extern int doorPositions[3][2];
extern int doorExists[3];

// Screen shake
extern float shakeAmount;
extern float shakeTime;

// Weather (0=none,1=dust,2=rain)
extern int weatherType;

// Game over reason (0=HP,1=time,2=enemy)
extern int gameOverReason;

// Flash effect (red when hurt)
extern float flashTime;
extern Color flashColor;

// ========== Animation ==========
extern int playerDir;
extern float animFrame;
extern float animTimer;
extern int playerAttacking;
extern float attackAnimTime;

// ========== Combat ==========
extern int playerAttack;
extern int playerDefense;
extern float attackCooldown;
extern int kills;

// ========== Run statistics (achievement tracking) ==========
extern int tookDamageThisLevel;
extern int bombsUsedTotal;
extern int hintsUsed;
extern int iceSlideCount;

// ========== First Person View ==========
extern int fpMode;
extern float fpDirX, fpDirY;
extern float fpPlaneX, fpPlaneY;
extern float fpPosX, fpPosY;
extern float fpMoveSpeed;
extern int fpMouseLook;

// ========== Helper Functions ==========
void TriggerShake(float amount, float duration);
void TriggerFlash(Color color, float duration);
void UpdateEffects(void);
void AddScore(int points);

// Animation
void UpdateAnimation(float dt);
void DrawPlayerAnimated(double px, double py, float cs);

// Combat
void PlayerAttack(void);
void DamageEnemy(int idx, int dmg);
void EnemyAttackPlayer(int idx);

// First Person
void InitFPView(void);
void UpdateFPView(float dt);
void DrawFPView(void);
void ToggleFPMode(void);

#endif
