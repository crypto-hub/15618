// cuckoo + no tags 

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "cuckoo_hash.h"
#include "hashtable.h"
#include "utils.h"


 unsigned char tagmask  = 0xff;
 unsigned int bucketmask;
 unsigned int lockmask;
 unsigned slotmask = SLOTS_PER_BUCKET - 1;

void add_kv_in_table(hashtable_t *ht, swap_pair_t *pair, 
										kv_obj_t *kv, unsigned char tag, int bucket);

void print_ver_count(hashtable_t *ht);
void print_ht_table(hashtable_t *ht);

void check_version_arr(hashtable_t *ht){

	for(int i= 0; i<ht->total_version_counters; i++){
		if((ht->version_counters[i] % 2 ) != 0){

			printf(" === >%d  %d < === \n",i,ht->version_counters[i]);
			assert(0);
		}
	}

}


// initialise hash table

hashtable_t* init_hash_table(int total_buckets_power, int version_counter_power){

	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));

	int version_counters;

	if (!total_buckets_power)
    	total_buckets_power = DEFAULT_BUCKETS_POWER;

    if(!version_counter_power){

    	version_counter_power = DEFAULT_VERSION_COUNTER_POWER;

    	version_counters = (1<<version_counter_power);
    }

    // if(!tagsize)
    // 	tagsize = DEFAULT_TAG_SIZE;

    int total_buckets = (1<<total_buckets_power);
    bucketmask = total_buckets - 1;
    lockmask = version_counters - 1;


    dbg_printf("bucketmask : %x, lockmask %x \n",bucketmask, lockmask);


    dbg_printf("total buckets : %d  total counters : %d \n", 
    						total_buckets, version_counters);

	if (ht){


		pthread_mutex_init(&ht->writer_mutex, NULL);

		ht->total_buckets = (1<<total_buckets_power);
		ht->total_version_counters = version_counters;

		ht->table = (bucket_t *)malloc(sizeof(bucket_t)*total_buckets);

		if (!ht->table){
			printf(" Unable to allocate memory for hash table \n");
			return NULL;
		}

		dbg_printf("%s : size of hash table : %d \n",__func__,sizeof(bucket_t)*total_buckets);


		ht->bitmap_size = (sizeof(unsigned char)*total_buckets*SLOTS_PER_BUCKET)/8;
		ht->bitmap = (unsigned char *)malloc(ht->bitmap_size);

		if (!ht->bitmap){
			printf(" Unable to allocate memory for bitmap \n");
			return NULL;
		}

		ht->version_counters = (int *)malloc(sizeof(int)*version_counters);

		if (!ht->version_counters){
			printf(" Unable to allocate memory for version counters \n");
			return NULL;
		}


		memset(ht->table, 0, sizeof(bucket_t)*total_buckets);
		memset(ht->version_counters, 0, sizeof(int)*version_counters);
	}

	return ht;
}




bool insert_cuckoo_path(hashtable_t *ht,cuckoo_path_t *cuckoo_path, int bucket, int slot){


	int id = bucket * SLOTS_PER_BUCKET + slot;
	int byte_offset = id / 8;
	int bit_offset = id % 8;

	unsigned char byte = ht->bitmap[byte_offset];
	if((byte >> bit_offset) & 0x1)
		return false;

	byte = byte | (1 << bit_offset);
	ht->bitmap[byte_offset] = byte;


	cuckoo_path->path[cuckoo_path->path_index].bucket = bucket;
	cuckoo_path->path[cuckoo_path->path_index].slot = slot;
	cuckoo_path->path_index++;

	return true;

}


void init_cuckoo_path(cuckoo_path_t *cuckoo_path){
	cuckoo_path->path_index = 0;
}

static inline int next_bucket(hashtable_t *ht, const size_t bucket,const size_t slot) {

    // 0x5bd1e995 is the hash constant from MurmurHash2
    uint32_t hash1 = 0, hash2 = 0;
    hashlittle2(ht->table[bucket].slots[slot].ptr->key.c_str(),ht->table[bucket].slots[slot].ptr->key.length(),&hash1, &hash2);
    //return (index ^ ((tag & tagmask ) * 0x5bd1e995)) & bucketmask;

    hash1 = hash1 & bucketmask;
    hash2 = hash2 & bucketmask;

    return (hash1 == bucket ? hash2:hash1);
}

static inline unsigned char tag_hash(const uint32_t hv) {

    uint32_t r =  hv & tagmask;
    return (unsigned char) r + (r == 0);
}




static inline int lock_hash(hashtable_t *ht, int i1, int i2) {

   //int key_version_index = HASH(key) % NUM_KEY_VERSION_COUNTERS;

   return ((i1 <  i2 ? i1 : i2) & lockmask);
}


