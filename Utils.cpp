
/**
 * @file Utils.cpp
 * @brief Utility function implementations.
 */
#include "Utils.h"

bool isCollision(const SDL_FRect& a, const SDL_FRect& b) {
    return a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y;
}

void moveEnemyTowardPlayer(SDL_FRect& enemy, const SDL_FRect& player, float speed) {
    if (enemy.x > player.x) enemy.x -= speed;
    if (enemy.x < player.x) enemy.x += speed;
    if (enemy.y > player.y) enemy.y -= speed;
    if (enemy.y < player.y) enemy.y += speed;
}