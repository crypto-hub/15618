## FINAL WRITEUP

### Summary

We implemented concurrent in-memory hash table based on optimistic cuckoo hashing[1].  This hash table is optimized for high space efficiency and high read throughput. Therefore, it is best suited for read heavy workload with multiple readers and single writer use-cases. We have incorporated improvements mentioned in MemC3 by Fan et al[1], and we were able to achieve similar performance numbers (95% space efficiency and read lookup throughput of ~21 Million Ops per sec). We developed and tested our implementation on Latedays cluster (Intel Xeon CPU E5-2620).


### Overview
<img src="multi_factor_analysis.png" width="500">
<img src="single_factor_analysis.png" width="500">


### Comparison of Locking Mechanism
<img src="multi_locking.png" width="500">


### Impact of Tag Usage
<img src="multi_tag1.png" width="500">
<img src="multi_tag2.png" width="500">

### No. of Threads vs Throughput
<img src="thread_vs_thr.png" width="500">

### Write Percentage vs Throughput
<img src="througput_vs_write.png" width="500">

### Read Throughput : Single vs MultiThreading
<img src="read_througput.png" width="500">


### Comparison with Reference Paper[1]
<img src="comparison_with_fen.png" width="500">


### References
[1] https://www.cs.cmu.edu/~dga/papers/memc3-nsdi2013.pdf