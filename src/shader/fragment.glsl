#version 460 core

const uint empty = 0;
const uint wall = 1;
const uint player = 2;
const uint powerup = 3;

in VsOut {
  flat uint cell_type;
} fs_in;

out vec4 FragColor;

void main() {
    switch (fs_in.cell_type) {
      case empty:
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        break;
      case wall:
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        break;
      case player:
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        break;
      default:
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        break;
    }
}
