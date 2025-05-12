#ifndef UTILSHDF5_H
#define UTILSHDF5_H
#include <string>
#include <vector>

using Vector = std::vector<float>;
using Dataset = std::vector<Vector>;

Dataset load_data_hdf5(
    const std::string& filename,
    const std::string& dataset_name
);
#endif