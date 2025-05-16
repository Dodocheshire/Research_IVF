// update_engine.h
#pragma once

#include <vector>
#include <cstddef>

#include "ClusterManager.h"
#include "PostingList.h"
#include "VectorRouter.h"

class UpdateEngine {
public:
    UpdateEngine(ClusterManager* cm, PostingList* pl, VectorRouter* router, size_t target_cluster_size);

    void Insert(const std::vector<float>& vec, size_t vec_id);
    void Delete(size_t vec_id);

    void MaybeReindex(); // 根据条件触发局部重聚类（后续实现）

private:
    ClusterManager* cluster_manager_;
    PostingList* posting_list_;
    VectorRouter* vector_router_;

    size_t target_cluster_size_;
    size_t insert_count_ = 0;
    size_t reindex_trigger_threshold_ = 1000; // 可调参数
};
