#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <random>
#include <cstdint>

Dataset load_data(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(1);
    }

    uint32_t num_vectors, dim;
    in.read(reinterpret_cast<char*>(&num_vectors), sizeof(num_vectors));
    in.read(reinterpret_cast<char*>(&dim), sizeof(dim));

    Dataset data;
    data.reserve(num_vectors);
    for (uint32_t i = 0; i < num_vectors; ++i) {
        Vector vec(dim);
        in.read(reinterpret_cast<char*>(vec.data()), dim * sizeof(float));
        data.emplace_back(std::move(vec));
    }

    std::cout << "Loaded " << data.size() 
              << " vectors, dimension = " << dim 
              << std::endl;
    return data;
}


void generate_test_data(
    const std::string& filename, 
    int num_vectors, 
    int dim
) {
    std::ofstream out(filename, std::ios::binary);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    uint32_t num = num_vectors;
    uint32_t dim_val = dim;
    out.write(reinterpret_cast<const char*>(&num), sizeof(num));
    out.write(reinterpret_cast<const char*>(&dim_val), sizeof(dim_val));

    for (int i = 0; i < num_vectors; ++i) {
        Vector vec(dim);
        for (int j = 0; j < dim; ++j) {
            vec[j] = dis(gen);
        }
        out.write(
            reinterpret_cast<const char*>(vec.data()), 
            dim * sizeof(float)
        );
    }

    std::cout << "Generated test data: " << filename 
              << " (" << num_vectors << "x" << dim << ")"
              << std::endl;
}