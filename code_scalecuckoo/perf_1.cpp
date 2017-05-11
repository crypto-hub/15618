#include <stdint.h>
#include <stdlib.h>

#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <string>
#include <chrono>
#include "hashtable.h"
#include "CycleTimer.h"

//#define SINGLE_THREADED

using namespace std;

#define 	NTHREADS 		6
//#define 	LOAD_SIZE		(1<<5)
#define 	LOAD_SIZE		(1<<24)
//#define 	NBUCKETS		(int)((double)LOAD_SIZE / (double)1.5)
#define 	NBUCKETS		(20)
#define 	VC_SIZE			(13)
#define 	NITERATIONS 	1

#ifdef SINGLE_THREADED
#define 	WR_PERCENT		0
#else
#define 	WR_PERCENT		5
#endif

#define 	WR_FRACTION		(WR_PERCENT/100.0f)
#define 	LOAD_FACTOR		(0.3f)

#define 	HT_SIZE			(int)((1<<NBUCKETS)*4*LOAD_FACTOR)

//#define 	NWRITES			((int)(HT_SIZE * (WR_FRACTION)))
#define 	NWRITES			200
#define 	NREADS 			(HT_SIZE - NWRITES)

#define 	READ 	0
#define  	WRITE 	1

typedef struct{

	int type = READ;
	kv_obj_t *kv;

}req_t;

req_t *req_arr;

hashtable_t *ht;


typedef struct {
	hashtable_t *ht;
	req_t *req_arrs;
	int size;
	int tid;

} tester_arg_t;

void * tester(void *arg) {

	int total = 0;
	tester_arg_t *targ = (tester_arg_t *)arg;


	for(int i=0; i<targ->size; i++){
			
		if (targ->req_arrs[i].type == READ){

			kv_obj_t *kv = lookup(targ->ht, targ->req_arrs[i].kv->key);
		}else{

			insert(targ->ht, targ->req_arrs[i].kv);
			total++;
		}


			/*if (kv){
				if (targ->kv_objs[i].key.compare(kv->val)){
					cout << "mismatch" << endl;
				}
			}else{
				cout << "=======   =========   ===== lookup failed " <<targ->kv_objs[i].key  << "  " << targ->tid << "   " << targ->size << "  i : " << i << endl;
			}*/

	}
	
	//cout << "total " <<total << "tid " << targ->tid << endl;
	
	return NULL;
}


void test_exclusive_read_load() {
	printf("test_exclusive_read_load started ...\n");

	pthread_t testers[NTHREADS];
	tester_arg_t targs[NTHREADS];
	double start_time, end_time, duration;

	/* initialize the hash table */
	ht = init_hash_table(NBUCKETS, VC_SIZE);

	/* insert into hash table */
	for(int i=0; i<HT_SIZE; i++) {
		//cout << "inserting : "<<kv_objs[i].key << " val : "<< kv_objs[i].val << endl; 
		//cout << i << endl;
		//cout << "inserting  " << kv_objs[i].key << endl;
		if (req_arr[i].type == READ){

			if (!insert(ht, req_arr[i].kv)) {
				
				cout << "===== >  insert failed " << endl;
				break;
			}
		}
	}

	//print_ht_table(ht);
	cout << "inserts done " << endl;

	int per_thread_size = HT_SIZE / NTHREADS;
	

	/* prepare threads */
	for(int i=0; i<NTHREADS; i++) {
		targs[i].ht = ht;
		targs[i].req_arrs = &req_arr[i*per_thread_size];
		targs[i].size = per_thread_size;

		targs[i].tid = i;
		if(i == NTHREADS-1) {

			targs[i].size = HT_SIZE - (i*per_thread_size);
		}
	}

	//auto begin = std::chrono::high_resolution_clock::now();
	start_time = CycleTimer::currentSeconds();

	/* run test multiple times */
	for(int i=0; i<NITERATIONS; i++) {

	#ifdef SINGLE_THREADED

		for(int j=0; j<load_size; j++){
			//cout << j << endl;
			kv_obj_t *kv = lookup(ht, kv_objs[j].key);
			if (kv){
				//cout << "recvd : " << kv->key << " " << kv->val << endl;
				if (kv_objs[j].key.compare(kv->val)){
					cout << "mismatch" << endl;
				}
			}else{
				cout << "lookup failed " <<kv_objs[j].key  << "  j : " << j << endl;
				assert(0);
			}

		}

	#else

		for(int j=0; j<NTHREADS; j++)
			pthread_create(&testers[j], NULL, tester, &targs[j]);

		for(int j=0; j<NTHREADS; j++)
			pthread_join(testers[j], NULL);

	#endif
	}

	end_time = CycleTimer::currentSeconds();
	//auto end = std::chrono::high_resolution_clock::now();

	duration = ((end_time - start_time)/NITERATIONS);


	double mors = ((HT_SIZE)/duration) / (1000 * 1000);


	
	/* calculate average */
	std::cout << "duration: " << duration << "s" 	<< std::endl;
	std::cout << "mors:     " << mors     << " mrs"  << std::endl;
}




int main() {

	srand(time(NULL));
	printf("----- Performance Measurement Start -------\n");

	/* create load: key value object */
	cout << "HT SIZE : " << HT_SIZE << " NWRITES " << NWRITES <<endl;
	req_arr = new req_t[HT_SIZE + 10];
	for (int i=0; i<HT_SIZE; i++) {
		kv_obj_t *kv = new kv_obj_t;
		kv->key = std::to_string(rand());
		
		//kv_objs[i].key = std::to_string(i);
		kv->val = kv->key;
		req_arr[i].kv = kv;
		req_arr[i].type = READ;

	}

	int index;

	for(int i = 0; i<NWRITES; i++){

		index = rand() % HT_SIZE;

		req_arr[index].type = WRITE;

	}

	/* test exclusive read */
	test_exclusive_read_load();

	printf("----- Performance Measurement End   -------\n");
}