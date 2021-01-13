#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <glm/glm.hpp>
#include <vector>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define PRINT_DEBUG true

#if PRINT_DEBUG
#define DBG(s) \
    if (PRINT_DEBUG) std::cerr << __FILENAME__ << "@" << __LINE__ << " : " << s << std::endl
#else
#define DBG(s)
#endif

template <typename T>
size_t vectorsizeof(const typename std::vector<T>& vec) {
    return sizeof(T) * vec.size();
}

template <typename T>
void sort_to_index_array(std::vector<T>& data, std::vector<unsigned int>& index) {
    const unsigned int SIZE = data.size() <= index.size() ? data.size() : index.size();
    for (unsigned int i = 0; i < SIZE; i++) {
        if (index[i] != i) {
            std::swap(data[i], data[index[i]]);
            unsigned int j = i + 1;
            while (i != index[j] && j < index.size()) {
                j++;
            }
            if (j < index.size()) {
                index[j] = index[i];
            }
            index[i] = i;
        }
    }
}

void pretty_print(glm::mat3 m);

#define modulus(n, M) ((((n) % (M)) + (M)) % (M))

#define TIMEIT_TIMEIT true

#define TIMEIT_DURATION_TYPE std::chrono::microseconds

#define TIMEIT_BEGIN                                               \
    auto TIMEIT_begin = std::chrono::high_resolution_clock::now(); \
    auto TIMEIT_start = std::chrono::high_resolution_clock::now(); \
    auto TIMEIT_end = std::chrono::high_resolution_clock::now();   \
    struct TIMEIT_sample {                                         \
        std::string label;                                         \
        long t;                                                    \
    };                                                             \
    std::vector<TIMEIT_sample> TIMEIT_times;

#define TIMEIT_SAMPLE_START                                       \
    if (TIMEIT_TIMEIT) {                                          \
        TIMEIT_start = std::chrono::high_resolution_clock::now(); \
    }

#define TIMEIT_SAMPLE_STOP(label)                                                                                                          \
    if (TIMEIT_TIMEIT) {                                                                                                                   \
        TIMEIT_end = std::chrono::high_resolution_clock::now();                                                                            \
        TIMEIT_times.push_back(TIMEIT_sample{label, std::chrono::duration_cast<TIMEIT_DURATION_TYPE>(TIMEIT_end - TIMEIT_start).count()}); \
    }

#define TIMEIT_END                                                                                                                       \
    if (TIMEIT_TIMEIT) {                                                                                                                 \
        long total = std::chrono::duration_cast<TIMEIT_DURATION_TYPE>(std::chrono::high_resolution_clock::now() - TIMEIT_begin).count(); \
        for (const TIMEIT_sample& t : TIMEIT_times) {                                                                                    \
            double ratio = double(t.t) / total;                                                                                          \
            std::cout << t.label << ": " << t.t << " (" << int(ratio * 100.0) << "%)" << std::endl;                                      \
        }                                                                                                                                \
    }
