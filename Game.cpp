/**
 * @file Game.cpp
 * @brief Main game logic implementation.
 */

#include "Game.h"
#include "Utils.h"

#include <iostream>
using namespace std;

int Game::run() {
    if (!init()) {
        return -1;
    }

    while (running) {
        Uint64 now = SDL_GetTicks();

        if (invincible && now - lastHitTime > INVINCIBLE_MS) {
            invincible = false;
        }

        handleInput();
        update();
        handleEnemyCollision(now);
        updateWindowTitle();
        render();

        SDL_Delay(16);
    }

    cleanup();
    return 0;
}

bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cout << SDL_GetError() << endl;
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("Digger", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        cout << SDL_GetError() << endl;
        return false;
    }

    if (!loadTextures()) {
        return false;
    }

    // ===== Initial tunnels =====
    for (int c = 0; c < 10; c++) {
        dug[2 * COLS + c] = true;
    }

    for (int r = 2; r < 6; r++) {
        dug[r * COLS + 4] = true;
    }

    for (int c = 4; c < 10; c++) {
        dug[5 * COLS + c] = true;
    }

    for (int r = 5; r < 10; r++) {
        dug[r * COLS + 9] = true;
    }

    return true;
}

bool Game::loadTextures() {
    SDL_Surface* pSurf = IMG_Load("C:/Users/IMOE001/source/repos/Project393/Project393/res/player.png");
    SDL_Surface* eSurf = IMG_Load("C:/Users/IMOE001/source/repos/Project393/Project393/res/enemy.png");
    SDL_Surface* gSurf = IMG_Load("C:/Users/IMOE001/source/repos/Project393/Project393/res/gold.png");
    SDL_Surface* hSurf = IMG_Load("C:/Users/IMOE001/source/repos/Project393/Project393/res/heart.png");
    SDL_Surface* bSurf = IMG_Load("C:/Users/IMOE001/source/repos/Project393/Project393/res/bonus.png");

    if (!pSurf || !eSurf || !gSurf || !hSurf || !bSurf) {
        cout << "Failed to load image: " << SDL_GetError() << endl;
        return false;
    }

    Uint32 whiteP = SDL_MapRGB(SDL_GetPixelFormatDetails(pSurf->format), nullptr, 245, 245, 245);
    Uint32 whiteE = SDL_MapRGB(SDL_GetPixelFormatDetails(eSurf->format), nullptr, 245, 245, 245);
    Uint32 whiteG = SDL_MapRGB(SDL_GetPixelFormatDetails(gSurf->format), nullptr, 245, 245, 245);
    Uint32 whiteH = SDL_MapRGB(SDL_GetPixelFormatDetails(hSurf->format), nullptr, 245, 245, 245);
    Uint32 whiteB = SDL_MapRGB(SDL_GetPixelFormatDetails(bSurf->format), nullptr, 245, 245, 245);

    SDL_SetSurfaceColorKey(pSurf, true, whiteP);
    SDL_SetSurfaceColorKey(eSurf, true, whiteE);
    SDL_SetSurfaceColorKey(gSurf, true, whiteG);
    SDL_SetSurfaceColorKey(hSurf, true, whiteH);
    SDL_SetSurfaceColorKey(bSurf, true, whiteB);

    playerTex = SDL_CreateTextureFromSurface(renderer, pSurf);
    enemyTex = SDL_CreateTextureFromSurface(renderer, eSurf);
    goldTex = SDL_CreateTextureFromSurface(renderer, gSurf);
    heartTex = SDL_CreateTextureFromSurface(renderer, hSurf);
    bonusTex = SDL_CreateTextureFromSurface(renderer, bSurf);

    SDL_DestroySurface(pSurf);
    SDL_DestroySurface(eSurf);
    SDL_DestroySurface(gSurf);
    SDL_DestroySurface(hSurf);
    SDL_DestroySurface(bSurf);

    if (!playerTex || !enemyTex || !goldTex || !heartTex || !bonusTex) {
        cout << "Failed to create texture: " << SDL_GetError() << endl;
        return false;
    }

    return true;
}

