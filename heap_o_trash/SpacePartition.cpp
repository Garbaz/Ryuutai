#include "SpacePartition.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <vector>

#include "Particle.hpp"

unsigned int modulus(int n, unsigned int m) {
    return ((n % m) + m) % m;
};

SpacePartition::SpacePartition(size_t num_buckets_per_dim, glm::vec3 chunk_size) {
    buckets = std::vector<std::vector<Particle>>(num_buckets_per_dim * num_buckets_per_dim * num_buckets_per_dim);
    this->chunk_size = chunk_size;
    this->num_buckets_per_dim = num_buckets_per_dim;
}

size_t SpacePartition::hash(glm::ivec3 chunk) {
    return modulus(chunk.x, num_buckets_per_dim) +
           num_buckets_per_dim * modulus(chunk.y, num_buckets_per_dim) +
           num_buckets_per_dim * num_buckets_per_dim * modulus(chunk.z, num_buckets_per_dim);
}

void SpacePartition::add(const Particle &p) {
    glm::ivec3 chunk = glm::floor(glm::vec3(p.pos.x / chunk_size.x, p.pos.y / chunk_size.y, p.pos.z / chunk_size.z));
    size_t index = hash(chunk);
    buckets[index].push_back(p);
}

Neighbourhood SpacePartition::get_neighbourhood(glm::ivec3 chunk) {
    std::vector<std::vector<Particle> *> chunks(27);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                
            }
        }
    }
}