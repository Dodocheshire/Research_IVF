#include "IVF.h"
#include "utils.hpp"
#include "utils_hdf5.hpp"
#include "PCA.h"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>
#include <omp.h>

int main() {
    try {
        // SIFT
        Dataset base = load_data_hdf5("sift-128-euclidean.hdf5", "/train");
        Dataset queries = load_data_hdf5("sift-128-euclidean.hdf5", "/test");
        Dataset neighbors = load_data_hdf5("sift-128-euclidean.hdf5", "/neighbors");

        omp_set_num_threads(32); // set limited thread

        // query quantity
        float percent = 0.01f;
        int k = static_cast<int>(base.size() * percent / 100.0f);
        k = std::max(1, k);

        // recall quantity
        int k_recall = 100;
        k_recall = std::min(100, k_recall);
        k_recall = std::min(k, k_recall);

        // ==== PCA降维处理 ====
        auto start = std::chrono::high_resolution_clock::now();
        const int original_dim = 128;
        const int pca_dim = 96;
        PCA pca(pca_dim);

        // 仅在训练集(base)上拟合PCA模型
        std::cout << "Fitting PCA...\n";
        pca.fit(base);

        // 转换所有数据到低维空间
        std::cout << "Transforming data...\n";
        Dataset base_lowdim = pca.transform(base);
        Dataset queries_lowdim = pca.transform(queries);
        auto PCA_time = std::chrono::high_resolution_clock::now() - start;

        // 使用降维后的数据构建IVF索引
        start = std::chrono::high_resolution_clock::now();
        IVFIndex index(256, 50);
        index.build(base_lowdim);
        auto build_time = std::chrono::high_resolution_clock::now() - start;

        // 随机选择100个查询索引
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, queries_lowdim.size() - 1);

        // 第一次查询100组向量
        double total_query_time_before_update = 0;
        double total_recall_before_update = 0;
        for (int i = 0; i < 100; ++i) {
            int test_query = dis(gen);

            std::vector<int> gt_neighbors;
            const Vector& raw_neighbors = neighbors[test_query];
            for (int j = 0; j < k_recall; ++j) {
                gt_neighbors.push_back(int(raw_neighbors[j]));
            }

            start = std::chrono::high_resolution_clock::now();
            auto results = index.search(queries_lowdim[test_query], k);
            if (results.empty()) {
                throw std::runtime_error("Search returned empty results");
            }
            auto query_time = std::chrono::high_resolution_clock::now() - start;
            total_query_time_before_update += std::chrono::duration<double>(query_time).count();

            // 计算recall
            int recall_count = 0;
            for (int j = 0; j < k_recall; ++j) {
                if (std::find(gt_neighbors.begin(), gt_neighbors.end(), results[j]) != gt_neighbors.end()) {
                    recall_count++;
                }
            }
            float recall = float(recall_count) / float(k_recall);
            total_recall_before_update += recall;
        }
        double average_query_time_before_update = total_query_time_before_update / 100;
        double average_recall_before_update = total_recall_before_update / 100;

        // 更新操作
        start = std::chrono::high_resolution_clock::now();
        // 测试插入操作
        Vector new_vec(pca_dim, 0.1);
        index.insert(new_vec);
        // 测试删除操作
        auto prev_results = index.search(queries_lowdim[dis(gen)], k);
        if (!prev_results.empty()) {
            index.remove(prev_results[0]);
        }
        // 测试分裂操作
        index.split_cluster(0);
        // 测试合并操作
        index.merge_clusters(0, 1);
        auto update_time = std::chrono::high_resolution_clock::now() - start;

        // 第二次查询100组向量
        double total_query_time_after_update = 0;
        double total_recall_after_update = 0;
        for (int i = 0; i < 100; ++i) {
            int test_query = dis(gen);

            std::vector<int> gt_neighbors;
            const Vector& raw_neighbors = neighbors[test_query];
            for (int j = 0; j < k_recall; ++j) {
                gt_neighbors.push_back(int(raw_neighbors[j]));
            }

            start = std::chrono::high_resolution_clock::now();
            auto results = index.search(queries_lowdim[test_query], k);
            if (results.empty()) {
                throw std::runtime_error("Search returned empty results");
            }
            auto query_time = std::chrono::high_resolution_clock::now() - start;
            total_query_time_after_update += std::chrono::duration<double>(query_time).count();

            // 计算recall
            int recall_count = 0;
            for (int j = 0; j < k_recall; ++j) {
                if (std::find(gt_neighbors.begin(), gt_neighbors.end(), results[j]) != gt_neighbors.end()) {
                    recall_count++;
                }
            }
            float recall = float(recall_count) / float(k_recall);
            total_recall_after_update += recall;
        }
        double average_query_time_after_update = total_query_time_after_update / 100;
        double average_recall_after_update = total_recall_after_update / 100;

        // 输出性能结果
        std::cout << "\n=== Performance With PCA===" << std::endl;
        std::cout << "Original dimension: " << original_dim
                  << " -> Reduced dimension: " << pca_dim << std::endl;
        std::cout << "PCA time: "
                  << std::chrono::duration<double>(PCA_time).count()
                  << "s" << std::endl;
        std::cout << "Build time: "
                  << std::chrono::duration<double>(build_time).count()
                  << "s" << std::endl;
        std::cout << "Update time: "
                  << std::chrono::duration<double>(update_time).count()
                  << "s" << std::endl;

        // 输出查询时间和recall对比
        std::cout << "\n=== Query Time Comparison ===" << std::endl;
        std::cout << "Average query time before update: " << average_query_time_before_update << "s" << std::endl;
        std::cout << "Average query time after update: " << average_query_time_after_update << "s" << std::endl;

        std::cout << "\n=== Recall Comparison ===" << std::endl;
        std::cout << "Average recall before update: " << average_recall_before_update << std::endl;
        std::cout << "Average recall after update: " << average_recall_after_update << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}