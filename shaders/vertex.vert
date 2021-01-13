#version 330

const float POINT_RADIUS = 64.0;

uniform mat4 view;
uniform mat4 proj;

in vec3 position;
in vec3 color;
in int advect;

out vec4 vertex_color;

void main() {
    // vertex_color = color;
    if (advect != 0) {
        vertex_color = vec4(0.2, 0.2, 0.5, 0.9);
    } else {
        vertex_color = vec4(0.5, 0.5, 0.5, 0.4);
    }

    vec4 pos = proj * view * vec4(position, 1.0);

    gl_PointSize = POINT_RADIUS / pos.w;
    gl_Position = pos;
}
