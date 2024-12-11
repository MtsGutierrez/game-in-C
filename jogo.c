#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include "helicopter.h"
#include "dino.h"
#include "scenario.h"

// Constantes
const int SCREEN_WIDTH = 1100;
const int SCREEN_HEIGHT = 700;
const int GROUND_HEIGHT = 100;
const int BRIDGE_WIDTH = 150;
const int BRIDGE_HEIGHT = GROUND_HEIGHT;
const int DINO_WIDTH = 100;
const int DINO_HEIGHT = 100;
const int HELICOPTER_WIDTH = 150;
const int HELICOPTER_HEIGHT = 75;
const int EXPLOSION_SIZE = 75;
const int HELICOPTER_SPEED = 3;
const int DINO_SPEED = 3;

bool destroyed = false;
bool gameover = false;

ScenarioElementInfo background;
ScenarioElementInfo groundInfo;

// Função pra renderizar os objetos
// Isso não pode ser concorrente porque a tela que o usuário vê é uma zona de exclusão mútua
void render(SDL_Renderer *renderer, DinoInfo *dino1Info, DinoInfo *dino2Info, HelicopterInfo *helicopterInfo)
{
    // Limpa a tela
    SDL_RenderClear(renderer);
    
    drawScenarioElement(renderer, &background);
    drawScenarioElement(renderer, &groundInfo);

    drawDino(dino1Info, renderer);
    drawDino(dino2Info, renderer);

    if (destroyed) 
    {
        drawExplosion(
            renderer,
            helicopterInfo->rect.x + (helicopterInfo->rect.w / 2),
            helicopterInfo->rect.y + (helicopterInfo->rect.h / 2)
        );
        gameover = true;
    }
    else drawHelicopter(helicopterInfo, renderer);

    // Atualiza a tela
    SDL_RenderPresent(renderer);
}

int getDifficultyChoice() {
    int choice;

    printf("======= Qual a dificuldade do jogo ? =======\n");
    printf("1 - Fácil\n");
    printf("2 - Médio\n");
    printf("3 - Difícil\n");
    printf("Digite 1, 2 ou 3: ");

    while (1) {
        if (scanf("%d", &choice) != 1) {
            // Input is not a valid integer
            printf("Por favor, digite um número válido (1, 2 ou 3): ");
            while (getchar() != '\n'); // Clear input buffer
        } else if (choice < 1 || choice > 3) {
            // Input is out of range
            printf("Por favor, escolha 1, 2 ou 3: ");
        } else {
            // Valid input
            break;
        }
    }

    return choice;
}

int main(int argc, char *argv[])
{
    int difficulty = getDifficultyChoice();

    // Inicializa o SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Problema ao inicializar SDL. Erro: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    // Cria uma janela SDL
    SDL_Window *window = SDL_CreateWindow("Jogo Concorrente", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Não foi possível abrir a janela do SDL. Erro: %s\n", SDL_GetError());
        return 1;
    }

    // Cria um renderizador SDL
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderizador do SDL não pôde ser criado. Erro: %s\n", SDL_GetError());
        return 1;
    }

    // Cria os elementos do cenário
    background = createScenarioElement(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    groundInfo = createScenarioElement(0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT);

    loadScenarioSpritesheet(renderer, &background, "sprites/background_spritesheet.png");
    loadScenarioSpritesheet(renderer, &groundInfo, "sprites/ground_spritesheet.png");

    DinoInfo dino1Info = createDino(
        SCREEN_WIDTH/3, 
        SCREEN_HEIGHT - GROUND_HEIGHT - DINO_HEIGHT, 
        DINO_WIDTH, 
        DINO_HEIGHT
    );

    DinoInfo dino2Info = createDino(
        2*SCREEN_WIDTH/3, 
        SCREEN_HEIGHT - GROUND_HEIGHT - DINO_HEIGHT, 
        DINO_WIDTH, 
        DINO_HEIGHT
    );
    
    loadDinoSprite(&dino1Info, renderer);
    loadDinoSprite(&dino2Info, renderer);

    // Inicializa o array de colisões para o helicóptero
    SDL_Rect **rectArray = (SDL_Rect **)malloc(sizeof(SDL_Rect *) * 3);

    rectArray[0] = &dino1Info.rect;
    rectArray[1] = &dino2Info.rect;
    rectArray[2] = &groundInfo.rect;

    HelicopterInfo helicopterInfo = createHelicopter(
        SCREEN_WIDTH - HELICOPTER_WIDTH,
        SCREEN_HEIGHT - GROUND_HEIGHT - HELICOPTER_HEIGHT * 1.5,
        HELICOPTER_WIDTH,
        HELICOPTER_HEIGHT,
        HELICOPTER_SPEED,
        rectArray
    );
    loadHelicopterSprite(&helicopterInfo, renderer);
   
    MoveDinoThreadParams paramsDino1;
    paramsDino1.helicopterInfo = &helicopterInfo;
    paramsDino1.dinoInfo = &dino1Info;

    MoveDinoThreadParams paramsDino2;
    paramsDino2.helicopterInfo = &helicopterInfo;
    paramsDino2.dinoInfo = &dino2Info;

    // Inicializa as threads
    pthread_t thread_dino1, thread_dino2, thread_helicopter;
    pthread_create(&thread_dino1, NULL, moveDino, &paramsDino1);                    // thread do dinossauro 1
    pthread_create(&thread_dino2, NULL, moveDino, &paramsDino2);                    // thread do dinossauro 2
    pthread_create(&thread_helicopter, NULL, moveHelicopter, &helicopterInfo);            // thread do helicóptero

    
    int quit = 0;
    SDL_Event e;

    while (!quit)
    {
        // Escuta o evento pra fechar a tela do jogo
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        if (!gameover) {
            render(renderer, &dino1Info, &dino2Info, &helicopterInfo);
        }
        else 
        {
            printf("Você perdeu! Seu helicóptero foi destruído!");
            quit = 1;
        };
    }
    
    free(helicopterInfo.fixed_collision_rects);

    // Destrói as threads
    pthread_cancel(thread_dino1);
    pthread_cancel(thread_dino2);
    pthread_cancel(thread_helicopter);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
