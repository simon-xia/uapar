/*
 * 
 *
 *							Jan. 14 2015
 *								By Simon Xia
 */
#include "up_hash.h"
#include "up_darray.h"
#include "up_common.h"

/*
struct hash_node {
	void *element;		//key is contained in element
	struct hash_node *next;
};

struct hash_table {
	unsigned int slot_size;
	unsigned int node_cnt;
	Hash_node **table_entry;

	void* (*hash_func)(void*);
	void* (*fetch_key)(void*);
	void  (*free_element)(void*);
	void  (*update_element)(void*, void*);
	void  (*display_element)(void*);

};

// can't iterate backwards, use two iterators instead
struct hash_iterator{
	Hash_table	*table;
	Hash_node   *current;
	unsigned	node_index;
	unsigned	slot_index;
};
*/

static int up_hash_del_mid_node(Hash_table *, Hash_node *);
static void up_hash_destroy_inner(Hash_table *, int);

Hash_table* up_hash_init(int slot_size, void* (*hash_func)(void*), void (*update_element)(void*, void*), void* (*fetch_key)(void*), void (*free_element)(void*), void (*display_element)(void*))
{
	if (slot_size > MAX_HASH_TABLE_SIZE || hash_func == NULL)
		return NULL;

	Hash_table *ht = (Hash_table*)malloc(sizeof(Hash_table));
	assert(ht != NULL);
	ht -> slot_size = slot_size;
	ht -> node_cnt = 0;
	ht -> table_entry = (Hash_node**)calloc(sizeof(Hash_node*), slot_size);
	assert(ht -> table_entry != NULL);
	ht -> hash_func = hash_func;
	ht -> fetch_key = fetch_key;
	if (free_element)
		ht -> free_element = free_element;
	else
		ht -> free_element = free;
	ht -> update_element = update_element;
	ht -> display_element = display_element;
	return ht;
}

Hash_node* up_hash_lookup(Hash_table *ht, void *key)
{
	unsigned hash_key = (unsigned)ht->hash_func(key);
	//assert(hash_key >= 0);
	Hash_node *target_slot = ht->table_entry[hash_key % ht->slot_size];
	if (!target_slot) {
		return NULL;
	}
	else {
		Hash_node *tmp = target_slot;
		while (tmp) {
			if (ht->fetch_key(tmp->element) == key)
				return tmp;
			tmp = tmp -> next;
		}
		return NULL;
	}
}

// because of encapsulation for element, unnecessary malloc/free for existed element. 
int up_hash_insert(Hash_table *ht, void *element)
{
	Hash_node *lookup;
	if ((lookup = up_hash_lookup(ht, ht->fetch_key(element))) == NULL) {
		lookup = (Hash_node*)malloc(sizeof(Hash_node));
		if (!lookup) {
			ERROR("Not enough memory\n");
			return UP_ERR;
		}
		lookup -> element = element;
		unsigned hash_key = (unsigned)ht->hash_func(ht->fetch_key(element));
		unsigned int target_slot = hash_key % ht->slot_size;
		lookup -> next = ht->table_entry[target_slot];
		ht->table_entry[target_slot] = lookup;
		ht->node_cnt++;
	}
	else {
		//WARNING("[load path function] can't reach here!\n");
		ht->update_element(lookup->element, element);
		// mark memory leak
		ht -> free_element(element);
	}
	return UP_SUCC;
}

int up_hash_del_element(Hash_table *ht, void *element)
{
	Hash_node *tmp_node;
	if (tmp_node = up_hash_lookup(ht, element))
		return up_hash_del_node(ht, tmp_node);

	WARNING("del err: can't find the element\n");
	return UP_ERR;
}

int up_hash_del_node(Hash_table *ht, Hash_node *node)
{
	if (node->next) 
		return up_hash_del_mid_node(ht, node);

	Hash_node *prev = NULL;
	Hash_node *tmp_node = ht->table_entry[(unsigned)(ht->hash_func(ht->fetch_key(node->element))) % ht->slot_size];
	// mark the head node
	if (!(tmp_node->next)) {
		ht->table_entry[(unsigned)(ht->hash_func(ht->fetch_key(node->element))) % ht->slot_size] = NULL;
	}
	else {
		while (tmp_node->next) {
			prev = tmp_node;
			tmp_node = tmp_node -> next;
		}
		prev->next = NULL;
	}

	ht->free_element(tmp_node->element);
	free(tmp_node);
	ht->node_cnt--;
	return UP_SUCC;
}

// del non-tail node of list
static int up_hash_del_mid_node(Hash_table *ht, Hash_node *node)
{
	Hash_node *tmp_node;

	ht->free_element(node->element);
	node->element = node->next->element;
	tmp_node = node->next;
	node->next = node->next->next;
	free(tmp_node);
	ht->node_cnt--;
	return UP_SUCC;
}

/* Replaced by iterator
// helper for hash, flag 1 for display, 0 for destory
static void up_hash_traverse(Hash_table *ht, int flag)
{
	if (ht == NULL) {
		WARNING("empty hash table!\n");
		return ;
	}

	int i = 0, node_counter = 1;
	Hash_node *tmp;
	for ( ; i < ht -> slot_size; i++)
	{
		if (!ht -> table_entry[i])
			continue;
		tmp = ht -> table_entry[i];
		if (flag) printf("In slot %d:\n", i);
		while (tmp) {
			if (flag) {
				printf("\tnode %5d: ", node_counter++);
				ht -> display_element(tmp->element);
			}
			else {
				if (ht -> free_element)
					ht -> free_element(tmp->element);
				else
					free(tmp->element);
			}
			tmp = tmp -> next;
			//free node
		}
	}
}
*/

