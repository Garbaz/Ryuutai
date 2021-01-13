#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

struct Particle {
    // Initialized data
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    float mass;
    unsigned int advect; // USED AS BOOLEAN, using u-int for glsl compat
    // Simulation data
    float density;
    float pressure;

    Particle(glm::vec3 pos_, float mass_, float advect_) {
        pos = pos_;
        vel = glm::vec3(0);
        acc = glm::vec3(0);
        mass = mass_;
        advect = advect_;
    }
};

namespace particle {
float mass(const Particle &p);
glm::vec3 vel(const Particle &p);
float pressure(const Particle &p);
}  // namespace particle
