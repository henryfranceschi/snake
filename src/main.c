#include <stddef.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include "config.h"
#include "error.h"
#include "game.h"
#include "geometry.h"
#include "input.h"
#include "map.h"
#include "player.h"
#include "util.h"
#include "vec.h"

unsigned int create_shader(const char *source, GLenum type);
unsigned int create_program(GLuint vs, GLuint fs);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void draw(const Geometry *geometry);

int main(int argc, const char** argv) {
  Config config;
  config_init(&config);
  if (!config_from_args(&config, argc, argv)) {
    return EXIT_FAILURE;
  }

  if (!glfwInit()) {
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // TODO: handle window resizing.
  GLFWwindow *window =
    glfwCreateWindow(512, 512, "Hello Square", nullptr, nullptr);
  if (!window) {
    report_error("failed to create window");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  char *vss = read_to_string("src/shader/vertex.glsl");
  if (!vss) {
    report_error("failed to open vertex shader source");
    free(vss);
    return EXIT_FAILURE;
  }

  char *fss = read_to_string("src/shader/fragment.glsl");
  if (!fss) {
    report_error("failed to open fragment shader source");
    free(fss);
    return EXIT_FAILURE;
  }

  const unsigned int vs = create_shader(vss, GL_VERTEX_SHADER);
  free(vss);
  if (vs == 0) {
    return EXIT_FAILURE;
  }

  const unsigned int fs = create_shader(fss, GL_FRAGMENT_SHADER);
  free(fss);
  if (fs == 0) {
    return EXIT_FAILURE;
  }

  const unsigned int program = create_program(vs, fs);
  if (program == 0) {
    return EXIT_FAILURE;
  }

  glUseProgram(program);

  glfwSetKeyCallback(window, key_callback);

  Game game;
  game_init(&game);
  {
    Map map;
    // TODO: Add support for loading maps from file.
    map_init(&map);
    map_set_dimensions(&map, 32, 32);
    map_fill(&map, (Cell){CELL_EMPTY});

    // Set vertical walls.
    for (int i = 0; i < map.height; ++i) {
      const Cell cell = {CELL_WALL};
      map_set_cell(&map, vec2i(0, i), cell);
      map_set_cell(&map, vec2i(map.width - 1, i), cell);
    }

    // Set horizontal walls.
    for (int i = 0; i < map.width; ++i) {
      const Cell cell = {CELL_WALL};
      map_set_cell(&map, vec2i(i, 0), cell);
      map_set_cell(&map, vec2i(i, map.height - 1), cell);
    }

    // TODO: Add support for multiple players.
    // TODO: Determine appropriate starting position for players.
    for (int i = 0; i < config.player_count; ++i) {
      Player player;
      player_init(&player);
      player.id = i;

      const int x_offset = (map.width / (config.player_count + 1)) * (i + 1);
      const int y_offset = map.height / 2;
      const PlayerSegment first = {vec2i(x_offset, y_offset)};
      const PlayerSegment second = {vec2i(x_offset, y_offset + 1)};
      player_spawn(&player, first, second);
      map_player(&map, &player);
      game_add_player(&game, player);
    }

    // TODO: Add a proper system for spawning powerups.
    Cell power_up = {CELL_POWERUP, {.powerup = {5}}};
    map_set_cell(&map, vec2i(4, 4), power_up);

    game.map = map;

    Player player = game.player_data[0].player;
    KeyMap keymap;
    keymap_init(&keymap);
    keymap_map(&keymap, GLFW_KEY_UP, player.id, (Action){ACTION_MOVE_UP});
    keymap_map(&keymap, GLFW_KEY_DOWN, player.id, (Action){ACTION_MOVE_DOWN});
    keymap_map(&keymap, GLFW_KEY_LEFT, player.id, (Action){ACTION_MOVE_LEFT});
    keymap_map(&keymap, GLFW_KEY_RIGHT, player.id, (Action){ACTION_MOVE_RIGHT});

    game.keymap = keymap;
  }

  glfwSetWindowUserPointer(window, &game);

  // Here we construct a matrix to fit the map into the viewport.
  float scale = 2.0 / max(game.map.width, game.map.height);
  float trans_x = -scale * game.map.width / 2;
  float trans_y = -scale * game.map.height / 2;
  float matrix[4][4] = {
      {scale, 0.0, 0.0, trans_x},
      {0.0, scale, 0.0, trans_y},
      {0.0, 0.0, 1.0, 0.0},
      {0.0, 0.0, 0.0, 1.0},
  };

  unsigned int matrix_location = glGetUniformLocation(program, "matrix");
  glUniformMatrix4fv(matrix_location, 1, GL_TRUE, &matrix[0][0]);

  Geometry geometry;
  geometry_init(&geometry);
  geometry_from_map(&geometry, &game.map);

  double update_limit = 1.0 / 8;
  double last_update_time = 0.0;
  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();

    glfwPollEvents();
    if (current_time - last_update_time >= update_limit) {
      game_update(&game);
      for (int i = 0; i < game.player_count; ++i) {
        map_player(&game.map, &game.player_data[i].player);
      }
      geometry_from_map(&geometry, &game.map);

      draw(&geometry);
      glfwSwapBuffers(window);
      last_update_time = current_time;
    }
  }

  glfwTerminate();
  geometry_free(&geometry);
  game_free(&game);

  return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  // TODO: the player can obviously press multiple keys at once, in cases where
  // the user is pressing at most two keys that have at most a 90 degree
  // difference between them, we should alternate between moving the two
  // directions.
  Game *game = glfwGetWindowUserPointer(window);
  unsigned int player_id;
  Action act;
  if (action == GLFW_PRESS && keymap_action(&game->keymap, key, &player_id, &act)) {
    PlayerData *player_data = &game->player_data[player_id];
    player_data->current_action = act;
  }
}

void draw(const Geometry *geometry) {
  glBindVertexArray(geometry->handle);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glDrawElements(GL_TRIANGLES, geometry->indices.datum_count, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(GL_NONE);
}


unsigned int create_shader(const char *source, GLenum type) {
  unsigned int handle = glCreateShader(type);
  glShaderSource(handle, 1, &source, nullptr);
  glCompileShader(handle);

  int success;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

  if (!success) {
    int size = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &size);

    char *log = malloc(size);
    glGetShaderInfoLog(handle, size, nullptr, log);
    report_error("failed to compile program:\n%s", log);
    free(log);

    return 0;
  }

  return handle;
}

unsigned int create_program(GLuint vs, GLuint fs) {
  unsigned int handle = glCreateProgram();
  glAttachShader(handle, vs);
  glAttachShader(handle, fs);
  glLinkProgram(handle);

  glDetachShader(handle, vs);
  glDetachShader(handle, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  int success;
  glGetProgramiv(handle, GL_LINK_STATUS, &success);

  if (!success) {
    int size = 0;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &size);

    char *log = malloc(size * sizeof(char));
    glGetProgramInfoLog(handle, size, nullptr, log);
    report_error("failed to link program:\n%s", log);

    return 0;
  }

  return handle;
}