void Game::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);
    const float playerSpeed = 3.0f;

    if (keys[SDL_SCANCODE_LEFT])  player.x -= playerSpeed;
    if (keys[SDL_SCANCODE_RIGHT]) player.x += playerSpeed;
    if (keys[SDL_SCANCODE_UP])    player.y -= playerSpeed;
    if (keys[SDL_SCANCODE_DOWN])  player.y += playerSpeed;

    if (player.x < 0) player.x = 0;
    if (player.y < 0) player.y = 0;
    if (player.x + player.w > WINDOW_WIDTH) player.x = WINDOW_WIDTH - player.w;
    if (player.y + player.h > WINDOW_HEIGHT) player.y = WINDOW_HEIGHT - player.h;
}

void Game::update() {
    Uint64 now = SDL_GetTicks();

    digCurrentCell();

    spawnEnemy(now);
    updateEnemies();

    handleShooting();
    updateBullet();
    updateFallingBonus();

    handleGoldCollection();
    handleHeartBonus();

    bool allCollected = true;
    for (int i = 0; i < GOLD_COUNT; i++) {
        if (goldActive[i]) {
            allCollected = false;
            break;
        }
    }

    if (allCollected) {
        cout << "YOU WIN! Final score: " << score << endl;
        running = false;
    }
}

void Game::digCurrentCell() {
    int gridX = (int)(player.x / TILE_SIZE);
    int gridY = (int)(player.y / TILE_SIZE);

    if (gridX >= 0 && gridX < COLS && gridY >= 0 && gridY < ROWS) {
        dug[gridY * COLS + gridX] = true;
    }
}

void Game::handleGoldCollection() {
    for (int i = 0; i < GOLD_COUNT; i++) {
        if (goldActive[i] && isCollision(player, gold[i])) {
            goldActive[i] = false;
            score += 10;
            cout << "Collected gold! Score: " << score << endl;
        }
    }
}

void Game::handleHeartBonus() {
    if (heartActive && isCollision(player, heartBonus)) {
        heartActive = false;
        lives++;
        cout << "Extra life! Lives: " << lives << endl;
    }
}

void Game::spawnEnemy(Uint64 now) {
    if (now - lastEnemySpawnTime < ENEMY_SPAWN_MS) {
        return;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemyActive[i]) {
            enemies[i] = { 760, 40, 40, 40 };
            enemyActive[i] = true;
            lastEnemySpawnTime = now;
            cout << "Enemy spawned!" << endl;
            return;
        }
    }
}

void Game::updateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyActive[i]) {
            moveEnemyTowardPlayer(enemies[i], player, 0.8f);
        }
    }
}

void Game::handleShooting() {
    const bool* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_SPACE] && !bulletActive) {
        bullet.x = player.x + player.w;
        bullet.y = player.y + player.h / 2 - 3;
        bullet.w = 12;
        bullet.h = 6;
        bulletActive = true;
    }
}

void Game::updateBullet() {
    if (!bulletActive) {
        return;
    }

    bullet.x += bulletSpeed;

    if (bullet.x > WINDOW_WIDTH) {
        bulletActive = false;
        return;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyActive[i] && isCollision(bullet, enemies[i])) {
            enemyActive[i] = false;
            bulletActive = false;
            score += 20;
            cout << "Enemy killed! Score: " << score << endl;
            return;
        }
    }
}

