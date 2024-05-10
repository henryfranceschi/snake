#include "action.h"
#include "vec.h"

void action_init(Action *action) {
  action->type = ACTION_NONE;
}

Vec2I action_direction(Action action) {
  ActionType type = action.type;
  switch (type) {
  case ACTION_MOVE_UP:
    return VEC2I_UP;
  case ACTION_MOVE_DOWN:
    return VEC2I_DOWN;
  case ACTION_MOVE_LEFT:
    return VEC2I_LEFT;
  case ACTION_MOVE_RIGHT:
    return VEC2I_RIGHT;
  default:
    return VEC2I_ZERO;
  }
}
