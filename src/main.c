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

GLFWwindow *create_window(const Config *config);
unsigned int create_shader(const char *source, GLenum type);
unsigned int create_program(const char *vs_path, const char *fs_path);
Map create_map(const Config *config);
Game create_game(const Config *config);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

typedef struct {
  GLFWwindow *window;
  unsigned int program;
  Game game;
  Geometry geometry;
} Application;

void setup(Application *app, const Config *config);
void run(Application *app);
void update(Application *app);
void draw(const Application *app);
void cleanup(Application *app);

int main(int argc, const char **argv) {
  Config config;
  config_init(&config);
  if (!config_from_args(&config, argc, argv)) {
    return EXIT_FAILURE;
  }

  Application app;
  setup(&app, &config);
  run(&app);
  cleanup(&app);

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
  if (action == GLFW_PRESS &&
      keymap_action(&game->keymap, key, &player_id, &act)) {
    PlayerData *player_data = &game->player_data[player_id];
    player_data->current_action = act;
  }
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

unsigned int create_program(const char *vs_path, const char *fs_path) {
  char *vss = read_to_string(vs_path);
  if (!vss) {
    report_error("failed to open vertex shader source");
    free(vss);
    exit(EXIT_FAILURE);
  }

  char *fss = read_to_string(fs_path);
  if (!fss) {
    report_error("failed to open fragment shader source");
    free(fss);
    exit(EXIT_FAILURE);
  }

  const unsigned int vs = create_shader(vss, GL_VERTEX_SHADER);
  free(vss);
  if (vs == 0) {
    exit(EXIT_FAILURE);
  }

  const unsigned int fs = create_shader(fss, GL_FRAGMENT_SHADER);
  free(fss);
  if (fs == 0) {
    exit(EXIT_FAILURE);
  }

  unsigned int program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  glDetachShader(program, vs);
  glDetachShader(program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (!success) {
    int size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);

    char *log = malloc(size * sizeof(char));
    glGetProgramInfoLog(program, size, nullptr, log);

    report_error("failed to link program:\n%s", log);
    exit(EXIT_FAILURE);
  }

  return program;
}

Map create_map(const Config *config) {
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

  return map;
}

Game create_game(const Config *config) {
  Game game;
  game_init(&game);
  game.map = create_map(config);

  // TODO: Add support for multiple players.
  // TODO: Determine appropriate starting position for players.
  for (int i = 0; i < config->player_count; ++i) {
    Player player;
    player_init(&player);
    player.id = i;

    const int x_offset = (game.map.width / (config->player_count + 1)) * (i + 1);
    const int y_offset = game.map.height / 2;
    const PlayerSegment first = {vec2i(x_offset, y_offset)};
    const PlayerSegment second = {vec2i(x_offset, y_offset + 1)};
    player_spawn(&player, first, second);
    map_player(&game.map, &player);
    game_add_player(&game, player);
  }

  // TODO: Add a proper system for spawning powerups.
  Cell power_up = {CELL_POWERUP, {.powerup = {5}}};
  map_set_cell(&game.map, vec2i(4, 4), power_up);

  Player player = game.player_data[0].player;
  KeyMap keymap;
  keymap_init(&keymap);
  keymap_map(&keymap, GLFW_KEY_UP, player.id, (Action){ACTION_MOVE_UP});
  keymap_map(&keymap, GLFW_KEY_DOWN, player.id, (Action){ACTION_MOVE_DOWN});
  keymap_map(&keymap, GLFW_KEY_LEFT, player.id, (Action){ACTION_MOVE_LEFT});
  keymap_map(&keymap, GLFW_KEY_RIGHT, player.id, (Action){ACTION_MOVE_RIGHT});

  game.keymap = keymap;

  return game;
}

void setup(Application *app, const Config *config) {
  // Setup window.
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
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
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  glfwSetKeyCallback(window, key_callback);

  app->window = window;

  // Setup shaders.
  app->program = create_program("src/shader/vertex.glsl", "src/shader/fragment.glsl");
  glUseProgram(app->program);

  app->game = create_game(config);

  glfwSetWindowUserPointer(app->window, &app->game);

  // Here we construct a matrix to fit the map into the viewport.
  float scale = 2.0 / max(app->game.map.width, app->game.map.height);
  float trans_x = -scale * app->game.map.width / 2;
  float trans_y = -scale * app->game.map.height / 2;
  float matrix[4][4] = {
      {scale, 0.0, 0.0, trans_x},
      {0.0, scale, 0.0, trans_y},
      {0.0, 0.0, 1.0, 0.0},
      {0.0, 0.0, 0.0, 1.0},
  };

  unsigned int matrix_location = glGetUniformLocation(app->program, "matrix");
  glUniformMatrix4fv(matrix_location, 1, GL_TRUE, &matrix[0][0]);

  Geometry geometry;
  geometry_init(&geometry);
  geometry_from_map(&geometry, &app->game.map);

  app->geometry = geometry;
}

void run(Application *app) {
  double update_limit = 1.0 / 8;
  double last_update_time = 0.0;
  while (!glfwWindowShouldClose(app->window)) {
    double current_time = glfwGetTime();

    glfwPollEvents();
    if (current_time - last_update_time >= update_limit) {
      update(app);
      draw(app);

      last_update_time = current_time;
    }
  }
}

void update(Application *app) {
  game_update(&app->game);
  for (int i = 0; i < app->game.player_count; ++i) {
    map_player(&app->game.map, &app->game.player_data[i].player);
  }
  geometry_from_map(&app->geometry, &app->game.map);
}

void draw(const Application *app) {
  glBindVertexArray(app->geometry.handle);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glDrawElements(GL_TRIANGLES, app->geometry.indices.datum_count, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(GL_NONE);

  glfwSwapBuffers(app->window);
}


void cleanup(Application *app) {
  geometry_free(&app->geometry);
  game_free(&app->game);
  glDeleteProgram(app->program);
  glfwTerminate();
}
