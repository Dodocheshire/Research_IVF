#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <vector>

using Vector = std::vector<float>;
using Dataset = std::vector<Vector>;

Dataset load_data(const std::string& filename);
void generate_test_data(
    const std::string& filename,
    int num_vectors = 10000,
    int dim = 128
);
#endif