#include <unordered_map>
#include "hashtable.h"
#include <stdlib.h>

unordered_map<string, kv_obj_t *>kv_map;
pthread_mutex_t lock;

hashtable_t* init_hash_table(int total_buckets_power, int version_counter_power){

	
	pthread_mutex_init(&lock,NULL);
	return NULL;

}

bool insert(hashtable_t *ht, kv_obj_t *kv){

	pthread_mutex_lock(&lock);
	kv_map[kv->key] = kv;
	pthread_mutex_unlock(&lock);

	return true;
}



kv_obj_t *lookup(hashtable_t *ht, string key){

	kv_obj_t *ans_kv = NULL;
	pthread_mutex_lock(&lock);
	auto it = kv_map.find(key);
	if (it != kv_map.end())
		ans_kv = it->second;
	pthread_mutex_unlock(&lock);

	return ans_kv;

}