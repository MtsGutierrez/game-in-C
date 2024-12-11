#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include "helicopter_bullet.h"
#include "helicopter.h"
#include "dino.h"

BulletInfo bullets[MAX_BULLETS];
int activeBullets = 0;
const int BULLET_SPEED = 10;
const int BULLET_WIDTH = 32;
const int BULLET_HEIGHT = 34;

void initBullets(SDL_Renderer *renderer) {
    SDL_Surface *bulletSurface = IMG_Load("sprites/helicopter_bullet_spritesheet.png");
    SDL_Texture *bulletTexture = SDL_CreateTextureFromSurface(renderer, bulletSurface);
    SDL_FreeSurface(bulletSurface);

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].texture = bulletTexture;
        bullets[i].active = false;
        bullets[i].speed = BULLET_SPEED;
    }
}

void createBullet(HelicopterInfo *helicopter, SDL_Renderer *renderer) {
    if (activeBullets >= MAX_BULLETS) return;

    // Encontra um slot livre para o novo projétil
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].rect.w = BULLET_WIDTH;
            bullets[i].rect.h = BULLET_HEIGHT;
            bullets[i].rect.x = helicopter->rect.x + (helicopter->rect.w / 2) - (BULLET_WIDTH / 2);
            bullets[i].rect.y = helicopter->rect.y + (helicopter->rect.h / 2) - (BULLET_HEIGHT / 2);
            bullets[i].active = true;
            bullets[i].direction = (helicopter->currentMovement == 1) ? -1 : 1;
            activeBullets++;
            break;
        }
    }
}

void moveBullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].rect.x += bullets[i].speed * bullets[i].direction;

            // Desativa o projétil se sair da tela
            if (bullets[i].rect.x < 0 || bullets[i].rect.x > SCREEN_WIDTH) {
                bullets[i].active = false;
                activeBullets--;
            }
        }
    }
}

void drawBullets(SDL_Renderer *renderer) {
    Uint32 ticks = SDL_GetTicks();
    Uint32 sprite = (ticks / 50) % 4; // 4 frames, mudando a cada 50ms
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            // Cada frame tem 32x34 pixels no spritesheet
            SDL_Rect srcrect = {sprite * 32, 0, 32, 34};
            SDL_RenderCopy(renderer, bullets[i].texture, &srcrect, &bullets[i].rect);
        }
    }
}

void checkBulletCollisions(DinoInfo *dino1, DinoInfo *dino2) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            if (SDL_HasIntersection(&bullets[i].rect, &dino1->rect) ||
                SDL_HasIntersection(&bullets[i].rect, &dino2->rect)) {
                bullets[i].active = false;
                activeBullets--;
                // Aqui você pode adicionar lógica adicional quando um dinossauro é atingido
            }
        }
    }
}

void cleanupBullets(void) {
    if (bullets[0].texture != NULL) {
        SDL_DestroyTexture(bullets[0].texture);
    }
} 