void up_hash_display(Hash_table *ht)
{
	if (ht == NULL) {
		WARNING("empty hash table!\n");
		return ;
	}

	printf("hash's total node is: %d\n", ht -> node_cnt);

	int i = 0, node_counter = 1;
	Hash_node *tmp;
	for ( ; i < ht -> slot_size; i++)
	{
		if (!ht -> table_entry[i])
			continue;
		tmp = ht -> table_entry[i];
		printf("In slot %d:\n", i);
		while (tmp) {
			printf("\tnode %5d: ", node_counter++);
			ht -> display_element(tmp->element);
			tmp = tmp -> next;
		}
	}
}

void up_hash_destroy(Hash_table *ht)
{
	return up_hash_destroy_inner(ht, 0);
}

void up_hash_destroy_retain_element(Hash_table *ht)
{
	return up_hash_destroy_inner(ht, 1);
}

static void up_hash_destroy_inner(Hash_table *ht, int element_retain_flag)
{
	if (ht == NULL) {
		WARNING("empty hash table!\n");
		return ;
	}

	int i = 0;
	Hash_node *tmp, *pre_tmp;

	for ( ; i < ht -> slot_size; i++)
	{
		if (!ht -> table_entry[i])
			continue;
		tmp = ht -> table_entry[i];
		while (tmp) {
			if (!element_retain_flag)
				ht -> free_element(tmp->element);

			pre_tmp = tmp;
			tmp = tmp -> next;
			free(pre_tmp);
		}
	}
	free(ht -> table_entry);
	free(ht);
}

void* test_hash_func(void* key)
{
	// mark unsigned
	return (void*)((unsigned)key % 100);
}

Hash_iterator* up_hash_iterator_init(Hash_table *table)
{
	if (!table) {
		ERROR("argument error\n");
		return NULL;
	}

	Hash_iterator *it = (Hash_iterator *)malloc(sizeof(Hash_iterator));
	it -> table = table;
	it -> current = NULL;
	it -> slot_index = 0;
	it -> node_index = 0;

	return it;
}

Hash_iterator* up_hash_iterator_dup(Hash_iterator *iter1, Hash_iterator *iter2)
{
	memcpy(iter1, iter2, sizeof(Hash_iterator));
	return iter1;
}

Hash_node* up_hash_iterator_next(Hash_iterator *it)
{
	if (it -> node_index == it -> table -> node_cnt)
		return NULL;

	if (it -> current && it -> current -> next) {
			it -> current = it -> current -> next;
	}
	else {
		do {
			it -> current = it -> table -> table_entry[it->slot_index++];
		}while(!(it->current));
	}
	it -> node_index++;	
	return it -> current;
}


void up_hash_iterator_destroy(Hash_iterator *it)
{
	free(it);
}

void up_hash_iterator_operate(Hash_table *ht, void (*operate_func)(void*))
{
	Hash_iterator *iter = up_hash_iterator_init(ht);
	Hash_node *tmp_node;
	while (tmp_node = up_hash_iterator_next(iter)) {
		operate_func(tmp_node->element);
	}
	up_hash_iterator_destroy(iter);
}

D_array* up_hash_dump_darray(Hash_table *ht)
{
	if (!ht || !(ht->node_cnt))
		return NULL;

	D_array *array = up_darray_init(ht->node_cnt, sizeof(void*));
	Hash_iterator *iter = up_hash_iterator_init(ht);
	Hash_node *tmp_node;

	while (tmp_node = up_hash_iterator_next(iter)) {
		up_darray_push(&array, &(tmp_node->element));
	}
	up_hash_iterator_destroy(iter);

	return array;
}


#ifdef UNIT_TEST_HASH

#include "up_ip4.h"
#include "up_path.h"
#include "up_interface.h"
#include <time.h>

#define TEST_HASH_SLOT_SIZE		10
#define TEST_HASH_NODE_CNT		100


int main()
{
	ip_t tmp;
	int i;
	Pos_on_path path;
	Interface *it;
	tmp.dot_ip.f1 = 192;
	tmp.dot_ip.f2 = 168;
	tmp.dot_ip.f3 = 1;

	Hash_table *ht = up_hash_init(TEST_HASH_SLOT_SIZE, test_hash_func, up_interface_update, up_fetch_interface_key, up_interface_destroy, up_interface_display);

	for (i = 0; i < TEST_HASH_NODE_CNT; i++)
	{
		tmp.dot_ip.f4 = rand() % 30 + 100;
		it = up_interface_init(tmp);
		path.path_id = rand() % 5 + 1;
		path.pos = rand() % 1000;
		up_interface_addpath(it, (void*)&path);
		up_hash_insert(ht, (void*)it); // need to be fixed in this unit test
		printf("insert No.%d: %d.%d.%d.%d\n",i+1, tmp.dot_ip.f1, tmp.dot_ip.f2, tmp.dot_ip.f3, tmp.dot_ip.f4);
	}

	up_hash_display(ht);
	up_hash_destroy(ht);

	return 0;
}

#endif
