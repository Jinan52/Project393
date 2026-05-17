
/**
 * @file Utils.h
 * @brief Utility helper functions.
 */
#pragma once
#include <SDL3/SDL.h>

 /**
  * @brief Checks collision between two rectangles.
  */
bool isCollision(const SDL_FRect& a, const SDL_FRect& b);

/**
 * @brief Moves enemy toward the player.
 */
void moveEnemyTowardPlayer(SDL_FRect& enemy, const SDL_FRect& player, float speed);