#include "Utils.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <glm/glm.hpp>

void pretty_print(glm::mat3 m) {
    std::string s[3][3];
    unsigned int max_len = 0;
    for (size_t x = 0; x < 3; x++) {
        for (size_t y = 0; y < 3; y++) {
            s[x][y] = std::to_string(m[x][y]);
            unsigned int len = s[x][y].length();
            if (len > max_len) max_len = len;
        }
    }
    printf("┌ %*s  %*s  %*s ┐\n", max_len, "", max_len, "", max_len, "");
    for (size_t x = 0; x < 3; x++) {
        printf("│");
        for (size_t y = 0; y < 3; y++) {
            printf(" %*.*s ", max_len, max_len, s[x][y].c_str());
        }
        printf("│\n");
    }
    printf("└ %*s  %*s  %*s ┘\n", max_len, "", max_len, "", max_len, "");
}