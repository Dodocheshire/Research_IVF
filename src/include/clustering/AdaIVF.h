#pragma once
#include "k_means.h"
#include <queue>
#include <vector>
#include <unordered_set>

class IVFIndex {
public:
    IVFIndex(int nlist_, int nprobe_, int max_iter_ = 100);
    void build(const DataSet& data);
    std::vector<int> search(const Vector& query, int k);
    void insert(const Vector& vec);
    void remove(int vec_id);
    void split_cluster(int cluster_id);
    void merge_clusters(int cluster_id1, int cluster_id2);

private:
    int nlist;
    int nprobe;
    KMeans kmeans;
    DataSet data;
    DataSet centroids;
    std::vector<std::vector<int>> inverted_lists_;
    std::unordered_set<int> deleted_ids_;
    bool centroids_updated_ = false;

    struct PartitionMetadata {
        int size;
        Vector initial_centroid;
        Vector current_centroid;
        float temperature;
        float reindexing_score;
    };
    std::vector<PartitionMetadata> partition_metadata_;
    float target_partition_size_ = 1000.0f; // Target partition size

    void build_inverted_lists(const DataSet& data);
    std::vector<int> find_nearest_clusters(const Vector& query, int nprobe) const;
    void update_centroid(int cluster_id);
    void update_temperature(const Vector& query, std::vector<int>& nearest_clusters);
    float calculate_reindexing_score(int cluster_id);
    void local_reindex(const std::vector<int>& clusters_to_reindex);
    void incremental_reassign();
    void validate_cluster(int cluster_id);
};