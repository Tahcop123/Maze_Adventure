#include "particle.h"
#include <stdlib.h>
#include <math.h>

static Particle particles[MAX_PARTICLES];
static int particleCount = 0;

void InitParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = 0;
    }
    particleCount = 0;
}

void SpawnParticle(float x, float y, float vx, float vy, float life, float size, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = vx;
            particles[i].vy = vy;
            particles[i].life = life;
            particles[i].maxLife = life;
            particles[i].size = size;
            particles[i].color = color;
            particles[i].active = 1;
            return;
        }
    }
}

void SpawnDust(float x, float y)
{
    for (int i = 0; i < 5; i++) {
        float angle = (float)rand() / RAND_MAX * PI * 2;
        float speed = (float)rand() / RAND_MAX * 1.5f + 0.5f;
        Color c = (Color){180, 160, 130, (unsigned char)(100 + rand() % 80)};
        SpawnParticle(x, y, cosf(angle) * speed, sinf(angle) * speed - 0.5f,
                      0.5f + (float)rand() / RAND_MAX * 0.3f, 2.0f + rand() % 3, c);
    }
}

void SpawnKeyGlow(float x, float y)
{
    float angle = (float)rand() / RAND_MAX * PI * 2;
    float radius = 8.0f + (float)rand() / RAND_MAX * 6.0f;
    float px = x + cosf(angle) * radius;
    float py = y + sinf(angle) * radius;
    Color c = (Color){255, 215, 0, (unsigned char)(150 + rand() % 100)};
    SpawnParticle(px, py, -cosf(angle) * 0.3f, -sinf(angle) * 0.3f - 0.2f,
                  1.0f + (float)rand() / RAND_MAX * 0.5f, 1.5f + rand() % 2, c);
}

void SpawnMagic(float x, float y)
{
    float angle = (float)rand() / RAND_MAX * PI * 2;
    float speed = 0.3f + (float)rand() / RAND_MAX * 0.8f;
    Color colors[] = {
        (Color){100, 200, 255, 200},
        (Color){150, 100, 255, 200},
        (Color){255, 100, 200, 200},
        (Color){100, 255, 200, 200}
    };
    Color c = colors[rand() % 4];
    SpawnParticle(x + (float)(rand() % 20 - 10), y,
                  cosf(angle) * speed * 0.3f, -speed,
                  1.2f + (float)rand() / RAND_MAX * 0.8f, 2.0f + rand() % 3, c);
}

void UpdateParticles(void)
{
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].vy += 0.02f; // gravity
            particles[i].life -= dt;
            if (particles[i].life <= 0) {
                particles[i].active = 0;
            }
        }
    }
}

void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            float alpha = particles[i].life / particles[i].maxLife;
            Color c = particles[i].color;
            c.a = (unsigned char)(c.a * alpha);
            DrawCircle((int)particles[i].x, (int)particles[i].y, particles[i].size, c);
        }
    }
}
