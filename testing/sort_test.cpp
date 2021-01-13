#include <iostream>
#include <vector>
#include <algorithm>

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

int main() {
    std::vector<unsigned int> ind = {2, 1, 4, 3, 0};
    std::vector<int> a = {1, 2, 3, 4, 5};

    for (const int& x : a) {
        std::cout << x << std::endl;
    }
    std::cout << "---" << std::endl;
    sort_to_index_array<int>(a, ind);
    std::cout << "---" << std::endl;
    for (const int& x : a) {
        std::cout << x << std::endl;
    }
}