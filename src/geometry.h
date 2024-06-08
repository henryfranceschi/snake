#ifndef SNAKE_DRAW_H
#define SNAKE_DRAW_H

#include "map.h"

typedef enum {
  BUFFER_ARRAY,
  BUFFER_ELEMENT,
} BufferType;

typedef struct {
  BufferType type;
  void *data;
  size_t datum_size;
  size_t datum_count;
  unsigned int handle;
} Buffer;

typedef enum {
  GEOMETRY_POINTS,
  GEOMETRY_LINES,
  GEOMETRY_TRIANGLES,
} GeometryType;

typedef struct {
  Vec2I pos;
  unsigned int type;
} Vertex;

#define VERTEX_UNPADDED_SIZE sizeof(Vec2I) + sizeof(unsigned int)
#define VERTEX_IS_UNPADDED sizeof(Vertex) == VERTEX_UNPADDED_SIZE

typedef struct {
  GeometryType type;
  Buffer vertices;
  Buffer indices;
  // Vertex attribute handle.
  unsigned int handle;
} Geometry;

void geometry_init(Geometry *geometry);
void geometry_free(Geometry *geometry);
void geometry_from_map(Geometry *geometry, const Map *map);

#endif // !SNAKE_DRAW_H