static inline bool check_key(string a, string b) {

   //int key_version_index = HASH(key) % NUM_KEY_VERSION_COUNTERS;

  	return (!a.compare(b));
}




bool add_attempt(hashtable_t *ht,kv_obj_t *kv,int bucket, unsigned char tag, int bucket2){

	for(int i =0 ; i<SLOTS_PER_BUCKET; i++){

		if (CHECK_EMPTY_SLOT(ht->table[bucket].slots[i])){

			swap_pair_t pair;
			pair.bucket = bucket;
			pair.slot = i;
			add_kv_in_table(ht,&pair,kv,tag, bucket2);
			
			return true;
		}
	}

	return false;

}



bool search_phase(hashtable_t *ht, cuckoo_path_t *cuckoo_path, int current_bucket, int victim_slot){


	int path_found = 0;

	while((cuckoo_path->path_index) != MAX_CUCKOO_PATH_LEN){

		// search 8 paths, terminate search if one path found
		current_bucket = next_bucket(ht,current_bucket,victim_slot);

		for(int i=0; i<SLOTS_PER_BUCKET; i++){


			if (CHECK_EMPTY_SLOT(ht->table[current_bucket].slots[i])){
				// it is empty
				// end of search
				insert_cuckoo_path(ht,cuckoo_path,current_bucket,i);
				path_found = 1;
				break;
			}
		}

		if(path_found)
			break;

		victim_slot  = rand() & slotmask;
		//victim_slot = 3;
		if (!insert_cuckoo_path(ht,cuckoo_path,current_bucket,victim_slot)) {

			int i;
			for(i=0; i<SLOTS_PER_BUCKET; i++){
				if(i == victim_slot)
					continue;
				if (insert_cuckoo_path(ht,cuckoo_path,current_bucket,i)){
					victim_slot = i;
					break;
				}
			}

			if(i == SLOTS_PER_BUCKET)
				return false;
		}
	}

	if(path_found)
		return true;
	else
		return false;


}




void add_kv_in_table(hashtable_t *ht, swap_pair_t *pair, 
										kv_obj_t *kv, unsigned char tag, int alt_bucket){


	//int version_array_index = lock_hash(ht, pair->bucket, alt_bucket); 

    // atomic increment 
	//__sync_fetch_and_add(&ht->version_counters[version_array_index],1);


	ht->table[pair->bucket].slots[pair->slot].ptr = kv;
	ht->table[pair->bucket].slots[pair->slot].tag = tag;

	//__sync_fetch_and_add(&ht->version_counters[version_array_index],1);


}




void reverse_swap(hashtable_t *ht, cuckoo_path_t *cuckoo_path){

	int i = cuckoo_path->path_index-1;
	while(i>0){

		//string key = ht->table[cuckoo_path->path[i-1].bucket].slots[cuckoo_path->path[i-1].slot].ptr->key;

		// int version_array_index = lock_hash(ht,cuckoo_path->path[i-1].bucket, 
		// 	next_bucket(ht,cuckoo_path->path[i-1].bucket, cuckoo_path->path[i-1].slot)); 

		

	
		//__sync_fetch_and_add(&ht->version_counters[version_array_index],1);

	
		// printf("**2 : %d == %d %d %d %d\n", ht->version_counters[version_array_index],
		// 					cuckoo_path->path[i].bucket,cuckoo_path->path[i].slot,
		// 					 cuckoo_path->path[i-1].bucket, cuckoo_path->path[i-1].slot);

		ht->table[cuckoo_path->path[i].bucket].slots[cuckoo_path->path[i].slot] 
					= ht->table[cuckoo_path->path[i-1].bucket].slots[cuckoo_path->path[i-1].slot];
		

		//__sync_fetch_and_add(&ht->version_counters[version_array_index],1);
	
		i--;
	}
}


void print_cuckoo_path(cuckoo_path_t *cuckoo_path){

	printf("\nCuckoo path : \n");
	for(int i=0; i<cuckoo_path->path_index; i++)
		printf("[%d  %d ] => ",cuckoo_path->path[i].bucket,cuckoo_path->path[i].slot);
	printf("\n");
}




