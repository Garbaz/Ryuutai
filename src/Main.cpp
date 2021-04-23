#include "Main.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <limits>
#include <chrono>
#include <thread>
// #include <mutex>

#include "GLUtils.hpp"
#include "ShaderProgram.hpp"
#include "Camera.hpp"
#include "Utils.hpp"
#include "Particle.hpp"
#include "SPH.hpp"
#include "SpaceSubdivision.hpp"

const float FLUID_PARTICLE_SPACING_H = 0.4;
const float FLUID_REST_DENSITY = 1.0;
const float FLUID_PARTICLE_MASS = FLUID_REST_DENSITY * (FLUID_PARTICLE_SPACING_H * FLUID_PARTICLE_SPACING_H * FLUID_PARTICLE_SPACING_H);

const float FLUID_KINEMATIC_VISCOSITY_NU = 1.0e-3 /*1.0e-6*/;
const float FLUID_STIFFNESS = 5000.0;

SPH sph;

const float CFL_LAMBDA = 0.7;
const float MAX_TIMESTEP = 0.01;

const glm::vec3 SIMULATION_DOMAIN_DIM = glm::vec3(8, 5, 4);
const glm::vec3 SIMULATION_DOMAIN_ORIGIN = glm::vec3(-0.5 * SIMULATION_DOMAIN_DIM.x, 0, -0.5 * SIMULATION_DOMAIN_DIM.z);
float SSUBDIV_CELL_SIZE;
glm::uvec3 SSUBDIV_GRID_SIZE;
unsigned int SSUBDIV_TOTAL_CELLS;

const unsigned int ALIGN_PART_DATA_EVERY = 100;
int align_part_data_counter = 0;

const glm::vec3 GRAVITY = glm::vec3(0, -9.81, 0);

// const float PHYSICS_TIME_SCALE = 0.1;

GLFWwindow *window;
glm::ivec2 viewport_size;

bool mouse_captured = false;
bool mouse_just_captured = true;
glm::ivec2 cursor_pos;

Camera camera(glm::vec3(0, 0, -10));

bool render_raytraced = false;

bool simulation_pause = true;
bool simulation_step = false;

struct DebugLine {
    glm::vec3 e0;
    glm::vec3 col0;
    glm::vec3 e1;
    glm::vec3 col1;
};

bool draw_debug_lines = false;
std::vector<DebugLine> debug_lines;
void add_debug_line(glm::vec3 e0, glm::vec3 e1, glm::vec3 col) {
    debug_lines.push_back(DebugLine{e0, col, e1, col});
}
void add_debug_line_(glm::vec3 b, glm::vec3 d, glm::vec3 col) {
    add_debug_line(b, b + d, col);
}
void add_debug_line_(glm::vec3 b, glm::vec3 d, glm::vec3 col, float len) {
    add_debug_line_(b, len * glm::normalize(d), col);
}

int neighbour_coloring_cutoff = 4;

std::vector<Particle> particles;
// std::mutex mutex_sim_data;

std::vector<unsigned int> ssubdiv_C;
std::vector<unsigned int> ssubdiv_L;

unsigned int sf_curve(glm::ivec3 klm) {
    return modulus(klm.x, SSUBDIV_GRID_SIZE.x) +
           modulus(klm.y, SSUBDIV_GRID_SIZE.y) * SSUBDIV_GRID_SIZE.x +
           modulus(klm.z, SSUBDIV_GRID_SIZE.z) * SSUBDIV_GRID_SIZE.x * SSUBDIV_GRID_SIZE.y;
}
glm::ivec3 sf_cell(glm::vec3 p) {
    return glm::ivec3((p - SIMULATION_DOMAIN_ORIGIN) / (2 * FLUID_PARTICLE_SPACING_H));
}
unsigned int sf_index(glm::vec3 p) {
    glm::ivec3 klm = sf_cell(p);
    return sf_curve(klm);
}

