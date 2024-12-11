#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "cannon.h"
#include "helicopter.h"

extern int CANNON_SPEED;
extern int AMMUNITION;
extern int CANNON_WIDTH;
extern int MIN_COOLDOWN_TIME;
extern int MAX_COOLDOWN_TIME;
extern int SCREEN_WIDTH;
extern int BRIDGE_WIDTH;
extern int MISSILE_WIDTH;
extern int MISSILE_HEIGHT;
extern int MISSILE_SPEED;
extern int RELOAD_TIME_FOR_EACH_MISSILE;

extern pthread_mutex_t bridgeMutex;

// Função pra criar um canhão
CannonInfo createCannon(int x, int y, int w, int h, int initialAmmunition)
{
    CannonInfo cannonInfo;
    cannonInfo.rect.x = x;
    cannonInfo.rect.y = y;
    cannonInfo.rect.w = w;
    cannonInfo.rect.h = h;
    cannonInfo.speed = CANNON_SPEED;
    return cannonInfo;
}

// Função concorrente para mover a posição lógica dos canhões
void *moveCannon(void *arg)
{
    MoveCannonThreadParams *params = (MoveCannonThreadParams *)arg;
    CannonInfo *cannonInfo = params->cannonInfo;

    while (1)
    {
        // Verifica se o canhão está em cima da ponte
        if (cannonInfo->rect.x + CANNON_WIDTH > 0 && cannonInfo->rect.x < BRIDGE_WIDTH)
        {
            pthread_mutex_lock(&bridgeMutex);

            while (
                cannonInfo->rect.x + CANNON_WIDTH > 0 &&
                cannonInfo->rect.x < BRIDGE_WIDTH)
            {
                cannonInfo->rect.x += abs(cannonInfo->speed);
                SDL_Delay(10);
            }

            pthread_mutex_unlock(&bridgeMutex);
        }

        // Atualiza a posição do canhão
        cannonInfo->rect.x += cannonInfo->speed;

        // Se o canhão alcançar os limites, inverte a direção
        if (cannonInfo->rect.x + CANNON_WIDTH > SCREEN_WIDTH)
            cannonInfo->speed = -CANNON_SPEED;
        else if (cannonInfo->rect.x <= BRIDGE_WIDTH)
            cannonInfo->speed = CANNON_SPEED;

        SDL_Delay(10);
    }

    return NULL;
}

void loadCannonSprite(CannonInfo *cannon, SDL_Renderer* renderer) {
    SDL_Surface * image = IMG_Load("sprites/cannon_spritesheet.png");
    cannon->texture = SDL_CreateTextureFromSurface(renderer, image);
}

void drawCannon(CannonInfo *cannon, SDL_Renderer* renderer) {	
    Uint32 ticks = SDL_GetTicks();
    Uint32 ms = ticks / 200;
    
    SDL_Rect srcrect = {(ms % 3) * 50, 0, 50, 25};
    SDL_RenderCopy(renderer, cannon->texture, &srcrect, &cannon->rect);
}