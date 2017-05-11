#ifndef __CUCKOO_HASHTABLE_H__
#define __CUCKOO_HASHTABLE_H__

#include "hashtable.h"


#define MAX_CUCKOO_PATH_LEN (500)
#define ASSOCOTIVITY 2 

#define CHECK_EMPTY_SLOT(a) ((a.ptr)?false:true)

struct swap_pair_t{	

	int bucket;
	int slot;
	slot_t slot_val;

}; 

struct cuckoo_path_t{

	swap_pair_t path[MAX_CUCKOO_PATH_LEN];
	int path_index;

};


void check_version_arr(hashtable_t *ht);


#endif //__CUCKOO_HASHTABLE_H__