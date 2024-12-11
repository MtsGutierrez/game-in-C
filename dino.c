#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "dino.h"
#include "helicopter.h"

extern int DINO_SPEED;
extern int SCREEN_WIDTH;

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
    return dinoInfo;
}

// Função concorrente para mover a posição lógica dos dinossauros
void *moveDino(void *arg)
{
    MoveDinoThreadParams *params = (MoveDinoThreadParams *)arg;
    DinoInfo *dinoInfo = params->dinoInfo;

    while (1)
    {
        // Atualiza a posição do dinossauro
        dinoInfo->rect.x += dinoInfo->speed;

        // Atualiza a direção que o dino está olhando
        dinoInfo->facingLeft = (dinoInfo->speed < 0);

        // Se o dinossauro alcançar os limites, inverte a direção
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
    Uint32 ticks = SDL_GetTicks();
    Uint32 sprite = (ticks / 100) % 6; // 6 frames, mudando a cada 100ms
    
    // Cada frame tem 100x100 pixels no spritesheet
    SDL_Rect srcrect = {sprite * 100, 0, 100, 100};
    SDL_RendererFlip flip = dino->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    SDL_RenderCopyEx(renderer, dino->texture, &srcrect, &dino->rect, 0, NULL, flip);
}