#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <raylib.h>

#define MAX_PARTICLES 500

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    float size;
    Color color;
    int active;
} Particle;

void InitParticles(void);
void SpawnParticle(float x, float y, float vx, float vy, float life, float size, Color color);
void SpawnDust(float x, float y);
void SpawnKeyGlow(float x, float y);
void SpawnMagic(float x, float y);
void UpdateParticles(void);
void DrawParticles(void);

#endif
