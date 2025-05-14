#include "SimpleKNN.h"
#include "define.h"

std::vector<int> SimpleKNN::find_k_nearest(
    const std::vector<float>& query,
    int k,
    const std::vector<std::vector<float>>& all_centroids) const
{
    std::vector<std::pair<float, int>> dist_idx;

    for (int i = 0; i < all_centroids.size(); ++i) {
        float dist = Distance(query, all_centroids[i]);
        dist_idx.emplace_back(dist, i);
    }

    std::partial_sort(dist_idx.begin(), dist_idx.begin() + k, dist_idx.end()); //Nlogk
    std::vector<int> result;
    for (int i = 0; i < k && i < dist_idx.size(); ++i) {
        result.push_back(dist_idx[i].second);
    }
    return result;
}

std::unordered_set<int> SimpleKNN::find_k_nearest_batch(
    const std::vector<std::vector<float>>& queries,
    int k,
    const std::vector<std::vector<float>>& all_centroids) const
{
    std::unordered_set<int> result;
    for (const auto& q : queries) {
        auto neighbors = find_k_nearest(q, k, all_centroids);
        result.insert(neighbors.begin(), neighbors.end());
    }
    return result;
}
