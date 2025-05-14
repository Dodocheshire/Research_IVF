#ifndef PARTITION_H
#define PARTITION_H
#include <vector>
#include "NullBitmap.h"
#include <iostream>
#include <cassert>
#include <unordered_map>
#include "define.h"
#include <stdexcept>

/*
    追踪 size、centroid、temperature、reindex_score
*/
class PartitionMetadata {
    friend Partition;
public:
    void serializeTo(std::ostream &os) const {
        os.write(reinterpret_cast<const char*>(&size_), sizeof(size_));

        uint32_t init_size = initial_centroid_.size();
        os.write(reinterpret_cast<const char*>(&init_size), sizeof(init_size));
        os.write(reinterpret_cast<const char*>(initial_centroid_.data()), init_size * sizeof(float));

        uint32_t curr_size = current_centroid_.size();
        assert(init_size == curr_size);
        os.write(reinterpret_cast<const char*>(&curr_size), sizeof(curr_size));
        os.write(reinterpret_cast<const char*>(current_centroid_.data()), curr_size * sizeof(float));

        os.write(reinterpret_cast<const char*>(&temperature_), sizeof(temperature_));
        os.write(reinterpret_cast<const char*>(&reindexing_score_), sizeof(reindexing_score_));

    }
    
    void deserializeFrom(std::istream &is) {
        is.read(reinterpret_cast<char*>(&size_), sizeof(size_));

        size_t init_size;
        is.read(reinterpret_cast<char*>(&init_size), sizeof(init_size));
        initial_centroid_.resize(init_size);
        is.read(reinterpret_cast<char*>(initial_centroid_.data()), init_size * sizeof(float));

        size_t curr_size;
        assert(init_size == curr_size);
        is.read(reinterpret_cast<char*>(&curr_size), sizeof(curr_size));
        current_centroid_.resize(curr_size);
        is.read(reinterpret_cast<char*>(current_centroid_.data()), curr_size * sizeof(float));

        is.read(reinterpret_cast<char*>(&temperature_), sizeof(temperature_));
        is.read(reinterpret_cast<char*>(&reindexing_score_), sizeof(reindexing_score_));
    }


private:
    uint32_t size_ = 0; //记录分区中有效的数据个数
    std::vector<float> initial_centroid_; // 每次重建时更新
    std::vector<float> current_centroid_; // 每次insert或delete时更新
    float temperature_ = 0;
    float reindexing_score_ = 0;
};



/*
    封装每个分区的向量集合及其状态
*/
class Partition {
    friend AdaIVF;
public:
    Partition(int partition_id);

    void insert(const std::vector<float> v, vector_id_t id);

    void remove(vector_id_t id);

    void update_centroid();


    void clear();

    const PartitionMetadata* getMetaData() const {
        return &partition_meta_;
    }

    std::vector<std::vector<float>>& getData() {
        return data_;
    }

    /**
     * @brief 序列化PartitionMetadata并根据nullbitmap序列化data_
     */
    void serializeTo(std::ostream &os) const;

    /**
     * @brief 反序列化PartitionMetadata并根据nullbitmap反序列化data_
     */
    void serializeFrom(std::istream &is);
    

private:
    partition_id_t partition_id_; // uniquely identify the partition
    PartitionMetadata partition_meta_; // the meta data

    std::vector<std::vector<float>> data_; // the actual data (with deleted ones)
    std::vector<vector_id_t> vector_ids; // the ids of vectors in the partition 
    std::unordered_map<vector_id_t, index_t> id_to_index; // quickly look up vectors in partition given id(向量在data_中的位置)

    NullBitmap null_bitmap_; // null_bitmap[i] == 1 -> ith vector in data_ is deleted
};

#endif