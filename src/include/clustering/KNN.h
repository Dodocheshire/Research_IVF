#ifndef KNN_H
#define KNN_H

#include <vector>
#include "define.h"
#include <unordered_set>

// interface
class KNN {
public:
    virtual ~KNN() = default;

    /**
     * @brief Find the k nearest partitions (by centroid) to the given query
     * @param query The vector to search neighbors for
     * @param k Number of neighbors
     * @param all_centroids Centroid vectors of all partitions
     * @return indices of the k nearest centroids
     */
    virtual std::vector<int> find_k_nearest(
        const std::vector<float>& query,
        int k,
        const std::vector<std::vector<float>>& all_centroids) const = 0;

    /**
     * @brief Batch version: for multiple queries, return a union set of k nearest for each
     * @param queries List of query vectors
     * @param k Number of neighbors per query
     * @param all_centroids Centroid vectors of all partitions
     * @return a set of indices (no duplication)
     */
    virtual std::unordered_set<int> find_k_nearest_batch(
        const std::vector<std::vector<float>>& queries,
        int k,
        const std::vector<std::vector<float>>& all_centroids) const = 0;
};


#endif