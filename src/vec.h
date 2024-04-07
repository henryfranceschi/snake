#ifndef SNAKE_VEC_H
#define SNAKE_VEC_H

#include <math.h>
#include <stdint.h>

// Represents a point or direction in 2D space.
typedef struct {
  union {
    struct {
      int x;
      int y;
    };
    int c[2];
  };
} Vec2I;

#define VEC2I_ELEMENT_TYPE int
#define VEC2I_UNPADDED_SIZE 2 * sizeof(int)
#define VEC2I_IS_UNPADDED sizeof(Vec2I) == VEC2I_UNPADDED_SIZE

Vec2I vec2i(int x, int y);

Vec2I vec2i_add(Vec2I a, Vec2I b);
Vec2I vec2i_sub(Vec2I a, Vec2I b);
Vec2I vec2i_negate(Vec2I a);

int vec2i_dot(Vec2I a, Vec2I b);

bool vec2i_eq(Vec2I a, Vec2I b);
bool vec2i_ne(Vec2I a, Vec2I b);

// It should be noted here that the 'corner' values are not normalised.
#define VEC2I_UP          (Vec2I) { .x =  0, .y =  1 }
#define VEC2I_DOWN        (Vec2I) { .x =  0, .y = -1 }
#define VEC2I_LEFT        (Vec2I) { .x = -1, .y =  0 }
#define VEC2I_RIGHT       (Vec2I) { .x =  1, .y =  0 }
#define VEC2I_UP_LEFT     (Vec2I) { .x = -1, .y =  1 }
#define VEC2I_UP_RIGHT    (Vec2I) { .x =  1, .y =  1 }
#define VEC2I_DOWN_LEFT   (Vec2I) { .x = -1, .y = -1 }
#define VEC2I_DOWN_RIGHT  (Vec2I) { .x =  1, .y = -1 }
#define VEC2I_ZERO        (Vec2I) { .x =  0, .y =  0 }

#endif // !SNAKE_VEC_H
