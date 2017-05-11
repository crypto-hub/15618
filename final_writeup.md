## FINAL WRITEUP

## SUMMARY

We implemented concurrent in-memory hash table based on optimistic cuckoo hashing[1].  This hash table is optimized for high space efficiency and high read throughput. Therefore, it is best suited for read heavy workload with multiple readers and single writer use-cases. We have incorporated improvements mentioned in MemC3 by Fan et al[1], and we were able to achieve similar performance numbers (95% space efficiency and read lookup throughput of ~21 Million Ops per sec). We developed and tested our implementation on Latedays cluster (Intel Xeon CPU E5-2620).

## RESULTS

We analyzed how various optimizations techniques contribute to the high throughput and space efficiency of our hash table’s implementation. We compared its performance with various other hash tables’ implementation, which selects different tradeoff point in design space. Finally, we compared our implementation’s performance with the performance results mentioned in reference paper [1].

### Platform

We developed and tested our implementation on Latedays cluster, which consists of Intel Xeon CPU E5-2620 v3 @ 2.40GHz, with 15 MB cache. This configuration is quite similar to the ones which were used for experiments in reference paper, which had Intel Xeon L5640 @ 2.27GHz, with 12 MB cache.

### Overall Comparison of Various Hash Table Approaches

We analyzed various hash table approaches both for single threaded and multi-threaded scenarios. Following figures demonstrate how various factors contribute towards the read throughput performance. We concluded cuckoo hash table with optimistic locking mechanism and tag byte gives the best performance for read heavy workloads. In subsequent sections we will analyze each of performance optimization in depth.

<img src="multi_factor_analysis.png" width="700">
<img src="single_factor_analysis.png" width="700">


### Optimistic Locking Mechanism

We implemented two kinds of locking mechanism for cuckoo hash table. One is per bucket locking, whereas other is optimistic locking based on version counter. Optimistic locking performs significantly better than bucket based locking. It also beats chaining based hash table with coarse grain as well as fine grain locking by a big margin. As in optimistic locking, there is no need to take any kind of locks, it allows multiple concurrent reads and reduces significant overhead associated with locking. As it is meant for read heavy workload, the overhead associated with read retries during read-write conflict, doesn’t hurt much overall throughput.

<img src="multi_locking.png" width="700">

### Cache Locality Improvement Using Tag

Introduction of tag significantly boost the throughput of the hash table, both for read only as well as read-write work load, as illustrated in the figure. The main reason for this boost is cache locality. During lookup, it needs to check for the key in all the slots of both buckets. First of all, the 1-byte tag eliminates the need for fetching complete variable length key from memory for most slot checks, except when there is tag hit. As the cache size is only 1 byte, as compared to variable length key, it makes sure that complete bucket fits in a single cache line. Similarly, it helps during path search phase of insertion. Using the tag which resides in cache, instead of complete key which may reside in memory, for finding the next bucket, significantly speedup the insertion.

<img src="multi_tag1.png" width="700">
<img src="multi_tag2.png" width="700">

### Space Efficiency

Our cuckoo hash table was able to achieve as high occupancy as ~95%, without compromising on O (1) worst case lookup. Each key can be mapped to 2 buckets, each having 4 slots. Cuckoo path length of 500 was used as an upper bound on no. of consecutive displacements in search for an empty slot for insert, before declaring it as unsuccessful insert. 

Also, as compared to chaining based hash table, it has lower overhead for bookkeeping information. Hash table with chaining needs to store 4 or 8-bytes pointer, based on machine type, to the next item in the bucket. On the other hand, cuckoo hash table just needs to store 1-byte tag for faster lookup and inserts.

### Analysis of write heavy workload

As load factor increases, the length of cuckoo path to find an empty slot increases. Therefore, insertion time gradually increases on average, with increase in occupancy of hash table. As a result, overall throughput, i.e. requests per sec, decreases with increase in percentage of write request in load, as illustrated in figure. However, as optimistic cuckoo hashing is targeted towards read heavy workload, this trade-off doesn’t hurt its performance goal significantly.

<img src="througput_vs_write.png" width="700">

### Scalability and Threads

As number of threads increases, throughput in case of read only as read-write workload increases. As there is no lock involved in optimistic cuckoo hashing, it doesn’t suffer from issue of lock contention. For chaining based hash table with coarse grain as well as fine grain locking, throughput doesn’t increase with no. of threads. This is because operations are serialized and there is significant issue of lock contention. 

<img src="thread_vs_thr.png" width="700">
<img src="read_througput.png" width="700">


### Comparison with Reference Paper

We incorporated improvements mentioned in MemC3 by Fan et al [1], and we were able to achieve similar performance numbers, as reported in paper. As mentioned in earlier section, we tested our implementation on similar configuration as mentioned in reference paper.

<img src="comparison_with_fen.png" width="700">


## References
[1] https://www.cs.cmu.edu/~dga/papers/memc3-nsdi2013.pdf <br>
[2] https://www.cs.princeton.edu/~mfreed/docs/cuckoo-eurosys14.pdf

## Work By Each Student
Equal work was performed by both project members.
