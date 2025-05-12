// pca.cpp
#include "PCA.h"
#include <Eigen/Dense>
#include <numeric>  // 新增头文件用于特征值排序

PCA::PCA(int n_components) : n_components(n_components) {}

void PCA::fit(const DataSet& data) {
    if (data.empty() || data[0].empty()) {
        throw std::invalid_argument("Input data cannot be empty");
    }
    const int rows = data.size();
    const int cols = data[0].size();
    
    // 转换为Eigen矩阵
    Eigen::MatrixXf mat(rows, cols);
    for (int i = 0; i < rows; ++i)
        mat.row(i) = Eigen::VectorXf::Map(data[i].data(), cols);
    
    // 计算均值
    mean = Vector(cols, 0);
    for (const auto& vec : data)
        for (int j = 0; j < cols; ++j) mean[j] += vec[j];
    for (auto& m : mean) m /= rows;
    
    // 中心化（已修复行向量问题）
    mat.rowwise() -= Eigen::RowVectorXf::Map(mean.data(), cols);
    
    // 计算协方差矩阵
    Eigen::MatrixXf cov = (mat.adjoint() * mat) / (rows - 1);
    
    // 特征分解
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> solver(cov);
    Eigen::VectorXf eigenvalues = solver.eigenvalues();
    Eigen::MatrixXf eigen_vectors = solver.eigenvectors();
    
    // 特征值排序（新增稳定性改进）
    std::vector<int> indices(cols);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
        [&](int a, int b) { return eigenvalues[a] > eigenvalues[b]; });
    
    // 取前n_components个特征向量
    components.resize(n_components);
    for (int i = 0; i < n_components; ++i) {
        components[i] = Vector(cols);
        Eigen::VectorXf v = eigen_vectors.col(indices[i]);
        std::copy(v.data(), v.data() + cols, components[i].begin());
    }
}

DataSet PCA::transform(const DataSet& data) const {
    DataSet result;
    for (const auto& vec : data)
        result.push_back(transform(vec));
    return result;
}

Vector PCA::transform(const Vector& vec) const {
    Vector proj(n_components, 0);
    Vector centered(vec.size());
    for (size_t i = 0; i < vec.size(); ++i)
        centered[i] = vec[i] - mean[i];
    
    for (int i = 0; i < n_components; ++i)
        for (size_t j = 0; j < centered.size(); ++j)
            proj[i] += centered[j] * components[i][j];
    
    return proj;
}