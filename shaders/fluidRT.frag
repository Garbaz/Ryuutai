#version 430

uniform float screen_ratio;
uniform float field_of_view_tan;

const float VIEW_DEPTH = 10;
const float DENSITY_SCALE = 100.0;
const vec3 WATER_COLOR = vec3(0,0.3,1.0);

uniform float step_size;
uniform uvec3 ssubdiv_grid_size;
uniform vec3 simulation_domain_origin;
uniform float kernel_h;
uniform float kernel_alpha;

uniform vec3 camera_position;
uniform vec3 camera_direction;

in vec2 pixel_position;
out vec4 out_color;

struct Particle {
    // Initialized data
    vec3 pos;
    vec3 vel;
    vec3 acc;
    float mass;
    uint advect;  // Used as boolean
    // Simulation data
    float density;
    float pressure;
};

layout(std430, binding = 3) readonly buffer _BUF_particles {
    Particle particles[];
};

layout(std430, binding = 4) readonly buffer _BUF_ssubdiv_C {
    uint ssubdiv_C[];
};

layout(std430, binding = 5) readonly buffer _BUF_ssubdiv_L {
    uint ssubdiv_L[];
};

uint sf_curve(ivec3 klm) {
    return uint(mod(klm.x, ssubdiv_grid_size.x) +
                mod(klm.y, ssubdiv_grid_size.y) * ssubdiv_grid_size.x +
                mod(klm.z, ssubdiv_grid_size.z) * ssubdiv_grid_size.x * ssubdiv_grid_size.y);
}
ivec3 sf_cell(vec3 p) {
    return ivec3((p - simulation_domain_origin) / (2.0 * kernel_h));
}
uint sf_index(vec3 p) {
    ivec3 klm = sf_cell(p);
    return sf_curve(klm);
}

float kernel(vec3 xi, vec3 xj) {
    vec3 d = (xi - xj) / kernel_h;
    float q = length(d);
    float t1 = max(1.0 - q, 0.0f);
    float t2 = max(2.0 - q, 0.0f);
    return kernel_alpha * (t2 * t2 * t2 - 4.0 * t1 * t1 * t1);
}

float density(vec3 pos) {
    float s = 0;

    uvec3 cell = sf_cell(pos);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                uint index = sf_curve(ivec3(cell.x - dx, cell.y - dy, cell.z - dz));
                uint c = ssubdiv_C[index];
                uint end = ssubdiv_C[index + 1];
                while(c < end) {
                    uint pi = ssubdiv_L[c];
                    s += particles[pi].mass * kernel(pos, particles[pi].pos);
                    c++;
                }
            }
        }
    }
    return s;
}

void main() {
    vec2 camera_offset = 2.0 * field_of_view_tan * vec2(pixel_position.x, screen_ratio * -pixel_position.y);
    vec3 offset_dir_horizontal = normalize(vec3(-camera_direction.z, 0.0, camera_direction.x));
    vec3 offset_dir_vertical = cross(camera_direction, offset_dir_horizontal);
    vec3 ray_direction = normalize(camera_direction + camera_offset.x * offset_dir_horizontal + camera_offset.y * offset_dir_vertical);

    float t = 0.0;
    float dens = 0.0;
    // while(t < VIEW_DEPTH) {
    //     vec3 p = camera_position + t * ray_direction;
    //     dens += DENSITY_SCALE * density(p);
    //     if(dens > 1.0) {
    //         dens = 1.0;
    //         break;
    //     }
    //     t+=step_size;
    // }

    dens = 1000.0*density(camera_position + 5.0 * ray_direction);

    out_color = vec4(dens * WATER_COLOR, 1.0);
}