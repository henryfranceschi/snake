#include "draw.h"

static Vertex vertex(const Map *map, unsigned int i);

// TODO: Think about redesigning this. Currently we write one quad per cell no
// matter what. We continually update the buffer data, but the only things that
// change each frame are the dynamic cells. Another thing to think about is the
// fact that we don't actually need to submit each cell as a quad, we could
// instead create larger quads that contain multiple adjacent cells of the same
// type.
void write_vertices(const Map *map, Vertex *vertices) {
  for (int i = 0; i < 4 * map->width * map->height; ++i) {
    vertices[i] = vertex(map, i);
  }
}

void write_indices(const Map *map, unsigned int *indices) {
  for (int i = 0; i < map->width * map->height; ++i) {
    // there are 6 unsigned ints per cell.
    size_t index_offset = i * 6;
    // There are 4 vertices per cell.
    size_t vertex_offset = i * 4;
    // The vertices are at vertex_offset +
    // 2 - 3
    // |   |
    // 0 - 1

    // Top triangle.
    indices[index_offset + 0] = vertex_offset + 0;
    indices[index_offset + 1] = vertex_offset + 3;
    indices[index_offset + 2] = vertex_offset + 2;

    // Bottom triangle.
    indices[index_offset + 3] = vertex_offset + 0;
    indices[index_offset + 4] = vertex_offset + 1;
    indices[index_offset + 5] = vertex_offset + 3;
  }
}

static Vertex vertex(const Map *map, unsigned int i) {
  unsigned int cell_number = i / 4;
  unsigned int vertex_number = i % 4;

  unsigned int x = cell_number % map->width;
  unsigned int y = cell_number / map->width;
  CellType type = map_get_cell(map, vec2i(x, y)).type;

  Vec2I pos;
  switch (vertex_number) {
  case 0:
    // Bottom left.
    pos.x = x + 0;
    pos.y = y + 0;
    break;
  case 1:
    // Bottom right.
    pos.x = x + 1;
    pos.y = y + 0;
    break;
  case 2:
    // Top left.
    pos.x = x + 0;
    pos.y = y + 1;
    break;
  case 3:
    // Top right.
    pos.x = x + 1;
    pos.y = y + 1;
    break;
  }

  Vertex v;
  v.pos = pos;
  v.type = type;
  return v;
}
