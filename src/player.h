#ifndef SNAKE_PLAYER_H
#define SNAKE_PLAYER_H

#include <assert.h>
#include <stdio.h>

#include "vec.h"

// We just store the position of each segment as a Vec2I. As segments are
// interchangeable, when we move the player we just add a new segment to the
// front of the deque instead of updating the position of every segment,
// this will need to change if in the future we need to associate other data
// with each segment.
typedef struct {
  Vec2I position;
} PlayerSegment;

// Contains a list of a player's segments, implemented using a deque, because
// we require random access and fast inserts to the front and the back.
// If a player is alive it should always have at least two segments.
typedef struct {
  unsigned int id;
  bool alive;
  uint8_t queued_growth;

  PlayerSegment *segments;
  size_t capacity;
  size_t count;
  size_t head;
} Player;

void player_init(Player *player);
void player_free(Player *player);

void player_spawn(Player *player, PlayerSegment head, PlayerSegment tail);
void player_kill(Player *player);

PlayerSegment *player_index(const Player *player, size_t index);
PlayerSegment *player_front(const Player *player);
PlayerSegment *player_back(const Player *player);

void player_push_front(Player *player, PlayerSegment value);
void player_push_back(Player *player, PlayerSegment value);
PlayerSegment player_pop_front(Player *player);
PlayerSegment player_pop_back(Player *player);

#endif // !SNAKE_PLAYER_H
