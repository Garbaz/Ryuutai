#version 330

const float RIM_WIDTH = 0.05;

in vec4 vertex_color;

out vec4 out_color;

void main() {
    // out_color = vec4(gl_PointCoord.x,0.0,gl_PointCoord.y,1.0);
    float l = length(gl_PointCoord - vec2(0.5));
    if (l > 0.5 || vertex_color.a == 0) {
        discard;
    }
    if (l > 0.5 - RIM_WIDTH) {
        out_color = vec4(0.0, 0.0, 0.0, vertex_color.a);
    } else {
        out_color = vertex_color;
    }
}