float physics_step(/*float dt*/) {
    // dt *= PHYSICS_TIME_SCALE;

#undef TIMEIT_TIMEIT  // To keep the compiler from bitching about redefinition
#define TIMEIT_TIMEIT false

    TIMEIT_BEGIN;

    // TIMEIT_SAMPLE_START
    // if (align_part_data_counter <= 0) {
    //     align_part_data_counter = ALIGN_PART_DATA_EVERY;
    //     sort_to_index_array(particles, ssubdiv_L);
    //     // for (unsigned int i = 0; i < particles.size(); i++) {
    //     //     particles[i].color = glm::vec3(float(i) / particles.size(), 0, 0);
    //     // }
    // }
    // align_part_data_counter--;
    // TIMEIT_SAMPLE_STOP("space subdiv - align part data")

    TIMEIT_SAMPLE_START
    std::fill(ssubdiv_C.begin(), ssubdiv_C.end(), 0);
    for (const Particle &p : particles) {
        ssubdiv_C[sf_index(p.pos)]++;
    }
    TIMEIT_SAMPLE_STOP("space subdiv - 1st particle pass")

    TIMEIT_SAMPLE_START
    for (unsigned int i = 1; i < ssubdiv_C.size(); i++) {
        ssubdiv_C[i] += ssubdiv_C[i - 1];
    }
    TIMEIT_SAMPLE_STOP("space subdiv - cell pass")

    TIMEIT_SAMPLE_START
    for (unsigned int i = 0; i < particles.size(); i++) {
        const Particle &p = particles[i];
        unsigned int c = sf_index(p.pos);
        ssubdiv_L[--ssubdiv_C[c]] = i;
    }
    TIMEIT_SAMPLE_STOP("space subdiv - 2nd particle pass")

    TIMEIT_SAMPLE_START
    for (Particle &p : particles) {
        p.density = sph.density(p.pos, Neighbourhood<Particle>(sf_cell(p.pos), &particles, &ssubdiv_C, &ssubdiv_L, sf_curve), particle::mass);
        // DBG(p.density);
        p.pressure = glm::max(FLUID_STIFFNESS * (p.density / FLUID_REST_DENSITY - 1.0f), 0.0f);
        // DBG(p.pressure);
    }
    TIMEIT_SAMPLE_STOP("simulation - density & pressure")

    TIMEIT_SAMPLE_START
    for (Particle &p : particles) {
        if (p.advect) {
            glm::vec3 a_nonp = FLUID_KINEMATIC_VISCOSITY_NU * sph.laplace(p, Neighbourhood<Particle>(sf_cell(p.pos), &particles, &ssubdiv_C, &ssubdiv_L, sf_curve), particle::vel) + GRAVITY;
            glm::vec3 a_p = -1 * sph.gradient_over_density(p, Neighbourhood<Particle>(sf_cell(p.pos), &particles, &ssubdiv_C, &ssubdiv_L, sf_curve), particle::pressure);
            // DBG(glm::to_string(a_nonp));
            // DBG(glm::to_string(a_p));
            p.acc = a_nonp + a_p;
            // add_debug_line_(p.pos, p.acc, glm::vec3(1, 0, 0), 0.25);
        }
    }
    TIMEIT_SAMPLE_STOP("simulation - accelerations")

    TIMEIT_SAMPLE_START

    float max_vel_sq = 0;
    for (Particle &p : particles) {
        float vel_mag_sq = glm::length2(p.vel);
        if (vel_mag_sq > max_vel_sq) {
            max_vel_sq = vel_mag_sq;
        }
    }
    float dt = CFL_LAMBDA * FLUID_PARTICLE_SPACING_H / glm::max(glm::sqrt(max_vel_sq), CFL_LAMBDA * FLUID_PARTICLE_SPACING_H / MAX_TIMESTEP);

    if (!simulation_pause || simulation_step) {
        for (Particle &p : particles) {
            if (p.advect) {
                p.vel += dt * p.acc;
                p.pos += dt * p.vel;
            }
        }
    }
    simulation_step = false;
    TIMEIT_SAMPLE_STOP("advect step")

    /*{
        for (Particle &p : particles) {
            p.color = glm::vec3(0, 0, 1);
        }

        // static int counter = 30 * 200;
        // Particle &p = particles[counter / 30];
        // Neighbourhood<Particle> n(sf_cell(p.pos), &particles, &ssubdiv_C, &ssubdiv_L, sf_curve);
        // n.for_all([](Particle &o) {
        //     o.color = glm::vec3(1, 0, 0);
        // });
        // float dens = 1 / sph.density(p.pos, n, [](const Particle &o) { return 1; });
        // float grad = glm::length(sph.gradient(p, n, [](const Particle &o) { return 1; }));
        // if (counter % 30 == 0) {
        //     std::cout << glm::to_string(p.pos) << ","
        //               << dens << ","
        //               << grad << std::endl;
        // }
        // p.color = glm::vec3(0, 1, 0);

        // counter++;

        for (Particle &p : particles) {
            Neighbourhood<Particle> n(sf_cell(p.pos), &particles, &ssubdiv_C, &ssubdiv_L, sf_curve);
            glm::vec3 grad = sph.gradient(p, n, [](const Particle &o) { return 1; });
            // p.color = glm::vec3(glm::length(grad), 0, 0);
            // add_debug_line_(p.pos, -grad, glm::vec3(0, 0, 0));
            if (p.advect) {
                // if (sph.kernel(p.pos, particles[0].pos) > 0) {
                //     p.color = glm::vec3(1, 0, 0);
                // }
                // glm::ivec3 cell = sf_cell(p.pos);
                // p.color = random_saturated_color(glm::sin(cell.x + cell.y + cell.z), cell.x + cell.y + cell.z);
            }
        }
    }*/

    TIMEIT_END

    return glm::sqrt(max_vel_sq);
}

