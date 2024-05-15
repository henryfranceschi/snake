#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "map.h"
#include "vec.h"

// Position must be a wrapped vector.
static inline size_t pos_to_index(const Map *map, Vec2I pos) {
  assert(pos.x < map->width);
  assert(pos.y < map->height);
  return pos.x + pos.y * map->width;
}

void map_init(Map *map) {
  map->width = 0;
  map->height = 0;
  map->cells = nullptr;
}

void map_free(Map *map) {
  free(map->cells);
  map_init(map);
}

void map_set_dimensions(Map *map, size_t width, size_t height) {
  map->width = width;
  map->height = height;
  Cell *cells = realloc(map->cells, width * height * sizeof(Cell));
  if (cells == nullptr) {
    report_error("failed to resize map allocation");
    exit(EXIT_FAILURE);
  }
  map->cells = cells;
}

void map_fill(Map *map, Cell cell) {
  for (int i = 0; i < map->width * map->height; ++i) {
    map->cells[i] = cell;
  }
}

Cell map_get_cell(const Map *map, Vec2I pos) {
  size_t index = pos_to_index(map, pos);
  return map->cells[index];
}

Cell map_set_cell(Map *map, Vec2I pos, Cell cell) {
  size_t index = pos_to_index(map, pos);
  Cell prev = map->cells[index];
  map->cells[index] = cell;
  return prev;
}

// Write player cells.
void map_player(Map *map, Player *player) {
  for (int i = 0; i < player->count; ++i) {
    PlayerSegment *segment = player_index(player, i);
    Cell cell = (Cell){CELL_PLAYER, {.player = {1}}};
    map_set_cell(map, segment->position, cell);
  }
}

// If a map is open at the edges, when a player exits the map on one side they
// reappear on the other side.
Vec2I map_wrap_pos(const Map *map, Vec2I pos) {
  int x = pos.x % map->width;
  if (x < 0)
    x = map->width + x;

  int y = pos.y % map->height;
  if (y < 0)
    y = map->height + y;

  return vec2i(x, y);
}

void map_debug(const Map *map) {
  for (int y = 0; y < map->height; ++y) {
    for (int x = 0; x < map->width; ++x) {
      Vec2I pos = vec2i(x, y);
      Cell cell = map_get_cell(map, pos);
      char symbol;
      switch (cell.type) {
      case CELL_EMPTY:
        symbol = 'e';
        break;
      case CELL_WALL:
        symbol = 'w';
        break;
      case CELL_PLAYER:
        symbol = cell.player.id + 48;
        break;
      case CELL_POWERUP:
        symbol = 'p';
        break;
      default:
        symbol = '?';
        break;
      }

      fprintf(stderr, "%c", symbol);
    }
    fprintf(stderr, "\n");
  }
}
