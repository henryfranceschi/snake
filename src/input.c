#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "input.h"
#include "util.h"

static uint32_t hash(void *data, size_t size) {
  uint32_t hash = 2166136261;
  for (int i = 0; i < size; ++i) {
    hash = hash ^ ((char *)data)[i];
    hash = hash * 16777619;
  }

  return hash;
}

static inline bool entry_is_tombstone(Entry *entry) {
  return entry->keycode == ENTRY_KEYCODE_DELETED;
}

static inline bool entry_is_unused(Entry *entry) {
  return entry->keycode == ENTRY_KEYCODE_UNUSED;
}

static inline bool entry_is_used(Entry *entry) {
  return entry->keycode != ENTRY_KEYCODE_UNUSED && !entry_is_tombstone(entry);
}

static Entry *find_entry(Entry *entries, size_t capacity, uint16_t key) {
  const uint32_t key_hash = hash(&key, sizeof(uint16_t));
  const size_t ideal_index = key_hash % capacity;

  Entry *tombstone = nullptr;
  for (int i = 0; i < capacity; ++i) {
    Entry *entry = &entries[(ideal_index + i) % capacity];
    // We need to find the first entry that is equal to the key or either the
    // first tombstone or the first unused entry.
    if (entry->keycode == key) {
      return entry;
    }
    // We don't return here because it is possible that the entry we are
    // looking for is located after the tombstone.
    else if (entry_is_tombstone(entry)) {
      if (tombstone == nullptr) {
        tombstone = entry;
      }
    }
    // If the entry is empty, it means the key does not exist in the
    // map, if we encountered a tombstone during a previous iteration we
    // return that, otherwise we return the empty entry.
    else if (entry_is_unused(entry)) {
      return tombstone ? tombstone : entry;
    }
  }

  // The following line is unreachable if the array is resized correctly, and
  // the maximum load factor is less than 1. The only way it would be possible
  // to reach this line is if count were equal to capacity, the key were not
  // present in the map, and there were no unused or tombstone entries.
  unreachable();
}

static void resize(KeyMap *map) {
  size_t capacity = new_capacity(map->capacity);
  Entry *entries = malloc(capacity * sizeof(Entry));
  // Initialize all the entries in the new allocation.
  for (int i = 0; i < capacity; ++i) {
    Entry *entry = &entries[i];
    entry->keycode = ENTRY_KEYCODE_UNUSED;
  }

  int count = 0;
  for (int i = 0; i < map->capacity; ++i) {
    Entry *source = &map->entries[i];
    // Copy all the used entries into the new allocation.
    if (entry_is_used(source)) {
      Entry *destination = find_entry(entries, capacity, source->keycode);
      destination->keycode = source->keycode;
      destination->player_id = source->player_id;
      destination->action = source->action;
      ++count;
    }
  }
  free(map->entries);

  map->entries = entries;
  map->capacity = capacity;
  map->count = count;
}

void keymap_init(KeyMap *map) {
  map->entries = nullptr;
  map->capacity = 0;
  map->count = 0;
}

void keymap_free(KeyMap *map) {
  free(map->entries);
  keymap_init(map);
}

bool keymap_map(KeyMap *map, uint16_t keycode, unsigned int player_id, Action action) {
  if (map->count + 1 > map->capacity * KEYMAP_MAX_LOAD)
    resize(map);

  Entry *entry = find_entry(map->entries, map->capacity, keycode);
  bool empty = entry_is_unused(entry);
  if (empty) {
    entry->keycode = keycode;
    ++map->count;
  }

  entry->player_id = player_id;
  entry->action = action;

  return empty;
}

bool keymap_unmap(KeyMap *map, uint16_t keycode) {
  if (map->count == 0)
    return false;

  Entry *entry = find_entry(map->entries, map->capacity, keycode);
  bool exists = entry_is_used(entry);
  if (exists) {
    entry->keycode = ENTRY_KEYCODE_DELETED;
  }

  return exists;
}

// Returns true if the action exists and sets the contents of action to the
// value of the entry.
bool keymap_action(const KeyMap *map, uint16_t keycode, unsigned int *player_id, Action *action) {
  if (map->count == 0)
    return false;

  Entry *entry = find_entry(map->entries, map->capacity, keycode);
  bool exists = entry_is_used(entry);
  if (exists) {
    *player_id = entry->player_id;
    *action = entry->action;
  }

  return exists;
}
