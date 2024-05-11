#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "player.h"
#include "util.h"
#include "vec.h"

// Resize the allocation storing the player's segments.
static void resize(Player *player) {
  size_t capacity = new_capacity(player->capacity);
  PlayerSegment *segments =
    realloc(player->segments, capacity * sizeof(PlayerSegment));
  if (segments == nullptr) {
    fprintf(stderr, "failed to resize player allocation\n");
    exit(EXIT_FAILURE);
  }

  player->segments = segments;
  player->capacity = capacity;
}

static inline size_t wrap_index(const Player *player, size_t index) {
  return player->capacity > 0 ? index % player->capacity : 0;
}

// Maps a logical index to a physical index.
static inline size_t physical_index(const Player *player, size_t index) {
  assert(index < player->capacity);
  return wrap_index(player, player->head + index);
}

static bool is_adjacent(Vec2I a, Vec2I b) {
  // TODO: update this function to handle map wrap around.
  Vec2I v = vec2i_sub(a, b);
  // If the difference between a and b is at most one in either x or y but not
  // both the cell is adjacent.
  return (abs(v.x) == 1) ^ (abs(v.y) == 1);
}

void player_init(Player *player) {
  player->segments = nullptr;
  player->capacity = 0;
  player->count = 0;
  player->head = 0;
  player->alive = false;
  player->queued_growth = 0;
}

void player_free(Player *player) {
  free(player->segments);
  player_init(player);
}

// Should only be called on a dead player.
void player_spawn(Player *player, PlayerSegment head, PlayerSegment tail) {
  assert(!player->alive);
  assert(is_adjacent(head.position, tail.position));

  resize(player);
  player->segments[player->count++] = head;
  player->segments[player->count++] = tail;
  player->alive = true;
}

void player_kill(Player *player) {
  player_init(player);
}

PlayerSegment *player_index(const Player *player, size_t index) {
  return &player->segments[physical_index(player, index)];
}

// Peek the first segment of the player.
PlayerSegment *player_front(const Player *player) {
  return &player->segments[physical_index(player, 0)];
}

// Peek the last segment of the player.
PlayerSegment *player_back(const Player *player) {
  return &player->segments[physical_index(player, player->count - 1)];
}

void player_push_front(Player *player, PlayerSegment segment) {
  if (player->count == player->capacity)
    resize(player);

  // Make sure the new segment is adjacent to the current tail in one of
  // the four cardinal directions except behind the head.
  PlayerSegment *first = player_front(player);
  PlayerSegment *second = player_index(player, 1);
  if (!is_adjacent(segment.position, first->position) ||
      vec2i_eq(segment.position, second->position)) {
    fprintf(stderr, "new front segment not adjacent to old front\n");
    exit(EXIT_FAILURE);
  }

  player->segments[wrap_index(player, player->head - 1)] = segment;
  player->head--;
  player->count++;
}

void player_push_back(Player *player, PlayerSegment segment) {
  if (player->count == player->capacity)
    resize(player);

  PlayerSegment *last = player_front(player);
  PlayerSegment *second_to_last = player_index(player, player->count - 1);
  if (!is_adjacent(segment.position, last->position) ||
      vec2i_eq(segment.position, second_to_last->position)) {
    fprintf(stderr, "new back segment not adjacent to old back\n");
    exit(EXIT_FAILURE);
  }

  player->segments[wrap_index(player, player->head + player->count)] = segment;
  player->count++;
}

PlayerSegment player_pop_front(Player *player) {
  PlayerSegment segment = player->segments[physical_index(player, 0)];
  ++player->head;
  --player->count;
  return segment;
}

PlayerSegment player_pop_back(Player *player) {
  PlayerSegment segment =
    player->segments[physical_index(player, player->count - 1)];
  --player->count;
  return segment;
}
