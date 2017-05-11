CPP_CC=g++

CPPFLAGS=-O3 -std=c++11 -Wall -Wextra -lpthread 


####### cuckoo - version counters 

#no tags 
#CPP_SRCS = hash.cpp optimistic_cuckoo_keyhash1.cpp perf_measurement.cpp

#tags used only for lookups, not for inserts 
#CPP_SRCS = hash.cpp optimistic_cuckoo_keyhash.cpp perf_measurement.cpp

# tags for both 
CPP_SRCS = hash.cpp optimistic_cuckoo_taghash.cpp perf_measurement.cpp

# tags used only for inserts, not for lookups
#CPP_SRCS = hash.cpp optimistic_cuckoo_taghash1.cpp perf_measurement.cpp


############## ####### ####### ####### ####### ####### ####### ####
#cuckoo - bucket locks

# tags only in inserts not in lookup
#CPP_SRCS = hash.cpp cuckoo_finegrain2.cpp perf_measurement.cpp   

#no tags 
#CPP_SRCS = hash.cpp cuckoo_finegrain_notags.cpp perf_measurement.cpp

#tags only in lookups, not in inserts (next bucket based on key)
#CPP_SRCS = hash.cpp cuckoo_finegrain1.cpp perf_measurement.cpp

#tags used for both 
#CPP_SRCS = hash.cpp cuckoo_finegrain.cpp perf_measurement.cpp

############## ####### ####### ####### ####### ####### ####### ####
# cuckoo - single threaded

#cuckoo + tags (only lookup) 
#CPP_SRCS = hash.cpp cuckoo_single1.cpp perf_measurement.cpp  


# cuckoo + no tags 
#CPP_SRCS = hash.cpp cuckoo_single2.cpp perf_measurement.cpp  

# cuckoo + tags for both 
#CPP_SRCS = hash.cpp cuckoo_single3.cpp perf_measurement.cpp  



#cuckoo + tags (only insert) 
#CPP_SRCS = hash.cpp cuckoo_single4.cpp perf_measurement.cpp  




############## ####### ####### ####### ####### ####### ####### ####
#chaining 

#CPP_SRCS = hash.cpp chaining_coarsegrain.cpp perf_measurement.cpp
#CPP_SRCS = hash.cpp chaining_finegrain.cpp perf_measurement.cpp
#CPP_SRCS = hash.cpp chaining_single.cpp perf_measurement.cpp



############## ####### ####### ####### ####### ####### ####### ####

#CPP_SRCS = hash.cpp unordered_map_hash.cpp perf_measurement.cpp
#CPP_SRCS = hash.cpp unordered_map_single.cpp perf_measurement.cpp


CPP_OBJS = $(CPP_SRCS:.cpp=.o)


TARGET = driver

all: $(TARGET)
 
$(TARGET): $(CPP_OBJS)
	$(CPP_CC) $(CPPFLAGS) -o $(TARGET) $(CPP_OBJS)

.cpp.o:
	$(CPP_CC) $(CPPFLAGS)  -c $<  -o $@


clean:
	$(RM) *.o *~ $(TARGET)