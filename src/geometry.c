#include <stdlib.h>

#include <glad/gl.h>

#include "error.h"
#include "geometry.h"

static unsigned int buffer_type_target(BufferType type) {
  switch (type) {
  case BUFFER_ARRAY:
    return GL_ARRAY_BUFFER;
  case BUFFER_ELEMENT:
    return GL_ELEMENT_ARRAY_BUFFER;
  }
}

static void buffer_init(Buffer *buffer, BufferType type, size_t datum_size) {
  buffer->type = type;
  buffer->data = nullptr;
  buffer->datum_size = datum_size;
  buffer->datum_count = 0;
  buffer->handle = 0;
}

static void buffer_free(Buffer *buffer) {
  free(buffer->data);
  glDeleteBuffers(1, &buffer->handle);
  buffer_init(buffer, buffer->type, buffer->datum_size);
}

static void buffer_set_length(Buffer *buffer, size_t length) {
  if (buffer->datum_count == length)
    return;

  if (length == 0) {
    free(buffer->data);
    buffer->data = nullptr;
  } else {
    void *data = realloc(buffer->data, length * buffer->datum_size);
    if (data == nullptr) {
      report_error("failed to reallocate buffer");
      exit(EXIT_FAILURE);
    }
    buffer->data = data;
  }

  buffer->datum_count = length;
}

// Should not be called unless we have already created and bound a vertex array.
static void buffer_sync(Buffer *buffer) {
  if (buffer->handle == 0) {
    glCreateBuffers(1, &buffer->handle);
  }

  int target = buffer_type_target(buffer->type);
  glBindBuffer(target, buffer->handle);

  if (target == GL_ARRAY_BUFFER) {
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void *)offsetof(Vertex, type));
    glEnableVertexAttribArray(1);
  }

  size_t buffer_size = buffer->datum_count * buffer->datum_size;
  int gpu_buffer_size;
  glGetBufferParameteriv(target, GL_BUFFER_SIZE, &gpu_buffer_size);
  // We could just check whether the buffer is large enough to hold the data,
  // the issue is that if the buffer on the cpu side shrinks then we will hold
  // on to some extra memory that we don't need.
  if (gpu_buffer_size != buffer_size) {
    glBufferData(target, buffer_size, buffer->data, GL_STATIC_DRAW);
  } else {
    glBufferSubData(target, 0, buffer_size, buffer->data);
  }
}

static size_t element_vertex_count(GeometryType type) {
  switch (type) {
  case GEOMETRY_POINTS:
    return 1;
  case GEOMETRY_LINES:
    return 2;
  case GEOMETRY_TRIANGLES:
    return 3;
  }
}

void geometry_init(Geometry *geometry) {
  geometry->type = GEOMETRY_TRIANGLES;
  buffer_init(&geometry->vertices, BUFFER_ARRAY, sizeof(Vertex));
  buffer_init(&geometry->indices, BUFFER_ELEMENT, sizeof(unsigned int));
  geometry->handle = 0;
}

void geometry_free(Geometry *geometry) {
  buffer_free(&geometry->vertices);
  buffer_free(&geometry->indices);
  glDeleteVertexArrays(1, &geometry->handle);
  geometry_init(geometry);
}

static Vertex vertex(const Map *map, unsigned int i);
void write_vertices(const Map *map, Vertex *vertices);
void write_indices(const Map *map, unsigned int *indices);

void geometry_sync(Geometry *geometry, bool resized) {
  if (geometry->handle == 0) {
    glCreateVertexArrays(1, &geometry->handle);
  }

  glBindVertexArray(geometry->handle);

  buffer_sync(&geometry->vertices);
  if (resized) {
    buffer_sync(&geometry->indices);
  }

  glBindVertexArray(GL_NONE);
}

// Currently this is the only correct way to make geometry.
void geometry_from_map(Geometry *geometry, const Map *map) {
  unsigned prev_cell_count = geometry->vertices.datum_count / 4;
  unsigned cell_count = map->width * map->height;
  bool need_resize = cell_count != prev_cell_count;
  // Resize the buffers if the size of the map has changed or the buffers are empty.
  if (need_resize) {
    // 4 vertices per cell.
    buffer_set_length(&geometry->vertices, 4 * cell_count);
    // 2 triangles per cell.
    buffer_set_length(&geometry->indices, 6 * cell_count);
  }

  // Update the contents of the buffers.
  write_vertices(map, geometry->vertices.data);
  if (need_resize)
    write_indices(map, geometry->indices.data);

  geometry_sync(geometry, need_resize);
}

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
