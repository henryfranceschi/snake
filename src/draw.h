#ifndef SNAKE_DRAW_H
#define SNAKE_DRAW_H

#include <stddef.h>
#include <stdlib.h>

#include "map.h"

typedef struct {
  Vec2I pos;
  unsigned int type;
} Vertex;

#define VERTEX_UNPADDED_SIZE sizeof(Vec2I) + sizeof(unsigned int)
#define VERTEX_IS_UNPADDED sizeof(Vertex) == VERTEX_UNPADDED_SIZE

void write_vertices(const Map *map, Vertex *vertices);
void write_indices(const Map *map, unsigned int *indices);

#endif // !SNAKE_DRAW_H
