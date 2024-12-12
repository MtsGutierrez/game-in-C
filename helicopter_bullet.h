#ifndef HELICOPTER_BULLET_H
#define HELICOPTER_BULLET_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "helicopter.h"
#include "dino.h"
#include "scenario.h"

#define MAX_BULLETS 10  // Número máximo de projéteis ativos ao mesmo tempo

typedef struct {
    SDL_Rect rect;
    float speed;
    SDL_Texture *texture;
    bool active;
    float angle;  // Para a direção do tiro
    int direction; // -1 para esquerda, 1 para direita
} BulletInfo;

extern BulletInfo bullets[MAX_BULLETS];
extern int activeBullets;

void initBullets(SDL_Renderer *renderer);
void createBullet(HelicopterInfo *helicopter, SDL_Renderer *renderer);
void moveBullets(void);
void drawBullets(SDL_Renderer *renderer);
void checkBulletCollisions(DinoManager* manager, SDL_Renderer *renderer);
void cleanupBullets(void);

#endif 