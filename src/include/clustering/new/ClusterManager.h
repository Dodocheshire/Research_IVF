#pragma once

#include <vector>
#include <cstddef>

class ClusterManager {
public:
    ClusterManager(size_t dim, size_t num_clusters);

    size_t FindNearestCluster(const std::vector<float>& vec) const;
    std::vector<size_t> FindTopKClusters(const std::vector<float>& vec, size_t k) const;

    void UpdateCluster(size_t cluster_id, const std::vector<float>& vec);
    const std::vector<float>& GetCentroid(size_t cluster_id) const;
    float GetCentroidDrift(size_t cluster_id) const;
    size_t GetClusterSize(size_t cluster_id) const;

    size_t GetNumClusters() const;

private:
    size_t dim_;
    size_t num_clusters_;

    std::vector<std::vector<float>> centroids_;      // 聚类中心
    std::vector<std::vector<float>> initial_centroids_; // 初始中心（用于漂移计算）
    std::vector<size_t> cluster_sizes_;              // 每个聚类当前大小
};
