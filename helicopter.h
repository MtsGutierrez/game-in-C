#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef HELICOPTER_H
#define HELICOPTER_H

typedef struct
{
    SDL_Rect rect;
    int speed;
    SDL_Rect **fixed_collision_rects;
    SDL_Texture *texture;
    int currentMovement;
} HelicopterInfo;

HelicopterInfo createHelicopter(int x, int y, int w, int h, int speed, SDL_Rect **collisionRectArray);
void checkHelicopterCollisions(SDL_Rect helicopterRect, SDL_Rect *rects[], int rects_length);
void *moveHelicopter(void *arg);
void loadHelicopterSprite(HelicopterInfo *helicopter, SDL_Renderer* renderer);
void drawHelicopter(HelicopterInfo* helicopter, SDL_Renderer* renderer);

#endif /* HELICOPTER_H */