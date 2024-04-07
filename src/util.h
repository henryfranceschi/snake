#ifndef SNAKE_UTIL_H
#define SNAKE_UTIL_H

#include <stddef.h>

#include "vec.h"

char *read_to_string(const char *path);

int min(int a, int b);
int max(int a, int b);

size_t row_maj_index(size_t w, size_t x, size_t y);
Vec2I row_maj_position(size_t w, size_t i);
size_t col_maj_index(size_t h, size_t x, size_t y);
Vec2I col_maj_position(size_t h, size_t i);

#define DBG_EXP(x)                                                             \
  printf(_Generic((x),                                                         \
         char *: "%s = %s\n",                                                  \
         char: "%s = %i\n",                                                    \
         short: "%s = %i\n",                                                   \
         int: "%s = %i\n",                                                     \
         long: "%s = %i\n",                                                    \
         long long: "%s = %i\n",                                               \
         float: "%s = %f\n",                                                   \
         double: "%s = %f\n",                                                  \
         long double: "%s = %f\n",                                             \
         unsigned char: "%s = %u\n",                                           \
         unsigned short: "%s = %u\n",                                          \
         unsigned int: "%s = %u\n",                                            \
         unsigned long: "%s = %u\n",                                           \
         unsigned long long: "%s = %u\n"),                                     \
         #x, x)

#endif // !SNAKE_UTIL_H
