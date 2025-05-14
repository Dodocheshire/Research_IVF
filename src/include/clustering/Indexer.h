#ifndef INDEXER_H
#define INDEXER_H

#include <vector>
#include "Partition.h"

// interface
class Indexer {

public:
    virtual ~Indexer() = default;

    virtual void build(std::vector<std::vector<float>>& data, std::vector<Partition*>& partitions) = 0;

    virtual void reindex(std::vector<Partition*>& violators, std::vector<Partition*>& partitions) = 0;

};

#endif