#ifndef SNAKE_ACTION_H
#define SNAKE_ACTION_H

#include <stdint.h>

#include "vec.h"

// Current possible actions are all mutually exclusive.
typedef enum {
  ACTION_NONE,
  ACTION_MOVE_UP,
  ACTION_MOVE_DOWN,
  ACTION_MOVE_LEFT,
  ACTION_MOVE_RIGHT,
} ActionType;

// Represents an action a player has taken.
typedef struct {
  uint8_t type;
} Action;

void action_init(Action *action);
Vec2I action_direction(Action action);

#endif // !SNAKE_ACTION_H
