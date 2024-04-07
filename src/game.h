#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "map.h"
#include "player.h"
#include "vec.h"

typedef struct {
  Player *players;
  size_t players_capacity;
  size_t players_count;
  Map map;
} Game;

void game_init(Game *game);
void game_free(Game *game);

void game_update(Game *game, Vec2I p1_in);
void game_add_player(Game *game, Player player);

#endif // !SNAKE_GAME_H
