#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in uint cell_type;

uniform mat4 matrix;

out VsOut {
  uint cell_type;
} vs_out;

void main() {
  vs_out.cell_type = cell_type;
  gl_Position = matrix * vec4(pos.x, pos.y, 0.0, 1.0);
}
