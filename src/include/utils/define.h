#include <vector>
#include <cassert>

using vector_id_t = int;
using index_t = int;
using cluster_id_t = int;
using partition_id_t = int;
using data_t = std::vector<float>;

float Distance(const std::vector<float>& v1, const std::vector<float>& v2) {
    assert(v1.size() > 0 && v1.size() == v2.size()); //测时间时删去
    double dist = 0;
    for(int i = 0; i < v1.size(); ++i) {
        dist += (v1[i] - v2[i]) * (v1[i] - v2[i]);
    }
    return sqrt(dist);
}