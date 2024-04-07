#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

char *read_to_string(const char *path) {
  FILE *f = fopen(path, "r");
  if (f == nullptr) {
    fprintf(stderr, "failed to open file: %s", path);
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  // fpos_t pos;
  // fgetpos(fp, &pos);
  size_t size = ftell(f);
  rewind(f);
  // fseek(f, 0, SEEK_SET);

  char *buffer = malloc(size + 1);
  fread(buffer, size, 1, f);
  buffer[size] = '\0';

  return buffer;
}

int min(int a, int b) {
  return a < b ? a : b;
}

int max(int a, int b) {
  return a > b ? a : b;
}

// Converts a position to a row major index.
size_t row_maj_index(size_t w, size_t x, size_t y) {
  return w * y + x;
}

Vec2I row_maj_position(size_t w, size_t i) {
  Vec2I v;
  v.x = i % w;
  v.y = i / w;
  return v;
}

// Converts a position to a column major index.
size_t col_maj_index(size_t h, size_t x, size_t y) {
  return h * x + y;
}

Vec2I col_maj_position(size_t h, size_t i) {
  Vec2I v;
  v.x = i / h;
  v.y = i % h;
  return v;
}
