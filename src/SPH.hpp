#pragma once

#include <functional>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

#include "Particle.hpp"
#include "SpaceSubdivision.hpp"

class SPH {
   public:
    float kernel_h, kernel_support, kernel_alpha;

    SPH(float particle_rest_spacing);
    SPH() : SPH(0){};

    float density(const glm::vec3 &pos, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a);
    glm::vec3 gradient(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a);
    glm::vec3 gradient_over_density(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a);
    glm::vec3 laplace(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<glm::vec3(const Particle &)> a);
    float divergence(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<glm::vec3(const Particle &)> a);

//    private:
    float kernel(const glm::vec3 &xi, const glm::vec3 &xj);
    glm::vec3 kernel_deriv_rev(const glm::vec3 &xi, const glm::vec3 &xj);
};