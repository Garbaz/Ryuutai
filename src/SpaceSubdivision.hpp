#pragma once

#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <functional>

#include "Utils.hpp"

template <typename T>
class Neighbourhood {
   public:
    glm::uvec3 cell;
    std::vector<T> *array;

    Neighbourhood(glm::uvec3 cell, std::vector<T> *array, std::vector<unsigned int> *C, std::vector<unsigned int> *L, const std::function<unsigned int(glm::uvec3)> &sf_curve) {
        this->cell = cell;
        this->array = array;
        this->C = C;
        this->L = L;
        this->sf_curve = sf_curve;
    }

    void for_all(std::function<void(const T&)> f) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -1; dz <= 1; dz++) {
                    unsigned int index = sf_curve(glm::uvec3(cell.x-dx,cell.y-dy,cell.z-dz));
                    //DBG(index);
                    unsigned int c = (*C)[index], end = (*C)[index + 1];
                    for (; c < end; c++) {
                        f((*array)[(*L)[c]]);
                    }
                }
            }
        }
    }

   private:
    std::vector<unsigned int> *C;
    std::vector<unsigned int> *L;
    std::function<unsigned int(glm::uvec3)> sf_curve;
};
