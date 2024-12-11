#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "helicopter.h"

#ifndef DINO_H
#define DINO_H

typedef struct
{
    SDL_Rect rect;
    int speed;
    SDL_Texture *texture;
    int currentFrame; // Para controlar a animação
    bool facingLeft; // Para controlar a direção que o dino está olhando
} DinoInfo;

typedef struct
{
    HelicopterInfo *helicopterInfo;
    DinoInfo *dinoInfo;
} MoveDinoThreadParams;

DinoInfo createDino(int x, int y, int w, int h);
void *moveDino(void *arg);
void loadDinoSprite(DinoInfo *dino, SDL_Renderer* renderer);
void drawDino(DinoInfo* dino, SDL_Renderer* renderer);

#endif /* DINO_H */ 