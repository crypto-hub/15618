
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "hash.h"
#include "chaining_hashtable.h"

hashtable_t* init_hash_table(int total_buckets_power, int version_counter_power){

	int total_buckets;
	if(!total_buckets_power)
		total_buckets_power = DEFAULT_BUCKETS_POWER;

	total_buckets = total_buckets_power;

	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
	ht->total_buckets = total_buckets;
	if(ht){

		ht->table = (bucket_t *)malloc(sizeof(bucket_t)*total_buckets);
		for(int i = 0; i< total_buckets; i++){
			ht->table[i].head = NULL;
			pthread_mutex_init(&ht->table[i].lock,NULL);
		}

		pthread_mutex_init(&ht->tablelock,NULL);

	}

	return ht;

}

bool insert(hashtable_t *ht, kv_obj_t *kv){

	uint32_t hash;
	//pthread_mutex_lock(&ht->tablelock);
	hash = hashlittle(kv->key.c_str(), kv->key.length(), 0);
	hash = hash % ht->total_buckets;
	kv->next = NULL;
	kv_obj_t *head = ht->table[hash].head;

	if (!head){
		ht->table[hash].head = kv;

	}else{

		while(head->next){
			head = head->next;
		}
		head->next = kv;
	}

	//pthread_mutex_unlock(&ht->tablelock);

	return true;
}



kv_obj_t *lookup(hashtable_t *ht, string key){

	kv_obj_t *ans_kv = NULL;
	uint32_t hash;
	//pthread_mutex_lock(&ht->tablelock);
		
	hash = hashlittle(key.c_str(), key.length(), 0);
	hash = hash % ht->total_buckets;

	kv_obj_t *head = ht->table[hash].head;

	while(head){
		if (!(head->key.compare(key))){
			ans_kv = head;
			break;
		}
		head = head->next;
	}

	//pthread_mutex_unlock(&ht->tablelock);

	return ans_kv;

}