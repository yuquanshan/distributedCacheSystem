#include <stdio.h>
#include "kv511.h"

int main(int argc, char const *argv[])
{
	node_t *heads = initialize_hashtable();
	k_t key = 'a';
	v_t val = '1';
	put_node(key, val, heads);

	node_t *node = get_node(key,heads);
	printf("The value is %d\n",node->val);
	v_t newval = '2';
	put_node(key, newval, heads);
	printf("The value is %d\n",node->val);
	return 0;
}