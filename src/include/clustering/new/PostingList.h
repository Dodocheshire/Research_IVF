// posting_list.h
#pragma once

#include <unordered_map>
#include <vector>
#include <cstddef>

class PostingList {
public:
    void Add(size_t cluster_id, size_t vec_id, const std::vector<float>& vec);
    void Remove(size_t vec_id);
    std::vector<std::pair<size_t, std::vector<float>>> GetClusterList(size_t cluster_id) const;

    const std::vector<float>* GetVector(size_t vec_id) const;

private:
    // 每个聚类的倒排表：vector of (vec_id, vec)
    std::unordered_map<size_t, std::vector<std::pair<size_t, std::vector<float>>>> lists_;
    // 快速查找vec_id -> cluster_id，用于删除等操作
    std::unordered_map<size_t, size_t> id_to_cluster_;
};
