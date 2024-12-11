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
    int currentFrame;
    bool facingLeft;
    bool alive;
    bool isJumping;
    int jumpHeight;
    int originalY;
} DinoInfo;

typedef struct
{
    HelicopterInfo *helicopterInfo;
    DinoInfo *dinoInfo;
} MoveDinoThreadParams;

#define MAX_DINOS 10  // Máximo de dinossauros possível

typedef struct {
    DinoInfo* dinos;
    int numDinos;
    int maxDinos;
    SDL_Renderer* renderer;
    int groundHeight;
    pthread_t* threads;
    MoveDinoThreadParams* threadParams;
    bool* threadActives;
} DinoManager;

// Funções para gerenciar dinossauros
DinoManager* createDinoManager(SDL_Renderer* renderer, int maxDinos, int groundHeight);
void addDino(DinoManager* manager, int x);
void updateDinos(DinoManager* manager);
void cleanupDinoManager(DinoManager* manager);

DinoInfo createDino(int x, int y, int w, int h);
void *moveDino(void *arg);
void loadDinoSprite(DinoInfo *dino, SDL_Renderer* renderer);
void drawDino(DinoInfo* dino, SDL_Renderer* renderer);

void* spawnNewDinos(void* arg);

#endif /* DINO_H */ 