/*
 *
 *
 *
 */

#include <iostream>
#include <assert.h>

#include <unordered_map>
#include "chaining_hashtable.h"
#include "utils.h"
using namespace std;

/* The entry point for scalecuckoo */

#define OPS (1<<20)





void run(){


	string* random_keys = new string[OPS];



	for (int i=0; i<OPS; i++) {
	 	//random_keys[i] = to_string(i);
		random_keys[i] = to_string(rand());
	}



	hashtable_t *ht = init_hash_table(0,0);

	int i = 0;

	while(1){


		kv_obj_t *kv = new kv_obj_t; // dont use malloc if string (c++) is used
		kv->key = random_keys[i];
		kv->val = random_keys[i];

		dbg_printf("Inserting.. %d %p \n",i,kv);
		
		dbg_printf("%d\n",i);
		bool ret = insert(ht,kv);
		dbg_printf("Insert done ! \n");
		

		//print_ht(ht);

		if(ret){

			
			dbg_printf("Looking up %s .. \n",kv->key.c_str());
			
			kv_obj_t *result = lookup(ht,kv->key);

			if (result){
				dbg_printf("val : %s \n",result->val.c_str());

				if(result->val.compare(result->key)){
					printf("value mismatch ! \n");
					break;
				}

				

			}else{
				dbg_printf("key not found \n");
			}

		}else{

			printf("insert failed !\n");
			break;
		}

		i++;
		if (i == OPS)
			break;
	}

	//print_ht_table(ht);


	printf("total inserts : %d\n",i);

}

int main(){

	srand(time(NULL));
	run();

}