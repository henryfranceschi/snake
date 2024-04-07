#ifndef SNAKE_MAP_H
#define SNAKE_MAP_H

#include <stddef.h>
#include <stdint.h>

#include "player.h"
#include "vec.h"

typedef enum {
  CELL_EMPTY,
  CELL_WALL,
  CELL_PLAYER,
  CELL_POWERUP,
} CellType;

typedef struct {
  // The id of the player that is occupying the cell.
  uint8_t id;
} PlayerCell;

typedef struct {
  // The number of new cells that will be added to the player upon pickup.
  uint8_t power;
} PowerUpCell;

typedef struct {
  CellType type;
  // This should only ever be read from if type is equal to either CELL_PLAYER
  // or CELL_POWERUP.
  union {
    PlayerCell player;
    PowerUpCell powerup;
  };
} Cell;

typedef struct {
  unsigned int width;
  unsigned int height;
  Cell *cells;
} Map;

void map_init(Map *map);
void map_free(Map *map);

void map_set_dimensions(Map *map, size_t width, size_t height);
void map_fill(Map *map, Cell cell);
Cell map_get_cell(const Map *map, Vec2I pos);
Cell map_set_cell(Map *map, Vec2I pos, Cell cell);
void map_player(Map *map, Player *player);

Vec2I map_wrap_pos(const Map *map, Vec2I pos);

void map_debug(const Map *map);

#endif // !SNAKE_MAP_H
