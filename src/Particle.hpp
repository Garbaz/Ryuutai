#pragma once

#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

struct Particle {
    // Initialized data
    glm::vec3 pos;
    glm::vec3 vel = glm::vec3(0);
    glm::vec3 acc = glm::vec3(0);
    GLfloat mass;
    GLint advect;/*bool*/
    // Simulation data
    GLfloat density;
    GLfloat pressure;
    // Render data
    glm::vec3 color = glm::vec3(0.2, 0.2, 0.5);
    GLint boundary = true;/*bool*/

    Particle(glm::vec3 pos_, float mass_, float advect_) {
        pos = pos_;
        mass = mass_;
        advect = advect_;
    }
    Particle(glm::vec3 pos_, float mass_, float advect_, glm::vec3 color_) : Particle(pos_, mass_, advect_) {
        color = color_;
    }
};

namespace particle {
float mass(const Particle &p);
glm::vec3 vel(const Particle &p);
float pressure(const Particle &p);
}  // namespace particle
