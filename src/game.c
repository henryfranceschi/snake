#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "map.h"
#include "player.h"
#include "util.h"
#include "vec.h"

void game_init(Game *game) {
  game->players = nullptr;
  game->players_capacity = 0;
  game->players_count = 0;
  map_init(&game->map);
}

void game_free(Game *game) {
  for (int i = 0; i < game->players_count; ++i) {
    player_free(&game->players[i]);
  }
  free(game->players);
  map_free(&game->map);
  game_init(game);
}

void game_add_player(Game *game, Player player) {
  if (game->players_count == game->players_capacity) {
    size_t capacity = new_capacity(game->players_capacity);
    Player *players = realloc(game->players, capacity * sizeof(Player));
    if (players == nullptr) {
      fprintf(stderr, "failed to resize game players allocation");
      exit(EXIT_FAILURE);
    }

    game->players = players;
    game->players_capacity = capacity;
  }

  game->players[game->players_count++] = player;
}

void game_update(Game *game, Vec2I direction) {
  for (int i = 0; i < game->players_count; ++i) {
    Player *player = &game->players[i];

    if (!player->alive)
      return;

    // Player's invariants guarantee that segments are adjacent to each other in
    // one of the four cardinal directions, meaning the distance between any
    // cell n and cell n + 1 is always equal to 1, and the difference between
    // them is a normalised vector, representing in this case the direction the
    // player is facing.
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
    // really fair, and should be fixed later. A better system would be to move
    // all players, keeping track of their new positions, checking whether there
    // was a colilision in any of the updated cells, and then killing the
    // appropriate players.
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
  }
}