void simulation_loop() {
    // auto lasttime = std::chrono::high_resolution_clock::now();
    // float time_step = 0.01 * CFL_LAMBDA * FLUID_PARTICLE_SPACING_H;
    while (true) {
        // auto currtime = std::chrono::high_resolution_clock::now();
        // double dt = std::chrono::duration_cast<std::chrono::duration<double>>(currtime - lasttime).count();
        // lasttime = currtime;
        // mutex_sim_data.lock();
        // float max_vel = physics_step();
        physics_step();
        // if (max_vel != 0.0f) {

        // }
        // mutex_sim_data.unlock();
    }
}

std::vector<glm::uvec2> edges;

std::vector<glm::uvec3> faces;

bool draw_vertecies = true, draw_edges = true, draw_faces = true;

bool render_boundary = false;

std::vector<ShaderProgram *> shaders;

std::thread thread_simulation;

int main() {
    //PARTICLE SETUP

    // The simulation domain boundary
    {
        const float PARTICLE_SPACING = 0.75 * FLUID_PARTICLE_SPACING_H;
        const int LAYERS = 2;
        const float DENSITY = 2.0;
        const float MASS = DENSITY * (PARTICLE_SPACING * PARTICLE_SPACING * PARTICLE_SPACING);
        const glm::vec3 COLOR = glm::vec3(0.5, 0.5, 0.5);
        for (int l = 0; l < LAYERS; l++) {
            glm::vec3 size = SIMULATION_DOMAIN_DIM + glm::vec3(l * PARTICLE_SPACING);
            for (float x = 0; x <= size.x; x += PARTICLE_SPACING) {
                for (float y = 0; y <= size.y; y += PARTICLE_SPACING) {
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(x, y, -l * PARTICLE_SPACING), MASS, false));
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(x, y, size.z), MASS, false));
                }
            }
            for (float x = 0; x <= size.x; x += PARTICLE_SPACING) {
                for (float z = 0; z <= size.z; z += PARTICLE_SPACING) {
                    float y = size.y;
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(x, -l * PARTICLE_SPACING, z), MASS, false));
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(x, size.y, z), MASS, false));
                }
            }
            for (float z = 0; z <= size.z; z += PARTICLE_SPACING) {
                for (float y = 0; y <= size.y; y += PARTICLE_SPACING) {
                    float x = size.x;
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(-l * PARTICLE_SPACING, y, z), MASS, false));
                    particles.push_back(Particle(SIMULATION_DOMAIN_ORIGIN + glm::vec3(size.x, y, z), MASS, false));
                }
            }
        }
    }

    // The fluid body
    for (float x = 0; x <= 3.5; x += FLUID_PARTICLE_SPACING_H) {
        for (float y = 1; y <= 4; y += FLUID_PARTICLE_SPACING_H) {
            for (float z = -1; z <= 1; z += FLUID_PARTICLE_SPACING_H) {
                particles.push_back(Particle(glm::vec3(x, y, z), FLUID_PARTICLE_MASS, true));
            }
        }
    }
    // particles.push_back(Particle(glm::vec3(0, 1, 0), 1.0, true));
    // particles.push_back(Particle(glm::vec3(0, 0, 0), 1.0, true));
    // {
    //     const glm::vec2 SIZE = glm::vec2(10, 5);
    //     const float SPACING = 0.75 * FLUID_PARTICLE_SPACING_H;
    //     const int LAYERS = 2;
    //     const float MASS = 10.0;
    //     const float HEIGHT = 2;
    //     for (int i = 0; i * SPACING <= SIZE.x; i++) {
    //         for (int j = 0; j * SPACING <= SIZE.y; j++) {
    //             float x = float(i) * SPACING - 0.5 * SIZE.x;
    //             float z = float(j) * SPACING - 0.5 * SIZE.y;
    //             for (int l = 0; l < LAYERS; l++) {
    //                 particles.push_back(Particle(glm::vec3(x, -l * SPACING, z), MASS, false));
    //             }
    //             if (i == 0 || (i + 1) * SPACING > SIZE.x) {
    //                 float ls = (i == 0 ? -1 : 1);
    //                 for (int l = 0; l < LAYERS; l++) {
    //                     for (int h = 1; h * SPACING <= HEIGHT; h++) {
    //                         particles.push_back(Particle(glm::vec3(x, h * SPACING, z + ls * SPACING * l), MASS, false));
    //                     }
    //                 }
    //             } else if (j == 0 || (j + 1) * SPACING > SIZE.y) {
    //                 float ls = (j == 0 ? -1 : 1);
    //                 for (int l = 0; l < LAYERS; l++) {
    //                     for (int h = 1; h * SPACING <= HEIGHT; h++) {
    //                         particles.push_back(Particle(glm::vec3(x, h * SPACING, z + ls * SPACING * l), MASS, false));
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    sph = SPH(1.0 * FLUID_PARTICLE_SPACING_H);

    // DBG("kernel_h = " << FLUID_PARTICLE_SPACING_H);

    SSUBDIV_CELL_SIZE = 2 * FLUID_PARTICLE_SPACING_H;
    // DBG(SSUBDIV_CELL_SIZE);
    SSUBDIV_GRID_SIZE = glm::uvec3(SIMULATION_DOMAIN_DIM / SSUBDIV_CELL_SIZE);
    // DBG(glm::to_string(SSUBDIV_GRID_SIZE));
    SSUBDIV_TOTAL_CELLS = SSUBDIV_GRID_SIZE.x * SSUBDIV_GRID_SIZE.y * SSUBDIV_GRID_SIZE.z;
    // DBG(SSUBDIV_TOTAL_CELLS);

    // DBG(glm::to_string(SIMULATION_DOMAIN_ORIGIN));

    ssubdiv_C = std::vector<unsigned int>(SSUBDIV_TOTAL_CELLS + 1);
    ssubdiv_L = std::vector<unsigned int>(particles.size());

    std::cout << "~~~~~~~~~~~~~  Ryuutai (流体)  ~~~~~~~~~~~~~\n"
              << "Total number of particles: " << particles.size() << "\n"
              << "Particle spacing h: " << FLUID_PARTICLE_SPACING_H << "\n"
              << "Particle volume h³: " << FLUID_PARTICLE_SPACING_H * FLUID_PARTICLE_SPACING_H * FLUID_PARTICLE_SPACING_H << "\n"
              << "Fluid rest density: " << FLUID_REST_DENSITY << "\n"
              << "Simulation domain size: " << glm::to_string(SSUBDIV_GRID_SIZE) << "\n"
              << "Ssubdiv cell size: " << SSUBDIV_CELL_SIZE << "\n"
              << "Ssubdiv total number of cells: " << SSUBDIV_TOTAL_CELLS << "\n"
              << std::endl;

    //GL SETUP
    window = init_glfw("流体");
    init_glew();

    glClearColor(0.7, 1.0, 1.0, 1.0);

    glfwGetFramebufferSize(window, &viewport_size.x, &viewport_size.y);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_LINE_SMOOTH);
    //glLineWidth(7.0f); //??????????????????????

    // {
    //     glm::vec2 lwr;
    //     glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, &lwr.x);
    //     DBG(glm::to_string(lwr));
    // }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vbo_particles;
    glGenBuffers(1, &vbo_particles);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_particles);
    glBufferData(GL_ARRAY_BUFFER, vectorsizeof(particles), particles.data(), GL_DYNAMIC_DRAW);

    GLuint ebo_edges;
    glGenBuffers(1, &ebo_edges);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vectorsizeof(edges), edges.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint ebo_faces;
    glGenBuffers(1, &ebo_faces);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_faces);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vectorsizeof(faces), faces.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    ShaderProgram prog_render_point = ShaderProgram("shaders/vertex.vert", "shaders/point.frag");
    shaders.push_back(&prog_render_point);
    prog_render_point.point_attribute("position", 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, pos));
    prog_render_point.point_attribute("color", 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, color));
    prog_render_point.point_attribute("advect", 1, GL_INT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, advect));
    prog_render_point.point_attribute("boundary", 1, GL_INT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, boundary));

    // ShaderProgram prog_render_line = ShaderProgram("shaders/vertex.vert", "shaders/line.frag");
    // shaders.push_back(&prog_render_line);
    // prog_render_line.point_attribute("position", 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, pos));

    // ShaderProgram prog_render_triangle = ShaderProgram("shaders/vertex.vert", "shaders/triangle.frag");
    // shaders.push_back(&prog_render_triangle);
    // prog_render_triangle.point_attribute("position", 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)offsetof(Particle, pos));

    glBindBuffer(GL_ARRAY_BUFFER, 0);  //Unbind vbo_particles

    GLuint vbo_debug_lines;
    glGenBuffers(1, &vbo_debug_lines);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_debug_lines);

    ShaderProgram prog_render_debug_line("shaders/debug_line.vert", "shaders/debug_line.frag");
    shaders.push_back(&prog_render_debug_line);
    prog_render_debug_line.point_attribute("position", 3, GL_FLOAT, GL_FALSE, sizeof(DebugLine) / 2, (void *)offsetof(DebugLine, e0));
    prog_render_debug_line.point_attribute("color", 3, GL_FLOAT, GL_FALSE, sizeof(DebugLine) / 2, (void *)offsetof(DebugLine, col0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /*** Raytrace stuff ***/
    GLuint ssbo_rt_particles, ssbo_rt_ssubdiv_C, ssbo_rt_ssubdiv_L;
    glGenBuffers(1, &ssbo_rt_particles);
    glGenBuffers(1, &ssbo_rt_ssubdiv_C);
    glGenBuffers(1, &ssbo_rt_ssubdiv_L);
    GLuint vbo_rt;
    glGenBuffers(1, &vbo_rt);

    ShaderProgram prog_raytrace("shaders/fluidRT.vert", "shaders/fluidRT.frag");

    glBindBuffer(GL_ARRAY_BUFFER, vbo_rt);
    {
        GLfloat vertices[]{
            -1.0, -1.0,
            -1.0, 1.0,
            1.0, 1.0,
            -1.0, -1.0,
            1.0, 1.0,
            1.0, -1.0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        prog_raytrace.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_rt_particles);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_rt_ssubdiv_C);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_rt_ssubdiv_L);
        prog_raytrace.point_attribute("position", 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    prog_raytrace.set_uniformf("step_size", 0.5f * FLUID_PARTICLE_SPACING_H);
    prog_raytrace.set_uniformu("ssubdiv_grid_size", SSUBDIV_GRID_SIZE);
    prog_raytrace.set_uniformf("simulation_domain_origin", SIMULATION_DOMAIN_ORIGIN);
    prog_raytrace.set_uniformf("kernel_h", FLUID_PARTICLE_SPACING_H);
    prog_raytrace.set_uniformf("kernel_alpha", sph.kernel_alpha);

    { /* Generate projection matrix and pass to all shaders*/
        float aspect = float(viewport_size.x) / float(viewport_size.y);
        float fovy = FIELD_OF_VIEW_HORIZONTAL / aspect;
        glm::mat4 proj = glm::perspective(glm::radians(fovy), aspect, 0.01f, 100.0f);
        // DBG(glm::to_string(proj));
        for (ShaderProgram *sp : shaders) {
            sp->set_uniformf("proj", proj);
        }

        //Pass fov & aspect to raytrace shader
        prog_raytrace.set_uniformi("field_of_view_tan", glm::tan(glm::radians(0.5 * fovy)));
        prog_raytrace.set_uniformf("screen_ratio", aspect);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    double start_time = glfwGetTime();
    double last_frame_time = start_time;

    thread_simulation = std::thread(simulation_loop);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // if (!simulation_pause || simulation_step) {
        debug_lines.clear();
        // }

        // Update time & deltatime
        double frame_time = glfwGetTime();
        double time = frame_time - start_time;
        double deltatime = frame_time - last_frame_time;
        last_frame_time = frame_time;

        // Update camera
        camera.update(deltatime);
        if (camera.dirty) {
            glm::mat4 view = camera.get_view_matrix();
            for (ShaderProgram *sp : shaders) {
                sp->set_uniformf("view", view);
            }
        }
        prog_raytrace.set_uniformf("camera_position", camera.get_position());
        prog_raytrace.set_uniformf("camera_direction", camera.get_direction());

        /* ----- PHYSICS ----- */
        // physics_step(deltatime);

        /* ----- RENDER ----- */

        /* --- OBJECTS --- */
        if (!render_raytraced) {  // show particles as spheres
            glBindBuffer(GL_ARRAY_BUFFER, vbo_particles);
            {
                //Update particle data
                // mutex_sim_data.lock();
                glBufferData(GL_ARRAY_BUFFER, vectorsizeof(particles), particles.data(), GL_DYNAMIC_DRAW);
                // mutex_sim_data.unlock();

                // // //FACES
                // if (draw_faces) {
                //     prog_render_triangle.use();
                //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_faces);
                //     glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
                //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                // }
                // //EDGES
                // if (draw_edges) {
                //     prog_render_line.use();
                //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_edges);
                //     glDrawElements(GL_LINES, 2 * edges.size(), GL_UNSIGNED_INT, 0);
                //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                // }
                //VERTICES
                if (draw_vertecies) {
                    prog_render_point.use();
                    glDrawArrays(GL_POINTS, 0, particles.size());
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        } else {  // render scene with ray tracer
            glBindBuffer(GL_ARRAY_BUFFER, vbo_rt);
            {
                prog_raytrace.use();

                // mutex_sim_data.lock();
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rt_particles);
                glBufferData(GL_SHADER_STORAGE_BUFFER, vectorsizeof(particles), particles.data(), GL_DYNAMIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rt_ssubdiv_C);
                glBufferData(GL_SHADER_STORAGE_BUFFER, vectorsizeof(ssubdiv_C), ssubdiv_C.data(), GL_DYNAMIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rt_ssubdiv_L);
                glBufferData(GL_SHADER_STORAGE_BUFFER, vectorsizeof(ssubdiv_L), ssubdiv_L.data(), GL_DYNAMIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                // mutex_sim_data.unlock();

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        // Rendering of the debug lines
        if (draw_debug_lines) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_debug_lines);
            {
                glBufferData(GL_ARRAY_BUFFER, vectorsizeof(debug_lines), debug_lines.data(), GL_DYNAMIC_DRAW);
                prog_render_debug_line.use();
                glDrawArrays(GL_LINES, 0, 2 * debug_lines.size());
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwTerminate();
        exit(0);
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (mouse_captured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouse_captured = false;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouse_just_captured = true;
            mouse_captured = true;
        }
    } else if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            camera.input_movement.y++;
        } else if (action == GLFW_RELEASE) {
            camera.input_movement.y--;
        }
    } else if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            camera.input_movement.y--;
        } else if (action == GLFW_RELEASE) {
            camera.input_movement.y++;
        }
    } else if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            camera.input_movement.x++;
        } else if (action == GLFW_RELEASE) {
            camera.input_movement.x--;
        }
    } else if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            camera.input_movement.x--;
        } else if (action == GLFW_RELEASE) {
            camera.input_movement.x++;
        }
    } else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        draw_vertecies = !draw_vertecies;
    } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        draw_edges = !draw_edges;
    } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        draw_faces = !draw_faces;
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        for (Particle &p : particles) {
            p.vel += glm::sphericalRand(1.0f);
        }
    } else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        for (Particle &p : particles) {
            p.vel += glm::vec3(0, 10, 0);
            p.vel += glm::sphericalRand(0.1f);
        }
    } else if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        simulation_pause = !simulation_pause;
    } else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        simulation_step = true;
    } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        draw_debug_lines = !draw_debug_lines;
        // } else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        //     particles[0].pos += glm::vec3(0.1, 0, 0);
        // } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        //     particles[0].pos += glm::vec3(-0.1, 0, 0);
        // } else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        //     particles[0].pos += glm::vec3(0, -0.1, 0);
        // } else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        //     particles[0].pos += glm::vec3(0, 0.1, 0);
    } else if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        render_raytraced = !render_raytraced;
    } else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        render_boundary = !render_boundary;
        for (ShaderProgram *s : shaders) {
            s->set_uniformi("render_boundary", render_boundary);
        }
    } else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        // neighbour_coloring_cutoff--;
        // std::cout << neighbour_coloring_cutoff << std::endl;
    } else if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        // neighbour_coloring_cutoff++;
        // std::cout << neighbour_coloring_cutoff << std::endl;
    }
}

void cursor_callback(GLFWwindow *window, double xpos, double ypos) {
    if (mouse_captured && !mouse_just_captured) {
        camera.update_direction(xpos - cursor_pos.x, ypos - cursor_pos.y);
    }
    mouse_just_captured = false;
    cursor_pos.x = xpos;
    cursor_pos.y = ypos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (mouse_captured) {
        // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        //     glm::vec3 camera_pos = camera.get_position();
        //     glm::vec3 camera_dir = camera.get_direction();
        //     spawn_projectile(camera_pos + 1.0 * camera_dir, PROJECTILE_SHOOT_VEL * camera_dir);
        // }
        // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        //     glm::vec3 camera_pos = camera.get_position();
        //     glm::vec3 camera_dir = camera.get_direction();
        //     for (size_t i = 0; i < particles.size(); i++) {
        //         Particle &p = particles[i];
        //         glm::vec3 &pos = particle_pos[i];
        //         glm::vec3 rel_pos = pos - camera_pos;
        //         float dist = glm::distance(rel_pos, camera_dir * glm::dot(rel_pos, camera_dir));
        //         if(dist < 0.01) {

        //         }
        //     }
        // }
    }
}
