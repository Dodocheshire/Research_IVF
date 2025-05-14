#ifndef SIMPLE_KNN_H
#define SIMPLE_KNN_H

#include "KNN.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

class SimpleKNN : public KNN {
public:
    std::vector<int> find_k_nearest(
        const std::vector<float>& query,
        int k,
        const std::vector<std::vector<float>>& all_centroids) const override;

    std::unordered_set<int> find_k_nearest_batch(
        const std::vector<std::vector<float>>& queries,
        int k,
        const std::vector<std::vector<float>>& all_centroids) const override;
};

#endif