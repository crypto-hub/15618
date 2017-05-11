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
#define 	WR_PERCENT		2
#endif

#define 	WR_FRACTION		(WR_PERCENT/100.0f)
//#define 	LOAD_FACTOR		0.4f
#define 	LOAD_FACTOR		0.4f

#define 	HT_SIZE			(int)((1<<NBUCKETS)*4*LOAD_FACTOR)
#define 	NREADS 			(HT_SIZE * (1-WR_FRACTION))
#define 	NWRITES			((int)(HT_SIZE * (WR_FRACTION)))
//#define 	NWRITES			400
#define 	READ 	0
#define  	WRITE 	1

typedef struct{

	int type = READ;
	kv_obj_t *kv;

}req_t;

kv_obj_t *kv_objs_wr;
kv_obj_t *kv_objs;
hashtable_t *ht;
unsigned total_inserts = 0;

typedef struct {
	hashtable_t *ht;
	kv_obj_t *kv_objs;
	int rd_size;
	kv_obj_t *kv_objs_wr;
	int wr_size;
	int tid;

} tester_arg_t;

void * tester(void *arg) {

	int total = 0;
	tester_arg_t *targ = (tester_arg_t *)arg;

	if (WR_PERCENT == 0) {
		for(int i=0; i<targ->rd_size; i++){
			//cout << " tid : " << targ->tid << " key : " << targ->kv_objs[i].key << endl;
			kv_obj_t *kv = lookup(targ->ht, targ->kv_objs[i].key);
			// if (kv){
			// 	if (targ->kv_objs[i].key.compare(kv->val)){
			// 		cout << "mismatch" << endl;
			// 	}
			// }else{
			// 	cout << "=======   =========   ===== lookup failed " <<targ->kv_objs[i].key  << "  " << targ->tid << "   " << targ->rd_size+targ->wr_size << "  i : " << i << endl;
			// }

		}
	}
	else {
		//int lo = rand() % (100-WR_PERCENT);
		//int hi = lo+WR_PERCENT;
		for(int i=0, j=0, k=0; i<(targ->rd_size+targ->wr_size); i++){
			//cout << " tid : " << targ->tid << " key : " << targ->kv_objs[i].key << endl;
			if ((i% (100/WR_PERCENT)) == targ->tid) {
			//int temp = i%100;
			//if(temp >= lo && temp <= hi) {
				if(j < (targ->wr_size) ){
					//cout << " tid : " << targ->tid << " write : " << i << endl;
					insert(targ->ht, &targ->kv_objs_wr[j]);
					j++;
					total++;

				}
				
			}
			else {
				if(k < targ->rd_size) {
					//cout << " tid : " << targ->tid << " read : " << i << endl;
					lookup(targ->ht, targ->kv_objs[k].key);
					k++;
				}

				
			}

		}
	}
	
	//cout << "tid : " << targ->tid << "  writes : " << total << endl; 
	return NULL;
}


void test_exclusive_read_load() {
	printf("test_exclusive_read_load started ...\n");

	pthread_t testers[NTHREADS];
	tester_arg_t targs[NTHREADS];
	double start_time, end_time, duration;
	int load_size = NREADS;

	/* initialize the hash table */
	ht = init_hash_table(NBUCKETS, VC_SIZE);

	/* insert into hash table */
	for(int i=0; i<NREADS; i++) {
		//cout << "inserting : "<<kv_objs[i].key << " val : "<< kv_objs[i].val << endl; 
		//cout << i << endl;
		//cout << "inserting  " << kv_objs[i].key << endl;
		if (!insert(ht, &kv_objs[i])) {
			load_size = i;
			cout << "load size : " << load_size << endl;
			break;
		}
	}

	//print_ht_table(ht);
	cout << "inserts done " << endl;

	int per_thread_write = NWRITES / NTHREADS;
	int per_thread_read = load_size/NTHREADS;

	/* prepare threads */
	for(int i=0; i<NTHREADS; i++) {
		targs[i].ht = ht;
		targs[i].kv_objs = &kv_objs[i*per_thread_read];
		targs[i].rd_size = per_thread_read;

		targs[i].kv_objs_wr = &kv_objs_wr[i*per_thread_write];
		targs[i].wr_size = per_thread_write;

		targs[i].tid = i;
		if(i == NTHREADS-1) {

			targs[i].rd_size = load_size - (i*per_thread_read);
			targs[i].wr_size = NWRITES - (i*per_thread_write);
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

	#ifdef SINGLE_THREADED
	double mors = ((load_size)/duration) / (1000 * 1000);
	#else
	double mors = ((load_size+NWRITES)/duration) / (1000 * 1000);
	#endif
	

	
	/* calculate average */
	std::cout << "duration: " << duration << "s" 	<< std::endl;
	std::cout << "mors:     " << mors     << " mrs"  << std::endl;
}




int main() {


	cout << "NWRITES " << NWRITES << endl;
	srand(time(NULL));
	printf("----- Performance Measurement Start -------\n");

	/* create load: key value object */
	kv_objs = new kv_obj_t[LOAD_SIZE];
	for (int i=0; i<LOAD_SIZE; i++) {
		kv_objs[i].key = std::to_string(rand());
		//kv_objs[i].key = std::to_string(i);
		kv_objs[i].val = kv_objs[i].key;

	}

	kv_objs_wr = new kv_obj_t[NWRITES];
	for (int i=0; i<NWRITES; i++) {
		kv_objs_wr[i].key = std::to_string(rand());
		//kv_objs[i].key = std::to_string(i);
		kv_objs_wr[i].val = kv_objs_wr[i].key;

	}

	/* test exclusive read */
	test_exclusive_read_load();

	printf("----- Performance Measurement End   -------\n");
}