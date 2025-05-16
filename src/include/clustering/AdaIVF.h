#pragma once
#include "B_k_means.h"
#include <queue>
#include <vector>
#include <unordered_set>
#include "define.h"
#include "Partition.h"
#include "Indexer.h"

class Indicator {
public:
    float local_indicator(const Partition *p) {

    }

    float global_indicator(const Partition *p) {

    }
private:

    float a = 0.1; // temperature factor
    float sz = 1000.0; // target partition size
    float b = 0.3; // controls the contribution of imbalance vs. drift

    /**
     * @brief temperature scaling function f_t
     * @details if the temperature is high,
it should have a larger score, and vice-versa for low temperature
     */
    float TSF(float T) {
        return a * T;
    }
    /**
     * @brief Local size Imbalance Function f_s.
     * @param [in] s: current partition size
     * @details captures if a partition has grown too large or small.
     */
    float LSIF(float s) {
        if(s >= sz) {
            return (s-sz)/sz;
        } else {
            return (sz-s)/s;
        }
    }
    /**
     * @brief Local Drift Function f_d
     * @details captures if a partition 𝑐 has drifted from its initial state 𝑐0, which is a proxy for increasing reconstruction error.
     */
    float LDF(const std::vector<float>& center_init, const std::vector<float>& center_curr) {
        return Distance(center_init, center_curr) / Distance(center_init, std::vector<float>(center_init.size(), 0.0));
    }
    /**
     * @brief Global Imbalance Function
     * @details the relative change in the standard deviation of partition sizes
     */
    float GIF(float size_deviation_init, float size_deviation_curr) {
        return fabs(size_deviation_init - size_deviation_curr) / size_deviation_init;
    }
    /**
     * @brief Global Reconstruction Error Function
     * @details the relative difference between the current observed error 𝜀(MSE) and an ideal error 𝜀. We define ideal error as 
     * the error that would be measured if a full index rebuild were performed, it is estimated by formula.
     */
    float GREF();

};

class AdaIVF {
public:
    AdaIVF(int nprobe_, int max_iter_ = 100);

    ~AdaIVF();

    void build(std::vector<std::vector<float>>& data);

    std::vector<vector_id_t> search(const std::vector<float>& query, int k);

    void insert(const std::vector<float>& vec, vector_id_t id);

    void remove(vector_id_t id);
    

private:
    int nprobe_;
    bool centroids_updated_ = false;

    std::unordered_map<vector_id_t, partition_id_t> vid_to_pid_; //global map from vectors to partition
    std::vector<Partition*> partitions_;

    // params for reindex
    const float target_partition_size_ = 1000.0f; // Target partition size
    float G_; // global threshold;
    float F_; // local threshold;
    int temperature_probe_; // number of heated partitions when querying, default equals nprobe
    Indicator indicator_; // global/local indicator class
    Indexer *indexer_; // 用于全局或局部重构索引
    
    // global meta data
    float initial_error_ = 0.0f; //上一次全局重建后的重建误差 ε₀,用于估算当前理想重建误差 ε'
    float initial_size_stddev_ = 0.0f; //上一次全局重建后的分区size的标准差 σ₀
    int initial_vector_count_ = 0;//全局重建后更新
    int current_vector_count_ = 0;//实时更新
    int vector_dim_ = 0; //用于估计ε'，可有可无

    void update_on_insert(Partition* p);

    void update_on_delete(Partition* p);

    /**
     * @brief update temperature(partitions near query is heated, otherwise cooled)
     */
    void update_on_query(const std::vector<partition_id_t>& accessed_partitions);

    /**
     * @brief 隔一段更新和查询才检查一次
     */
    bool should_local_reindex(partition_id_t pid) const;

    bool should_global_reindex() const;

    /**
     * @brief 隔一段更新和查询检查一次，并决定是否要局部重索引
     */
    void local_reindex();

    /**
     * @brief 隔一段更新和查询检查一次，并决定是否要全局重索引
     */
    void global_reindex();

    // void build_inverted_lists(const DataSet& data);
    // std::vector<int> find_nearest_clusters(const Vector& query, int nprobe) const;
    // void update_centroid(int cluster_id);
    // void update_temperature(const Vector& query, std::vector<int>& nearest_clusters);
    // float calculate_reindexing_score(int cluster_id);
    // void local_reindex(const std::vector<int>& clusters_to_reindex);
    // void incremental_reassign();
    // void validate_cluster(int cluster_id);
};