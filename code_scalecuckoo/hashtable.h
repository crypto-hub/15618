#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include "hash.h"

#define SLOTS_PER_BUCKET (4)

//#define DEFAULT_VERSION_COUNTER_POWER  (2)
//#define DEFAULT_VERSION_COUNTERS  (4)
#define DEFAULT_VERSION_COUNTER_POWER  (13)


//#define DEFAULT_BUCKETS_POWER (3)
#define DEFAULT_BUCKETS_POWER (15)





using namespace std;

struct kv_obj_t{

	string key;
	string val;


};



struct slot_t{

	unsigned char tag;
	kv_obj_t *ptr;

};



struct bucket_t{

	slot_t slots[SLOTS_PER_BUCKET];
	pthread_mutex_t lock;

};


struct hashtable_t{	

	bucket_t *table;
	int *version_counters;
	int total_buckets;
	int total_version_counters;
	pthread_mutex_t writer_mutex;
	unsigned char *bitmap;
	unsigned bitmap_size;
};

hashtable_t* init_hash_table(int total_buckets, int version_counters);
bool insert(hashtable_t *ht, kv_obj_t *kv);
kv_obj_t *lookup(hashtable_t *ht, string key);
void print_ht(hashtable_t *ht);
void print_ht_table(hashtable_t *ht);

#endif //__HASHTABLE_H__