bool insert(hashtable_t *ht, kv_obj_t *kv){

	cuckoo_path_t cuckoo_path;
	init_cuckoo_path(&cuckoo_path);
	unsigned char tag;
	uint32_t hash1 = 0,hash2 = 0; 
	int bucket1, bucket2;
	int victim_bucket;
	int other_bucket;
	hashlittle2(kv->key.c_str(), kv->key.length(), &hash1, &hash2);

	//pthread_mutex_lock(&ht->writer_mutex);

	bucket1 =  hash1 & bucketmask;
	tag = tag_hash(hash1);

	victim_bucket = bucket1;


	//bucket2 = next_bucket(bucket1,tag);
	bucket2 = hash2 & bucketmask;

	other_bucket = bucket2;

	dbg_printf(" Key : %s, hash : %xu, tag : %u , bucket1 : %d, bucket2 : %d \n",
									kv->key.c_str(),hash1,tag,bucket1,bucket2);

	
	if(add_attempt(ht,kv,bucket1, tag, bucket2)){
		//pthread_mutex_unlock(&ht->writer_mutex);
		return true;
	}

	

	if(add_attempt(ht,kv,bucket2, tag, bucket2)){
		//pthread_mutex_unlock(&ht->writer_mutex);
		return true;
	}

	

	int victim_slot  = rand() % (2*SLOTS_PER_BUCKET);

	if(victim_slot >= SLOTS_PER_BUCKET){
		victim_bucket = bucket2;
		victim_slot -= SLOTS_PER_BUCKET;
		other_bucket = bucket1;
	}


	memset(ht->bitmap, 0, ht->bitmap_size);
	insert_cuckoo_path(ht,&cuckoo_path,victim_bucket,victim_slot);


	if (search_phase(ht, &cuckoo_path, victim_bucket, victim_slot)){

		
		reverse_swap(ht,&cuckoo_path);

		
		add_kv_in_table(ht,&(cuckoo_path.path[0]), kv, tag, other_bucket);

		//pthread_mutex_unlock(&ht->writer_mutex);
		return true;

	}else{
		//pthread_mutex_unlock(&ht->writer_mutex);
		return false;
	}
	

}




kv_obj_t *lookup(hashtable_t *ht, string key){

	

	// do not derive from bucket
	// as it should be bucket independent after expansion 
	uint32_t hash1 = 0, hash2 = 0;

	hashlittle2(key.c_str(), key.length(), &hash1, &hash2);

	int bucket1 = hash1 & bucketmask;
	int bucket2 = hash2 & bucketmask;
	int bucket = bucket1;
	unsigned char tag = tag_hash(hash1);
	int initial_counter;
	int version_array_index = lock_hash(ht, bucket1, bucket2); 

	dbg_printf(" Key : %s, hash : %xu, tag : %u , bucket1 : %d \n",
									key.c_str(),hash1,tag,bucket);

	kv_obj_t *ans_kv = NULL;

	//do {

		dbg_printf(" ===> initial_counter : %d \n", ht->version_counters[version_array_index]);

		// while((initial_counter = __sync_fetch_and_add(&ht->version_counters[version_array_index],0)) 
		// 																%2 != 0);

		

		for(int i=0; i<SLOTS_PER_BUCKET; i++){

			if(ht->table[bucket].slots[i].ptr){

				dbg_printf("tag matched for bucket %d slot %d \n",bucket,i);
				if(check_key(ht->table[bucket].slots[i].ptr->key,key)){

					ans_kv = ht->table[bucket].slots[i].ptr;
					
					dbg_printf("version counter : %d \n", ht->version_counters[version_array_index]);
					//if(initial_counter == __sync_fetch_and_add(&ht->version_counters[version_array_index],0)){
						return ans_kv;
					//}
				}

			}

		}

		bucket = bucket2;

		for(int i=0; i<SLOTS_PER_BUCKET; i++){

			if(ht->table[bucket].slots[i].ptr){

				if(check_key(ht->table[bucket].slots[i].ptr->key,key)){

					ans_kv = ht->table[bucket].slots[i].ptr;
					//if(initial_counter == __sync_fetch_and_add(&ht->version_counters[version_array_index],0)){
						return ans_kv;
					//}
				}

			}

		}

	//}while(initial_counter != __sync_fetch_and_add(&ht->version_counters[version_array_index],0));

	return ans_kv;

}

void print_ver_count(hashtable_t *ht){

	printf("printing version counters \n");
	for(int i=0; i<ht->total_version_counters; i++){

		printf("%d : %d \n",i,ht->version_counters[i]);

	}
}


void print_ht_table(hashtable_t *ht){

	printf("\nprinting hash-table, buckets : %d \n\n",ht->total_buckets);

	for(int i=0; i<ht->total_buckets; i++){

		printf("\n bucket %d : ",i);
		for(int j = 0; j < SLOTS_PER_BUCKET; j++){

			printf("   [%u:(%p)]  ",ht->table[i].slots[j].tag,
				ht->table[i].slots[j].ptr);
		}

	}

	printf("\n\n");
}


void print_ht(hashtable_t *ht){

	
	print_ver_count(ht);
	print_ht_table(ht);
	


}



