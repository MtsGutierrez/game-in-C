#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "dino.h"
#include "helicopter.h"

extern int DINO_SPEED;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int DINO_WIDTH;
extern int DINO_HEIGHT;

// Função pra criar um dinossauro
DinoInfo createDino(int x, int y, int w, int h)
{
    DinoInfo dinoInfo;
    dinoInfo.rect.x = x;
    dinoInfo.rect.y = y;
    dinoInfo.rect.w = w;
    dinoInfo.rect.h = h;
    dinoInfo.speed = DINO_SPEED;
    dinoInfo.currentFrame = 0;
    dinoInfo.facingLeft = false;
    dinoInfo.alive = true;
    return dinoInfo;
}

// Função concorrente para mover a posição lógica dos dinossauros
void *moveDino(void *arg)
{
    MoveDinoThreadParams *params = (MoveDinoThreadParams *)arg;
    DinoInfo *dinoInfo = params->dinoInfo;
    const int JUMP_MAX_HEIGHT = 150;
    const int JUMP_SPEED = 5;
    bool jumpingUp = true;

    while (1)
    {
        if (!dinoInfo->alive) {
            pthread_exit(NULL);
        }

        // Movimento horizontal
        dinoInfo->rect.x += dinoInfo->speed;
        dinoInfo->facingLeft = (dinoInfo->speed < 0);

        // Lógica de pulo
        if (dinoInfo->isJumping) {
            if (jumpingUp) {
                dinoInfo->jumpHeight += JUMP_SPEED;
                if (dinoInfo->jumpHeight >= JUMP_MAX_HEIGHT) {
                    jumpingUp = false;
                }
            } else {
                dinoInfo->jumpHeight -= JUMP_SPEED;
                if (dinoInfo->jumpHeight <= 0) {
                    dinoInfo->isJumping = false;
                    dinoInfo->rect.y = dinoInfo->originalY;
                    jumpingUp = true;
                }
            }
            dinoInfo->rect.y = dinoInfo->originalY - dinoInfo->jumpHeight;
        }

        // Chance aleatória de pular
        if (!dinoInfo->isJumping && (rand() % 100 < 2)) { // 2% de chance de pular
            dinoInfo->isJumping = true;
            dinoInfo->jumpHeight = 0;
            dinoInfo->originalY = dinoInfo->rect.y;
        }

        // Inversão de direção nas bordas
        if (dinoInfo->rect.x + dinoInfo->rect.w > SCREEN_WIDTH)
            dinoInfo->speed = -DINO_SPEED;
        else if (dinoInfo->rect.x <= 0)
            dinoInfo->speed = DINO_SPEED;

        SDL_Delay(10);
    }

    return NULL;
}

void loadDinoSprite(DinoInfo *dino, SDL_Renderer* renderer) {
    SDL_Surface * image = IMG_Load("sprites/dino_spritesheet.png");
    dino->texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
}

void drawDino(DinoInfo *dino, SDL_Renderer* renderer) {	
    if (!dino->alive) return;  // Não desenha se estiver morto
    
    Uint32 ticks = SDL_GetTicks();
    Uint32 sprite = (ticks / 100) % 6;
    
    SDL_Rect srcrect = {sprite * 100, 0, 100, 100};
    SDL_RendererFlip flip = dino->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    SDL_RenderCopyEx(renderer, dino->texture, &srcrect, &dino->rect, 0, NULL, flip);
}

DinoManager* createDinoManager(SDL_Renderer* renderer, int maxDinos, int groundHeight) {
    DinoManager* manager = (DinoManager*)malloc(sizeof(DinoManager));
    manager->dinos = (DinoInfo*)malloc(sizeof(DinoInfo) * maxDinos);
    manager->threads = (pthread_t*)malloc(sizeof(pthread_t) * maxDinos);
    manager->threadParams = (MoveDinoThreadParams*)malloc(sizeof(MoveDinoThreadParams) * maxDinos);
    manager->threadActives = (bool*)malloc(sizeof(bool) * maxDinos);
    manager->numDinos = 0;
    manager->maxDinos = maxDinos;
    manager->renderer = renderer;
    manager->groundHeight = groundHeight;
    return manager;
}

void addDino(DinoManager* manager, int x) {
    if (manager->numDinos >= manager->maxDinos) return;
    
    int idx = manager->numDinos;
    manager->dinos[idx] = createDino(
        x,
        SCREEN_HEIGHT - manager->groundHeight - DINO_HEIGHT,
        DINO_WIDTH,
        DINO_HEIGHT
    );
    loadDinoSprite(&manager->dinos[idx], manager->renderer);
    manager->threadActives[idx] = true;
    manager->numDinos++;
}

void* spawnNewDinos(void* arg) {
    DinoManager* manager = (DinoManager*)arg;
    while (1) {
        SDL_Delay(10000); // 10 segundos
        if (manager->numDinos < manager->maxDinos) {
            int x = rand() % (SCREEN_WIDTH - DINO_WIDTH);
            addDino(manager, x);
        }
    }
    return NULL;
}