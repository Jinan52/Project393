#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
/**
 * @file Game.h
 * @brief Main game class for Digger demo.
 */
class Game {
public:
    int run();

private:
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;

    static constexpr int TILE_SIZE = 40;
    static constexpr int ROWS = 15;
    static constexpr int COLS = 20;
    static constexpr int CELL_COUNT = ROWS * COLS;

    static constexpr int GOLD_COUNT = 5;
    static constexpr int MAX_ENEMIES = 5;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Texture* playerTex = nullptr;
    SDL_Texture* enemyTex = nullptr;
    SDL_Texture* goldTex = nullptr;
    SDL_Texture* heartTex = nullptr;
    SDL_Texture* bonusTex = nullptr;

    bool dug[CELL_COUNT] = { false };

    SDL_FRect player = { 0, 0, 40, 40 };

    SDL_FRect enemies[MAX_ENEMIES] = {};
    bool enemyActive[MAX_ENEMIES] = { false };

    Uint64 lastEnemySpawnTime = 0;
    static constexpr Uint64 ENEMY_SPAWN_MS = 5000;

    SDL_FRect bullet = { 0, 0, 12, 6 };
    bool bulletActive = false;
    float bulletSpeed = 7.0f;

    static constexpr int BONUS_COUNT = 4;

    SDL_FRect fallingBonus[BONUS_COUNT] = {
        { 360, 120, 35, 35 },
        { 520, 80, 35, 35 },
        { 240, 240, 35, 35 },
        { 640, 320, 35, 35 }
    };

    bool bonusActive[BONUS_COUNT] = { true, true, true, true };
    bool bonusFalling[BONUS_COUNT] = { false, false, false, false };

    float bonusFallSpeed = 3.0f;

    SDL_FRect heartBonus = { 320, 320, 30, 30 };
    bool heartActive = true;

    SDL_FRect gold[GOLD_COUNT] = {
        { 120, 80, 35, 35 },
        { 400, 200, 35, 35 },
        { 680, 120, 35, 35 },
        { 200, 480, 35, 35 },
        { 560, 400, 35, 35 }
    };

    bool goldActive[GOLD_COUNT] = { true, true, true, true, true };

    int score = 0;
    int lives = 3;
    bool running = true;

    bool invincible = false;
    Uint64 lastHitTime = 0;
    static constexpr Uint64 INVINCIBLE_MS = 1000;

private:
    bool init();
    bool loadTextures();
    void handleInput();
    void update();
    void render();
    void cleanup();

    void digCurrentCell();
    void updateWindowTitle();

    void handleGoldCollection();
    void handleHeartBonus();
    void handleEnemyCollision(Uint64 now);

    void spawnEnemy(Uint64 now);
    void updateEnemies();
    void handleShooting();
    void updateBullet();
    void updateFallingBonus();
    void renderEnemies();
};