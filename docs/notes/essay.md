## Ada-IVF background
- search performance measure
    1. **partition imbalance**: uneven size distribution of partitions
    > Solution: use balanced k-means to produce roughly equal-sized partitions
    2. **reconstruction error**：average distance from each vector to its nearest centroid which means less compact clusters
    > 
**conclusion1**: partition imbalance and reconstruction error must be minimized to
optimize search performance in IVF indexes.

- simply modifying partitions of the IVF index via partition append and deletes can support update operations, but will lead to an increase in partition imbalance and reconstruction error
- state of the art incremental IVF indexing strategy LIRE, incurs unnecessary maintenance overhead by over-triggering reindexing for **rarely accessed partitions** because in real world workload, read distribution is skewed and many partitions are modified but not read from
- compared to LIRE : 1.achieve similar QPS 2. creates 1/3 as many partitions
and requires 1/4 the update time due to its maintenance of
partitions that are not accessed by the queries
> with 80% of updates affecting partitions that have not been accessed by search
operations and 96% affecting partitions that are accessed by fewer
than ten search queries. Inspiration: **utilize read locality**, **lazily reindexes partitions
as queries access them**

## Ada-IVF overview
- updates by selectively reindexing a subset of partitions that negatively contribute to the partition imbalance and reconstruction error,
### Step
1. Maintain **index metadata**. Including size(initital/current), centroid, read temperature and reindexing score of partitions.
> Search operations modify the temperature
(S), and update operations modify the partition contents,
size, and centroid (U, A, B).
2. After each update, monitor index partitions for increases in imbalance and reconstruction error
> s for increases in imbalance and reconstruction error. Partitions are assigned a reindexing score (C), which is a function of imbalance and reconstruction error.Partitions exceeding a score threshold of $𝜏_f$ are violators
3. Reindex violating partitions using **local reindexing**. 
> Violating partitions are **split and merged with neighboring partitions(D) using balanced kmeans** to minimize imbalance and reconstruction error

### Track index metadata
1. The temperature is increased for partitions accessed by S and decreased for all nonaccessed partitions
2. The update operation U modifies the size,
centroid, and reindexing score of a set of partitions.The centroids of modified partitions
 are **updated incrementally** rather than computingthe mean over all vectors in the partition.

 ### update manager
- Update Manager. Ada-IVF’s update manager consists of a reindexing policy that identifies which and when partitions should be reindexed and a mechanism for reindexing them.
- The reindexing policy is informed by two indicator functions:
 **local and global indicator functions**, where both functions are evaluated after each update.
> The local indicator function computes a reindexing score indicating if a specific partition 𝑐 is imbalanced has drifted from its initial state.The global indicator function 𝐺(𝐼) acts as a fail-safe procedure to ensure that our local reindexing actions don’t degrade imbalance or reconstruction error globally
- Indicator function values are checked after **a batch of updates** are issued to the index
- The entire index is rebuilt if the global indicator exceeds a threshold
- the nearest 𝑟𝑐 partitions to the violator are selected based on centroid distance. The split partitions and neighboring partitions are then iteratively refined using a clustering algorithm.