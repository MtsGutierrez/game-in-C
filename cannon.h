#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "helicopter.h"

#ifndef CANNON_H
#define CANNON_H
// Guarda as informações dos objetos
typedef struct
{
    SDL_Rect rect;
    int speed;
    SDL_Texture *texture;
} CannonInfo;

typedef struct
{
    HelicopterInfo *helicopterInfo;
    CannonInfo *cannonInfo;
} MoveCannonThreadParams;

CannonInfo createCannon(int x, int y, int w, int h, int initialAmmunition);
void *moveCannon(void *arg);
void loadCannonSprite(CannonInfo *cannon, SDL_Renderer* renderer);
void drawCannon(CannonInfo* cannon, SDL_Renderer* renderer);

#endif /* CANNON_H */