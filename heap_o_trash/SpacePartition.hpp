#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <vector>

#include "Particle.hpp"

struct Neighbourhood {
    glm::ivec3 chunk;
    std::vector<std::vector<Particle>*> buckets;
};

class SpacePartition {
   public:
    SpacePartition(size_t num_buckets_per_dim, glm::vec3 chunk_size);
    void add(const Particle &p);
    Neighbourhood get_neighbourhood(glm::ivec3 chunk);

   private:
    glm::vec3 chunk_size;
    size_t num_buckets_per_dim;
    std::vector<std::vector<Particle>> buckets;

    size_t hash(glm::ivec3 chunk);
};