
#ifndef __CHAIN_HASH_H__
#define __CHAIN_HASH_H__

#include <string>


//#define DEFAULT_VERSION_COUNTER_POWER  (13)


//#define DEFAULT_BUCKETS_POWER (3)
#define DEFAULT_BUCKETS_POWER (15)

using namespace std;

struct kv_obj_t{

	string key;
	string val;
	kv_obj_t *next;

};




struct bucket_t{

	kv_obj_t *head;
	pthread_mutex_t lock;

};


struct hashtable_t{	

	bucket_t *table;
	int total_buckets;
	pthread_mutex_t tablelock;
};

hashtable_t* init_hash_table(int total_buckets, int version_counters);
bool insert(hashtable_t *ht, kv_obj_t *kv);
kv_obj_t *lookup(hashtable_t *ht, string key);

#endif