#ifndef K_MEANS_H
#define K_MEANS_H
#include <vector>

using Vector = std::vector<float>;
using DataSet = std::vector<Vector>;

class KMeans {
public:
    KMeans(int k_, int max_iter_ = 20, bool verbose_ = false);
    void fit(const DataSet& data);
    const DataSet& get_centroids() const { return centroids; }
    const std::vector<int>& get_labels() const { return labels; }
    static double distance(const Vector& a, const Vector& b);

private:
    int k;
    int max_iter;
    bool verbose;
    DataSet centroids;
    std::vector<int> labels;
    
    void init_centroids(const DataSet& data);
    bool update_centroids(const DataSet& data);
};
#endif