// vector_router.h
#pragma once

#include <vector>
#include <cstddef>
#include "ClusterManager.h"

class VectorRouter {
public:
    VectorRouter(size_t topk);

    std::vector<size_t> Route(const std::vector<float>& vec, const ClusterManager& cm) const;

private:
    size_t routing_k_; // 查询/插入时使用的 top-k 路由分配数
};
