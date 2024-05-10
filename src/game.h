#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "input.h"
#include "map.h"
#include "player.h"
#include "vec.h"

typedef struct {
  Player player;
  Action current_action;
  Action previous_action;
} PlayerData;

void player_data_init(PlayerData *player_data);
void player_data_free(PlayerData *player_data);
void player_data_set_action(PlayerData *player_data, Action action);

typedef struct {
  // TODO: Currently we just store an array of data associated with each player,
  // this array is indexed by id. An issue with this is that while order is
  // currently stable, it cannot be if networked multiplayer is to be
  // implemented, as players can connect and disconnect at any time. We will
  // need to replace this with something else.
  PlayerData *player_data;
  size_t player_capacity;
  size_t player_count;
  Map map;
  KeyMap keymap;
} Game;

void game_init(Game *game);
void game_free(Game *game);

void game_update(Game *game);
void game_add_player(Game *game, Player player);

#endif // !SNAKE_GAME_H
