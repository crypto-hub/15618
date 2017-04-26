## CHECKPOINT REPORT

### Updated schedule

- 4/25 - 5/1: 
- Implement basic version of in-memory hashmap using cuckoo hashing for a single node.
- Implement optimistic concurrent cuckoo hashmap using key-version counter.

- 5/2 - 5/8: 
- Complete and optimise in-memory hashmap using cuckoo hashing for a single node.
- Scale this hashtable in distributed environment (Hope to achieve) 

- 5/9 - 5/11: 
- Final optimisation and report preparation.


### Work completed so far
- We have completed literature survey. We read the research papers related to cuckoo hashing and decided on what we need to implement exactly.
- We wrote the baseline implementation for comparison using unordered_map in C++.
- We finalised the pseudo code for implementing cuckoo hashing on a single node. 


### Presentation format
- We will present a bunch of graphs to show our benchmarking results. As this hashing algorithm is specialised for heavy read-request load, we will try to compare our performance with various other hashing libraries and key value stores.

### Current Issues
- We are still investigating on how we can integrate cache optimisations mentioned in the paper with our implementation. Even after implementing cache optimisations, we are having trouble in figuring how to get efficient statistics on cache misses and hits.

