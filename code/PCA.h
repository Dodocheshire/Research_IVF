// pca.h
#ifndef PCA_H
#define PCA_H
#include <vector>
using Vector = std::vector<float>;
using DataSet = std::vector<Vector>;

class PCA {
public:
    PCA(int n_components);
    void fit(const DataSet& data);
    DataSet transform(const DataSet& data) const;
    Vector transform(const Vector& vec) const;
    int get_dim() const { return n_components; }
    
private:
    int n_components;
    Vector mean;
    std::vector<Vector> components;
};
#endif