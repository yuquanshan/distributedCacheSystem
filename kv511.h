#include <stdlib.h>

#define MAX_SESSIONS 10000
#define HASH_ENTRY_SIZE 50
typedef char k_t;
typedef char v_t;

typedef struct setnode{
	k_t key;
	v_t val;
	int empty;	// 1 is empty; 0 is non-empty
	struct setnode *next;
} node_t;

node_t* initialize_hashtable(){
	node_t* heads = malloc(HASH_ENTRY_SIZE*sizeof(node_t));
	int i;
	node_t *tmp;
	for(i = 0; i<HASH_ENTRY_SIZE; i++){	
		tmp = (heads+i*sizeof(node_t));
		tmp->empty = 1;	// set all entries to be empty
		tmp->next = NULL;
	}
	return heads;
}

int divhash_func(k_t key){	// very simple division-based hash
	return (((int)key)%HASH_ENTRY_SIZE+HASH_ENTRY_SIZE)%HASH_ENTRY_SIZE;
}

node_t* get_node(k_t key, node_t *heads){	// find the node with key, return NULL if no such node
	node_t* res = NULL;
	int entry = divhash_func(key);
	node_t* tmp;
	if((heads+entry*sizeof(node_t))->empty != 1){
		tmp = (heads+entry*sizeof(node_t))->next;
		while(tmp != NULL){
			if(tmp->key == key)
				return tmp;
			tmp = tmp->next;
		}
	}
	return res;
}

void put_node(k_t key, v_t val, node_t *heads){
	int entry = divhash_func(key);
	node_t* node = get_node(key,heads);
	if(node != NULL){
		node->val = val;
	}else{
		node = malloc(sizeof(node_t));
		node->key = key;
		node->val = val;
		node_t* tmp = heads+entry*sizeof(node_t);
		tmp->empty = 0;	// not empty no more
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = node;
	}
}