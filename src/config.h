#ifndef SNAKE_ARG_H
#define SNAKE_ARG_H

#include <stddef.h>
#include <assert.h>

typedef struct {
  // The index of the chunk.
  unsigned int index;
  // The length of the chunk.
  unsigned int len;
  // The position in the current chunk.
  unsigned int pos;
} Cursor;

typedef struct {
  const char **chunks;
  const size_t chunk_count;
  size_t cursor;
} ParseContext;

typedef struct {
  // Has a default value of 1.
  // Must be greater than or equal to 1.
  unsigned int player_count;
} Config;

void config_init(Config *config);
bool config_from_args(Config *config, int argc, const char **argv);

#endif // !SNAKE_ARG_H
