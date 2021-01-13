#version 430

in vec2 position;

out vec2 pixel_position;

void main() {
    pixel_position = position;

    gl_Position = vec4(position, 0.0, 1.0);
}
