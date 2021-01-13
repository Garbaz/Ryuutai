#include "SPH.hpp"

#include <functional>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>

#include "Particle.hpp"
#include "Utils.hpp"

SPH::SPH(float h) {
    kernel_h = h;
    kernel_alpha = 1 / (4 * glm::pi<float>() * h * h * h);
}

float SPH::kernel(const glm::vec3 &xi, const glm::vec3 &xj) {
    glm::vec3 d = (xi - xj) / kernel_h;
    float q = glm::length(d);
    float t1 = glm::max(1 - q, 0.0f);
    float t2 = glm::max(2 - q, 0.0f);
    return kernel_alpha * (t2 * t2 * t2 - 4 * t1 * t1 * t1);
}

glm::vec3 SPH::kernel_deriv_rev(const glm::vec3 &xi, const glm::vec3 &xj) {
    glm::vec3 d = (xi - xj) / kernel_h;
    float q = glm::length(d);
    if (q > 0) {
        float t1 = glm::max(1 - q, 0.0f);
        float t2 = glm::max(2 - q, 0.0f);
        return kernel_alpha * d / q * (-3 * t2 * t2 + 12 * t1 * t1);
    } else {
        return glm::vec3(0);
    }
}

float SPH::density(const glm::vec3 &pos, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a) {
    float s = 0;
    neighbours.for_all([this,&s,&a,&pos](const Particle &pj) {
        s += a(pj) * kernel(pos, pj.pos);
    });
    return s;
}

glm::vec3 SPH::gradient_over_density(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a) {
    glm::vec3 s = glm::vec3(0);
    neighbours.for_all([this,&s,&a,&pi](const Particle &pj) {
        s += pj.mass * (a(pi) / (pi.density * pi.density) + a(pj) / (pj.density * pj.density)) * kernel_deriv_rev(pi.pos, pj.pos);
    });
    return s;
}

glm::vec3 SPH::gradient(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<float(const Particle &)> a) {
    return pi.density * gradient_over_density(pi, neighbours, a);
}

glm::vec3 SPH::laplace(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<glm::vec3(const Particle &)> a) {
    glm::vec3 s = glm::vec3(0);
    neighbours.for_all([this,&s,&a,&pi](const Particle &pj) {
        glm::vec3 d = pi.pos - pj.pos;
        s += (pj.mass / pj.density) * (glm::dot(a(pi) - a(pj), d) / (glm::length2(d) + 0.01 * kernel_h * kernel_h)) * kernel_deriv_rev(pi.pos, pj.pos);
    });
    return 2 * s;
}

float SPH::divergence(const Particle &pi, Neighbourhood<Particle> neighbours, const std::function<glm::vec3(const Particle &)> a) {
    float s = 0;
    neighbours.for_all([this,&s,&a,&pi](const Particle &pj) {
        s += pj.mass * glm::dot(a(pj), kernel_deriv_rev(pi.pos, pj.pos));
    });

    return -1 / pi.density * s;
}
