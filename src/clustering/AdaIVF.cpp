#include "AdaIVF.h"
#include "B_k_means.h"
#include "KNN.h"


AdaIVF::AdaIVF(int nprobe, int max_iter) : nprobe_(nprobe) {
    indexer_ = new B_K_means(max_iter);
}

void AdaIVF::build(std::vector<std::vector<float>>& data){
    indexer_->build(data, partitions_, vid_to_pid_);
}

std::vector<vector_id_t> AdaIVF::search(const std::vector<float>& query, int k) {
    using Centroid = std::pair<std::vector<float>, partition_id_t>;
    std::vector<Centroid> centroids;
    centroids.reserve(partitions_.size());
    // 1. 获取所有 centroid
    for(int i = 0; i < partitions_.size(); ++i) {
        centroids.emplace_back(partitions_[i]->getCentroid(), i);
    }
    auto DistFunc = [](const Centroid& a, const Centroid& b) -> float {
        return Distance(a.first, b.first);
    };
    // 2. 使用 knn 选择 nprobe 个最接近分区
    KNN<Centroid> knn(DistFunc) ;
    auto parititon_ids = knn.find_k_nearest(Centroid(query, -1), nprobe_, centroids);

    using Candidate = std::pair<std::vector<float>, vector_id_t>;
    std::vector<Candidate> candidates;

    // 3. 从这些分区提取候选向量
    for (int pid : parititon_ids) {
        const auto& part = partitions_[pid];
        const auto& data = part->getData();
        const auto& ids = part->vector_ids;
        const auto& bitmap = part->null_bitmap_;
        for (size_t i = 0; i < data.size(); ++i) {
            if (!bitmap.is_null(i)) {
                candidates.emplace_back(data[i], ids[i]);
            }
        }
    }

    // 4. 精排
    std::vector<int> nearest_idx = knn.find_k_nearest({query, -1}, k, candidates);

    // 5. 提取最终 vector_id_t
    std::vector<vector_id_t> results;
    for (int idx : nearest_idx) {
        results.push_back(candidates[idx].second);
    }

    return results;

}
