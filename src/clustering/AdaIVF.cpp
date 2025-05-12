#include "AdaIVF.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

struct Candidate {
    int id;
    double dist;
    Candidate(int i, double d) : id(i), dist(d) {}
    bool operator<(const Candidate& other) const { return dist < other.dist; }
};

IVFIndex::IVFIndex(int nlist_, int nprobe_, int max_iter_)
    : nlist(nlist_), nprobe(nprobe_), 
      kmeans(nlist_, max_iter_, false), data() {}

void IVFIndex::build(const DataSet& data_) {
    data = data_;
    kmeans.fit(data_);
    centroids = kmeans.get_centroids();
    build_inverted_lists(data_);
    deleted_ids_.clear();

    partition_metadata_.resize(nlist);
    for (int i = 0; i < nlist; ++i) {
        partition_metadata_[i].size = inverted_lists_[i].size();
        partition_metadata_[i].initial_centroid = centroids[i];
        partition_metadata_[i].current_centroid = centroids[i];
        partition_metadata_[i].temperature = 1.0f;
        partition_metadata_[i].reindexing_score = 0.0f;
    }
}

void IVFIndex::build_inverted_lists(const DataSet& data) {
    inverted_lists_.resize(nlist);
    const auto& labels = kmeans.get_labels();
    
    for (size_t i = 0; i < data.size(); ++i) {
        if (deleted_ids_.count(i)) continue;
        int cluster_id = labels[i];
        if (cluster_id >= 0 && cluster_id < nlist) {
            inverted_lists_[cluster_id].push_back(i);
        }
    }
}