void Game::updateFallingBonus() {
    for (int i = 0; i < BONUS_COUNT; i++) {
        if (!bonusActive[i]) {
            continue;
        }

        int leftCol = (int)(fallingBonus[i].x / TILE_SIZE);
        int rightCol = (int)((fallingBonus[i].x + fallingBonus[i].w - 1) / TILE_SIZE);

        int bottomRow = (int)((fallingBonus[i].y + fallingBonus[i].h - 1) / TILE_SIZE);
        int rowBelow = bottomRow + 1; // התא שמתחת ל־bonus באמת

        bool emptyBelow = false;

        if (rowBelow >= 0 && rowBelow < ROWS) {
            if (leftCol >= 0 && leftCol < COLS &&
                dug[rowBelow * COLS + leftCol]) {
                emptyBelow = true;
            }

            if (rightCol >= 0 && rightCol < COLS &&
                dug[rowBelow * COLS + rightCol]) {
                emptyBelow = true;
            }
        }

        if (emptyBelow) {
            bonusFalling[i] = true;
        }

        if (bonusFalling[i]) {

            fallingBonus[i].y += bonusFallSpeed;

            if (isCollision(fallingBonus[i], player)) {
                lives = 0;
                cout << "Killed by falling bonus!" << endl;
                cout << "Game Over!" << endl;
                running = false;
            }

            for (int j = 0; j < MAX_ENEMIES; j++) {

                if (enemyActive[j] && isCollision(fallingBonus[i], enemies[j])) {

                    enemyActive[j] = false;

                    bonusActive[i] = false;
                    bonusFalling[i] = false;

                    score += 20;

                    cout << "Enemy crushed by falling bonus! Score: "
                        << score << endl;

                    break;
                }
            }

            if (fallingBonus[i].y + fallingBonus[i].h >= WINDOW_HEIGHT) {

                fallingBonus[i].y = WINDOW_HEIGHT - fallingBonus[i].h;

                bonusFalling[i] = false;
            }
        }
    }
}
void Game::handleEnemyCollision(Uint64 now) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemyActive[i]) {
            continue;
        }

        if (!invincible && isCollision(player, enemies[i])) {
            lives--;
            invincible = true;
            lastHitTime = now;

            cout << "Hit by enemy. Lives: " << lives << endl;

            if (lives <= 0) {
                cout << "Game Over!" << endl;
                running = false;
            }
        }
    }
}

void Game::updateWindowTitle() {

    string text = "Digger - Score: "
        + to_string(score)
        + " Lives: "
        + to_string(lives);

    if (!running) {

        if (lives <= 0) {
            text += " - GAME OVER";
        }
        else {
            text += " - YOU WIN";
        }
    }

    SDL_SetWindowTitle(window, text.c_str());
}

void Game::renderEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyActive[i]) {
            SDL_RenderTexture(renderer, enemyTex, nullptr, &enemies[i]);
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 25, 20, 15, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 90, 60, 30, 255);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int index = r * COLS + c;

            if (!dug[index]) {
                SDL_FRect tile = {
                    (float)c * TILE_SIZE,
                    (float)r * TILE_SIZE,
                    (float)TILE_SIZE,
                    (float)TILE_SIZE
                };

                SDL_RenderFillRect(renderer, &tile);

                SDL_SetRenderDrawColor(renderer, 60, 40, 20, 255);
                SDL_RenderRect(renderer, &tile);
                SDL_SetRenderDrawColor(renderer, 90, 60, 30, 255);
            }
        }
    }

    for (int i = 0; i < GOLD_COUNT; i++) {
        if (goldActive[i]) {
            SDL_RenderTexture(renderer, goldTex, nullptr, &gold[i]);
        }
    }

    if (heartActive) {
        SDL_RenderTexture(renderer, heartTex, nullptr, &heartBonus);
    }

    for (int i = 0; i < BONUS_COUNT; i++) {
        if (bonusActive[i]) {
            SDL_RenderTexture(renderer, bonusTex, nullptr, &fallingBonus[i]);
        }
    }

    if (bulletActive) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &bullet);
    }

    Uint64 now = SDL_GetTicks();

    if (!invincible || (now / 100) % 2 == 0) {
        SDL_RenderTexture(renderer, playerTex, nullptr, &player);
    }

    renderEnemies();

    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex);
    SDL_DestroyTexture(goldTex);
    SDL_DestroyTexture(heartTex);
    SDL_DestroyTexture(bonusTex);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}