#include <stdio.h>
#include <stdlib.h>

#include "action.h"
#include "game.h"
#include "input.h"
#include "map.h"
#include "player.h"
#include "util.h"
#include "vec.h"

void player_data_init(PlayerData *player_data) {
  player_init(&player_data->player);
  action_init(&player_data->current_action);
  action_init(&player_data->previous_action);
}

void player_data_free(PlayerData *player_data) {
  player_free(&player_data->player);
  player_data_init(player_data);
}

void player_data_set_action(PlayerData *player_data, Action action) {
  player_data->previous_action = player_data->current_action;
  player_data->current_action = action;
}

void game_init(Game *game) {
  game->player_data = nullptr;
  game->player_capacity = 0;
  game->player_count = 0;
  map_init(&game->map);
  keymap_init(&game->keymap);
}

void game_free(Game *game) {
  for (int i = 0; i < game->player_count; ++i) {
    player_data_free(&game->player_data[i]);
  }
  map_free(&game->map);
  keymap_free(&game->keymap);
  game_init(game);
}

void game_add_player(Game *game, Player player) {
  if (game->player_count == game->player_capacity) {
    size_t capacity = new_capacity(game->player_capacity);
    PlayerData *player_data =
        realloc(game->player_data, capacity * sizeof(PlayerData));
    if (player_data == nullptr) {
      fprintf(stderr, "failed to resize game player_data allocation\n");
      exit(EXIT_FAILURE);
    }

    game->player_data = player_data;
    game->player_capacity = capacity;
  }

  PlayerData *player_data = &game->player_data[game->player_count++];
  player_data_init(player_data);
  player_data->player = player;
}

void game_update(Game *game) {
  for (int i = 0; i < game->player_count; ++i) {
    Player *player = &game->player_data[i].player;
    Action *action = &game->player_data[i].current_action;

    if (!player->alive)
      return;

    Vec2I direction = action_direction(*action);

    // Player's invariants guarantee that segments are adjacent to each other
    // in one of the four cardinal directions, meaning the distance between
    // any cell n and cell n + 1 is always equal to 1, and the difference
    // between them is a normalised vector, representing in this case the
    // direction the player is facing.
    PlayerSegment *first = player_front(player);
    PlayerSegment *second = player_index(player, 1);
    Vec2I forward = vec2i_sub(first->position, second->position);

    if (vec2i_eq(direction, VEC2I_ZERO)) {
      direction = forward;
    }

    // If the player is not attempting to turn, ignore the input.
    if (vec2i_dot(forward, direction) != 0) {
      direction = forward;
    }

    // Figure out which cell the player's new segment is in, and check for
    // collisions.
    PlayerSegment new_head = {
        map_wrap_pos(&game->map, vec2i_add(first->position, direction))};
    player_push_front(player, new_head);
    // Check if the player picked up a power-up last update. If so, we do not
    // remove the player's last segment.
    if (player->queued_growth > 0) {
      --player->queued_growth;
    } else {
      PlayerSegment segment = player_pop_back(player);
      Cell cell = {CELL_EMPTY};
      map_set_cell(&game->map, segment.position, cell);
    }

    // As it currently works, if two players are attempting to occupy a cell,
    // the one that is updated first will live and the other die. This isn't
    // really fair, and should be fixed later. A better system would be to
    // move all players, keeping track of their new positions, checking
    // whether there was a colilision in any of the updated cells, and then
    // killing the appropriate players.
    Cell head_cell = map_get_cell(&game->map, new_head.position);
    switch (head_cell.type) {
    case CELL_WALL:
    case CELL_PLAYER: {
      player->alive = false;
      player_init(player);
    } break;
    case CELL_POWERUP: {
      PowerUpCell cell = head_cell.powerup;
      player->queued_growth += cell.power;
    } break;
    case CELL_EMPTY:
      break;
    }

    // Clear action.
    // game->actions[player->id].type = 0;
  }
}
