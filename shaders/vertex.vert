#version 330

const float POINT_RADIUS = 64.0;

uniform mat4 view;
uniform mat4 proj;

uniform int render_boundary = 0; /*bool*/

in vec3 position;
in vec3 color;
in int advect;   /*bool*/
in int boundary; /*bool*/

out vec4 vertex_color;

void main() {
    // vertex_color = color;
    if (advect != 0) {
        // vertex_color = vec4(0.2, 0.2, 0.5, 0.9);
        vertex_color = vec4(color, 0.9);
    } else {
        // vertex_color = vec4(0.5, 0.5, 0.5, 0.4);
        if (boundary != 0 && render_boundary != 0) {
            vertex_color = vec4(color, 0.2);
        } else {
            vertex_color = vec4(0);
        }
    }

    vec4 pos = proj * view * vec4(position, 1.0);

    gl_PointSize = POINT_RADIUS / pos.w;
    gl_Position = pos;
}
