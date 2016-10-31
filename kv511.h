#include <stdlib.h>
#include <unordered_map>
#define MAX_SESSIONS 500
#define HASH_ENTRY_SIZE 50
#define BUFSIZE 100

typedef char k_t;
typedef char v_t;

std::unordered_map<k_t,v_t> mmap;

typedef struct setnode{
	k_t key;
	v_t val;
	int empty;	// 1 is empty; 0 is non-empty
	struct setnode *next;
} node_t;

typedef struct thread_arg{	// input argument of client thread
	float lambda;		// session poisson arrival rate
	int nsession;	// number of sessions for each threads
	int nreq;		// number of requests for each session
	const char* iaddr;
} arg_t;

int get_or_put(const char* buf){	// return 0 if get, 1 if put, -1 if N/A
	int i;
	char *getstr = "GET";
	char *putstr = "PUT";
	char box[4];
	box[3] = '\0';
	strncpy(box,buf,3);
	if(strcmp(box,getstr)==0){
		return 0;
	}else if(strcmp(box,putstr)==0){
		return 1;
	}
	return -1;
}

node_t* initialize_hashtable(){
	node_t* heads = (node_t*)malloc(HASH_ENTRY_SIZE*sizeof(node_t));
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
	std::unordered_map<k_t,v_t>::const_iterator got = mmap.find(key);
	if(got == mmap.end()){
		return NULL;
	}else{
		node_t* res = (node_t *)malloc(sizeof(node_t));
		res->key = key;
		res->val = mmap[key];
		return res;
	}
	/*printf("trying to get key %c...\n",key);
	node_t* res = NULL;
	int entry = divhash_func(key);
	printf("hash table entry is %d\n",entry);
	node_t* tmp;
	if((heads+entry*sizeof(node_t))->empty != 1){
		tmp = (heads+entry*sizeof(node_t))->next;
		printf("emm...\n");
		while(tmp != NULL){
			printf("humm...\n");
			printf("-------- %p\n",tmp);
			printf("-------- %c\n",tmp->empty);
			printf("-------- %c\n",tmp->val);
			printf("-------- met key %c\n",tmp->key);
			if(tmp->key == key){
				printf("find key %c, its value is %c\n", key, tmp->val);
				return tmp;
			}
			tmp = tmp->next;
		}
	}
	printf("can't find key %c\n",key);
	return res;*/
}

void put_node(k_t key, v_t val, node_t *heads){
	mmap[key] = val;
	/*printf("trying to put key-value pair: <%c,%c>...\n",key,val);
	int entry = divhash_func(key);
	printf("hash table entry is %d\n",entry);
	node_t* node = get_node(key,heads);
	if(node != NULL){
		node->val = val;
	}else{
		node = (node_t *)malloc(sizeof(node_t));
		node->key = key;
		node->val = val;
		node->next = NULL;
		node_t* tmp = heads+entry*sizeof(node_t);
		tmp->empty = 0;	// not empty no more
		while(tmp->next != NULL){
			tmp = tmp->next;
			printf("yeepee!\n");
			printf("-------- %p\n",tmp);
		}
		printf("not done yet...\n");
		tmp->next = node;
	}
	*/
}
