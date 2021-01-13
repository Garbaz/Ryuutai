#version 330

const vec3 COLOR = vec3(0.9, 0.6, 0.8);

in vec3 vertex_color;

out vec4 out_color;

void main() {
    out_color = vec4(COLOR, 1.0);
}
