#include "vec.h"

Vec2I vec2i(int x, int y) {
  Vec2I v;
  v.x = x;
  v.y = y;
  return v;
}

Vec2I vec2i_add(Vec2I a, Vec2I b) {
  return vec2i(a.x + b.x, a.y + b.y);
}

Vec2I vec2i_sub(Vec2I a, Vec2I b) {
  return vec2i(a.x - b.x, a.y - b.y);
}

Vec2I vec2i_negate(Vec2I a) {
  return vec2i(-a.x, -a.y );
}

bool vec2i_eq(Vec2I a, Vec2I b) {
  return a.x == b.x && a.y == b.y;
}

bool vec2i_ne(Vec2I a, Vec2I b) {
  return !vec2i_eq(a, b);
}

int vec2i_dot(Vec2I a, Vec2I b) {
  return a.x * b.x + a.y * b.y;
}
