#include "Particle.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

namespace particle {
float mass(const Particle &p) {
    return p.mass;
}
glm::vec3 vel(const Particle &p) {
    return p.vel;
}
float pressure(const Particle &p) {
    return p.pressure;
}
}  // namespace particle