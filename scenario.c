#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "scenario.h"

extern int EXPLOSION_SIZE;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int BUILDING_HEIGHT;
extern int GROUND_HEIGHT;

// Função pra criar um objeto do cenário
ScenarioElementInfo createScenarioElement(int x, int y, int w, int h)
{
    ScenarioElementInfo rectInfo;
    rectInfo.rect.x = x;
    rectInfo.rect.y = y;
    rectInfo.rect.w = w;
    rectInfo.rect.h = h;
    return rectInfo;
}

void drawExplosion(SDL_Renderer* renderer, int x, int y, int size)
{
    SDL_Surface* image = IMG_Load("sprites/explosion_spritesheet.png");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);

    for (int i = 0; i < 4; i++)
    {
        SDL_Rect srcrect = {i * 32, 0, 32, 32};
        SDL_Rect dstrect = { x - size/2, y - size/2, size, size};
        SDL_RenderCopy(renderer, texture, &srcrect, &dstrect);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
}

void loadScenarioSpritesheet(SDL_Renderer* renderer, ScenarioElementInfo *scenarioElement, char *spritesheet)
{
    SDL_Surface * image = IMG_Load(spritesheet);
    scenarioElement->texture = SDL_CreateTextureFromSurface(renderer, image);
}

void drawScenarioElement(SDL_Renderer* renderer, ScenarioElementInfo* scenarioElement)
{
    SDL_Rect srcrect = {0, 0, scenarioElement->rect.w, scenarioElement->rect.h};
    SDL_RenderCopy(renderer, scenarioElement->texture, &srcrect, &scenarioElement->rect);
}