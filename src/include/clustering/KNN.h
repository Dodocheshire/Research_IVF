#ifndef KNN_H
#define KNN_H

#include <vector>
#include "define.h"
#include <unordered_set>
#include <functional>
#include <algorithm>
// interface with default implementation
template <typename T>
class KNN {
public:
    using DistanceFunc = std::function<float(const T&, const T&)>;
    explicit KNN(DistanceFunc distance_func) : distance_func_(distance_func) {}
    virtual ~KNN() = default;

    /**
     * @brief Find the k nearest neighbors to the given query
     * @param query The vector to search neighbors for
     * @param k Number of neighbors
     * @param candidates All availabe candidate vectors
     * @return indices of the k nearest centroids
     */
    virtual std::vector<int> find_k_nearest(
        const T& query,
        int k,
        const std::vector<T>& candidates) {

        using Pair = std::pair<float, int>;  // {distance, index}
        std::priority_queue<Pair> max_heap;

        for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
            float dist = distance_func_(query, candidates[i]);
            if (static_cast<int>(max_heap.size()) < k) {
                max_heap.emplace(dist, i);
            } else if (dist < max_heap.top().first) {
                max_heap.pop();
                max_heap.emplace(dist, i);
            }
        }

        std::vector<int> result;
        while (!max_heap.empty()) {
            result.push_back(max_heap.top().second);
            max_heap.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    /**
     * @brief Batch version: for multiple queries, return union set of k nearest indices
     * @return a set of indices (no duplication)
     */
    virtual std::unordered_set<int> find_k_nearest_batch(
        const std::vector<T>& queries,
        int k,
        const std::vector<T>& candidates) {

        std::unordered_set<int> result;
        for (const auto& query : queries) {
            auto nearest = find_k_nearest(query, k, candidates);
            result.insert(nearest.begin(), nearest.end());
        }
        return result;
        };

protected:
    DistanceFunc distance_func_;

};
#endif