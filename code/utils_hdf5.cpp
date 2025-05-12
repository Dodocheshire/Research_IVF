#include "utils_hdf5.hpp"
#include <fstream>
#include <iostream>
#include <H5Cpp.h>
#include <cstdint>

/*
HDF5 "sift-128-euclidean.hdf5" {
    FILE_CONTENTS {
     group      /
     dataset    /distances(float)
     dataset    /neighbors(int)
     dataset    /test(int)
     dataset    /train(int)
    }
}
*/

Dataset load_data_hdf5(
    const std::string &filename,
    const std::string &dataset_name)
{
    try
    {
        // open hdf5
        H5::H5File in(filename, H5F_ACC_RDONLY);
        // open group
        H5::DataSet dataset = in.openDataSet(dataset_name);
        H5::DataSpace dataspace = dataset.getSpace();

        // get information of dimension
        hsize_t dims[2];
        dataspace.getSimpleExtentDims(dims, nullptr);
        size_t num_vectors = dims[0];
        size_t dim = dims[1];

        std::vector<float> buffer(num_vectors * dim);
        // read data into buffer
        dataset.read(buffer.data(), H5::PredType::NATIVE_FLOAT);
        
        //to format of Dataset
        Dataset data;
        data.reserve(num_vectors);
        for (size_t i = 0; i < num_vectors; ++i)
        {
            Vector vec(
                buffer.begin() + i * dim,
                buffer.begin() + (i + 1) * dim);
            data.emplace_back(std::move(vec));
        }

        std::cout << "Loaded SIFT dataset '" << dataset_name
                  << "' (" << num_vectors << "*" << dim << ")"
                  << std::endl;
        return data;
    } catch (const H5::Exception& e){
        std::cerr << "HDF5 Error: " << e.getCDetailMsg() << std::endl;
        throw std::runtime_error("Failed to load HDF5 data");
    }
}
