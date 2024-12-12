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
#include "helicopter_bullet.h"

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
int DINO_SPEED = 3;  // Velocidade inicial padrão
const int BULLET_EXPLOSION_SIZE = 50;  // Tamanho da explosão do projétil

bool destroyed = false;
bool gameover = false;
bool thread_dino1_active = true;
bool thread_dino2_active = true;

ScenarioElementInfo background;
ScenarioElementInfo groundInfo;

// Função pra renderizar os objetos
// Isso não pode ser concorrente porque a tela que o usuário vê é uma zona de exclusão mútua
void render(SDL_Renderer *renderer, DinoManager* manager, HelicopterInfo *helicopterInfo)
{
    // Limpa a tela
    SDL_RenderClear(renderer);
    
    drawScenarioElement(renderer, &background);
    drawScenarioElement(renderer, &groundInfo);

    // Desenha todos os dinossauros
    for (int i = 0; i < manager->numDinos; i++) {
        drawDino(&manager->dinos[i], renderer);
    }

    if (destroyed) 
    {
        drawExplosion(
            renderer,
            helicopterInfo->rect.x + (helicopterInfo->rect.w / 2),
            helicopterInfo->rect.y + (helicopterInfo->rect.h / 2),
            EXPLOSION_SIZE
        );
        gameover = true;
    }
    else drawHelicopter(helicopterInfo, renderer);

    drawBullets(renderer);

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

void setupDifficulty(int difficulty, DinoManager* manager) {
    switch(difficulty) {
        case 1: // Fácil - 2 dinos mais lentos
            DINO_SPEED = 2;
            addDino(manager, SCREEN_WIDTH/3);
            addDino(manager, 2*SCREEN_WIDTH/3);
            break;
            
        case 2: // Médio - 3 dinos com velocidade normal
            DINO_SPEED = 3;
            addDino(manager, SCREEN_WIDTH/4);
            addDino(manager, 2*SCREEN_WIDTH/4);
            addDino(manager, 3*SCREEN_WIDTH/4);
            break;
            
        case 3: // Difícil - 2 dinos iniciais mais rápidos + spawn de novos
            DINO_SPEED = 4;
            addDino(manager, SCREEN_WIDTH/3);
            addDino(manager, 2*SCREEN_WIDTH/3);
            
            // Criar thread para adicionar novos dinos
            pthread_t spawner_thread;
            pthread_create(&spawner_thread, NULL, spawnNewDinos, manager);
            break;
    }
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

    DinoManager* manager = createDinoManager(renderer, MAX_DINOS, GROUND_HEIGHT);
    setupDifficulty(difficulty, manager);

    // Inicializa o array de colisões para o helicóptero
    SDL_Rect **rectArray = (SDL_Rect **)malloc(sizeof(SDL_Rect *) * (MAX_DINOS + 1));

    for (int i = 0; i < manager->numDinos; i++) {
        rectArray[i] = &manager->dinos[i].rect;
    }
    rectArray[manager->numDinos] = &groundInfo.rect;

    HelicopterInfo helicopterInfo = createHelicopter(
        SCREEN_WIDTH - HELICOPTER_WIDTH,
        SCREEN_HEIGHT - GROUND_HEIGHT - HELICOPTER_HEIGHT * 1.5,
        HELICOPTER_WIDTH,
        HELICOPTER_HEIGHT,
        HELICOPTER_SPEED,
        rectArray
    );
    loadHelicopterSprite(&helicopterInfo, renderer);
   
    // Criar threads dos dinossauros
    for (int i = 0; i < manager->numDinos; i++) {
        manager->threadParams[i].helicopterInfo = &helicopterInfo;
        manager->threadParams[i].dinoInfo = &manager->dinos[i];
        pthread_create(&manager->threads[i], NULL, moveDino, &manager->threadParams[i]);
    }

    // Adicionar a thread do helicóptero
    pthread_t thread_helicopter;
    pthread_create(&thread_helicopter, NULL, moveHelicopter, &helicopterInfo);

    int quit = 0;
    SDL_Event e;

    initBullets(renderer);

    while (!quit)
    {
        // Escuta o evento pra fechar a tela do jogo
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_SPACE)
                {
                    createBullet(&helicopterInfo, renderer);
                }
            }
        }

        if (!gameover) {
            moveBullets();
            checkBulletCollisions(manager, renderer);
            render(renderer, manager, &helicopterInfo);
        }
        else 
        {
            printf("Você perdeu! Seu helicóptero foi destruído!");
            quit = 1;
        };
    }
    
    free(helicopterInfo.fixed_collision_rects);

    // Destrói as threads
    for (int i = 0; i < manager->numDinos; i++) {
        if (manager->threadActives[i] && manager->dinos[i].alive)
            pthread_cancel(manager->threads[i]);
    }
    pthread_cancel(thread_helicopter);  // Cancela a thread do helicóptero

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    cleanupBullets();

    return 0;
}