std::vector<int> IVFIndex::find_nearest_clusters(const Vector& query, int nprobe) const {
    std::priority_queue<Candidate> pq;
    
    for (int c = 0; c < nlist; ++c) {
        if (inverted_lists_[c].empty()) continue;
        double dist = KMeans::distance(query, centroids[c]);
        pq.emplace(c, dist);
        if (pq.size() > nprobe) pq.pop();
    }
    
    std::vector<int> result;
    while (!pq.empty()) {
        result.push_back(pq.top().id);
        pq.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<int> IVFIndex::search(const Vector& query, int k) {
    auto cluster_ids = find_nearest_clusters(query, nprobe);
    update_temperature(query, cluster_ids);
    std::priority_queue<Candidate> pq;
    
    for (int c : cluster_ids) {
        for (int vec_id : inverted_lists_[c]) {
            if (deleted_ids_.count(vec_id)) continue;
            double dist = KMeans::distance(query, data[vec_id]);
            pq.emplace(vec_id, dist);
            if (pq.size() > k) pq.pop();
        }
    }
    
    std::vector<int> result;
    while (!pq.empty()) {
        result.push_back(pq.top().id);
        pq.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// void IVFIndex::insert(const Vector& vec) {
//     // 寻找有效聚类中心
//     int nearest_cluster = -1;
//     double min_dist = std::numeric_limits<double>::max();
//     for (int c = 0; c < nlist; ++c) {
//         if (inverted_lists_[c].empty()) continue;
//         double dist = KMeans::distance(vec, centroids[c]);
//         if (dist < min_dist) {
//             min_dist = dist;
//             nearest_cluster = c;
//         }
//     }

//     // 处理全空情况
//     if (nearest_cluster == -1) {
//         nearest_cluster = 0;
//         centroids[0] = vec;
//     }

//     // 添加新数据
//     data.push_back(vec);
//     int new_id = data.size() - 1;
//     inverted_lists_[nearest_cluster].push_back(new_id);

//     // 重新计算聚类中心
//     update_centroid(nearest_cluster);

//     // 更新partition
//     partition_metadata_[nearest_cluster].size++;
//     partition_metadata_[nearest_cluster].current_centroid = centroids[nearest_cluster];
// }

// void IVFIndex::remove(int vec_id) {
//     if (vec_id < 0 || vec_id >= static_cast<int>(data.size())) return;
    
//     // 标记逻辑删除
//     deleted_ids_.insert(vec_id);
//     centroids_updated_ = false;

//     // 触发增量重分配
//     if (deleted_ids_.size() > data.size() / 100) {
//         incremental_reassign();
//     }
// }

void IVFIndex::update_temperature(const Vector& query, std::vector<int>& nearest_clusters) {
    float heating_factor = 0.1f;
    float cooling_factor = 0.05f;
    std::vector<float> cluster_distances;
    for(int c : nearest_clusters)
    {
        cluster_distances.push_back(KMeans::distance(query, centroids[c]));
    }
    float min_distance = *std::min_element(cluster_distances.begin(),cluster_distances.end());
    for (int i = 0; i < nlist; ++i) {
        bool accessed = std::find(nearest_clusters.begin(), nearest_clusters.end(), i) != nearest_clusters.end();
        // if accessed by query, increase temperature; otherwise decrease temperature
        if (accessed) {
            partition_metadata_[i].temperature *= 
                (1.0f + (min_distance / (KMeans::distance(query, centroids[i])) * heating_factor));
        } else {
            partition_metadata_[i].temperature = std::max(1.0f, partition_metadata_[i].temperature * (1.0f - cooling_factor));
        }
    }
}

float IVFIndex::calculate_reindexing_score(int cluster_id) {
    float alpha = 0.1f;
    float beta = 0.5f;

    // fT 温度函数
    float temperature_term = partition_metadata_[cluster_id].temperature * alpha;
    // fs 大小偏差函数
    float size_deviation = partition_metadata_[cluster_id].size >= target_partition_size_?
                           (partition_metadata_[cluster_id].size - target_partition_size_) / target_partition_size_ :
                           (target_partition_size_ - partition_metadata_[cluster_id].size) / partition_metadata_[cluster_id].size;
    // fd 漂移函数
    float drift = KMeans::distance(partition_metadata_[cluster_id].initial_centroid, partition_metadata_[cluster_id].current_centroid) / 
                  KMeans::distance(partition_metadata_[cluster_id].initial_centroid, std::vector<float>(partition_metadata_[cluster_id].initial_centroid.size(), 0.0f));

    return temperature_term * (beta * size_deviation + (1 - beta) * drift);
}

// 改写增量更新
void IVFIndex::update_centroid(int cluster_id) {
    Vector new_centroid(centroids[0].size(), 0.0);
    int count = 0;
    
    for (int vec_id : inverted_lists_[cluster_id]) {
        if (deleted_ids_.count(vec_id)) continue;
        for (size_t i = 0; i < data[vec_id].size(); ++i) {
            new_centroid[i] += data[vec_id][i];
        }
        count++;
    }
    
    if (count > 0) {
        for (auto& val : new_centroid) val /= count;
        centroids[cluster_id] = new_centroid;
    }
}

void IVFIndex::local_reindex(const std::vector<int>& clusters_to_reindex) {
    
}

// void IVFIndex::incremental_reassign() {
//     // 重建有效数据集
//     DataSet valid_data;
//     std::vector<int> id_map(data.size());
    
//     int new_id = 0;
//     for (size_t i = 0; i < data.size(); ++i) {
//         if (!deleted_ids_.count(i)) {
//             valid_data.push_back(data[i]);
//             id_map[i] = new_id++;
//         }
//     }
    
//     // 更新倒排列表
//     for (auto& list : inverted_lists_) {
//         std::vector<int> new_list;
//         for (int id : list) {
//             if (!deleted_ids_.count(id)) {
//                 new_list.push_back(id_map[id]);
//             }
//         }
//         list = new_list;
//     }
    
//     // 更新数据集
//     data = valid_data;
//     deleted_ids_.clear();
    
//     kmeans.fit(data);
//     centroids = kmeans.get_centroids();
// }

// void IVFIndex::split_cluster(int cluster_id) {
//     validate_cluster(cluster_id);
//     if (inverted_lists_[cluster_id].size() < 2) return;

//     // 准备有效数据
//     DataSet valid_data;
//     for (int vec_id : inverted_lists_[cluster_id]) {
//         if (!deleted_ids_.count(vec_id)) {
//             valid_data.push_back(data[vec_id]);
//         }
//     }

//     KMeans sub_kmeans(2, 50, false);
//     sub_kmeans.fit(valid_data);
//     auto& sub_labels = sub_kmeans.get_labels();

//     // 创建新聚类
//     nlist++;
//     centroids.push_back(sub_kmeans.get_centroids()[1]);
//     inverted_lists_.emplace_back();

//     // 重新分配向量
//     std::vector<int> new_list;
//     size_t label_idx = 0;
//     for (int vec_id : inverted_lists_[cluster_id]) {
//         if (deleted_ids_.count(vec_id)) continue;
//         if (sub_labels[label_idx++] == 0) {
//             new_list.push_back(vec_id);
//         } else {
//             inverted_lists_.back().push_back(vec_id);
//         }
//     }
//     inverted_lists_[cluster_id] = new_list;
//     centroids[cluster_id] = sub_kmeans.get_centroids()[0];
// }

// void IVFIndex::merge_clusters(int cluster_id1, int cluster_id2) {
//     validate_cluster(cluster_id1);
//     validate_cluster(cluster_id2);

//     // 合并列表
//     inverted_lists_[cluster_id1].insert(
//         inverted_lists_[cluster_id1].end(),
//         inverted_lists_[cluster_id2].begin(),
//         inverted_lists_[cluster_id2].end()
//     );
    
//     // 更新中心点
//     update_centroid(cluster_id1);
    
//     // 清理被合并的聚类
//     inverted_lists_[cluster_id2].clear();
//     centroids.erase(centroids.begin() + cluster_id2);
//     inverted_lists_.erase(inverted_lists_.begin() + cluster_id2);
//     nlist--;
// }

void IVFIndex::validate_cluster(int cluster_id) {
    if (cluster_id < 0 || cluster_id >= nlist) {
        throw std::runtime_error("Invalid cluster ID");
    }
}