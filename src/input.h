#ifndef SNAKE_INPUT_H
#define SNAKE_INPUT_H

#include <stddef.h>
#include <stdint.h>

#include "action.h"

typedef struct {
  // We use a sentinal value of 0 to indicate that the enty is unused.
  // We use a sentinal value of UINT16_MAX to indicate that the entry was
  // deleted.
  uint16_t keycode;
  unsigned int player_id;
  Action action;
} Entry;

#define ENTRY_KEYCODE_UNUSED 0
#define ENTRY_KEYCODE_DELETED UINT16_MAX

// We need to map key presses to player actions.
// This is implemented with a hashmap.
typedef struct {
  Entry *entries;
  size_t capacity;
  size_t count;
} KeyMap;

#define KEYMAP_MAX_LOAD 0.75

void keymap_init(KeyMap *map);
void keymap_free(KeyMap *map);

bool keymap_map(KeyMap *map, uint16_t keycode, unsigned int player_id, Action action);
bool keymap_unmap(KeyMap *map, uint16_t keycode);
bool keymap_action(const KeyMap *map, uint16_t keycode, unsigned int *player_id, Action *action);

#endif // !SNAKE_INPUT_H
