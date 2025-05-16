#ifndef INDEXER_H
#define INDEXER_H

#include <vector>
#include "Partition.h"

// interface
class Indexer {

public:
    virtual ~Indexer() = default;

    virtual void build(
        std::vector<std::vector<float>>& data, 
        std::vector<Partition*>& partitions, 
        std::unordered_map<vector_id_t, partition_id_t>& vid_to_pid) = 0;

    virtual void reindex(
        std::vector<partition_id_t>& violators, 
        std::vector<Partition*>& partitions, 
        std::unordered_map<vector_id_t, partition_id_t>& vid_to_pid) = 0;

};

#endif