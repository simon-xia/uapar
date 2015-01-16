/*
 * 
 *
 *							Jan. 14 2015
 *								By Simon Xia
 */
#include "up_hash.h"
#include "up_common.h"

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
	ht -> free_element = free_element;
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
		if (!lookup)
			return 0;
		lookup -> element = element;
		unsigned hash_key = (unsigned)ht->hash_func(ht->fetch_key(element));
		unsigned int target_slot = hash_key % ht->slot_size;
		lookup -> next = ht->table_entry[target_slot];
		ht->table_entry[target_slot] = lookup;
		ht->node_cnt++;
	}
	else {
		ht->update_element(lookup->element, element);
		// mark memory leak
		if (ht -> free_element)
			ht -> free_element(element);
		else
			free(element);
	}
}

// helper for hash, flag 1 for display, 0 for destory
static void up_hash_traverse(Hash_table *ht, int flag)
{
	assert(ht != NULL);
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

void up_hash_display(Hash_table *ht)
{
	assert(ht != NULL);

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
	assert(ht != NULL);
	int i = 0;
	Hash_node *tmp, *pre_tmp;
	void (*free_ele_func)(void*);

	if (ht -> free_element)
		free_ele_func = ht -> free_element;
	else
		free_ele_func = free;

	for ( ; i < ht -> slot_size; i++)
	{
		if (!ht -> table_entry[i])
			continue;
		tmp = ht -> table_entry[i];
		while (tmp) {
			free_ele_func(tmp->element);
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
	return (void*)((unsigned int)key % 100);
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

	Hash_table *ht = up_hash_init(TEST_HASH_SLOT_SIZE, test_hash_func, interface_update, fetch_interface_key, interface_destroy, interface_display);

	for (i = 0; i < TEST_HASH_NODE_CNT; i++)
	{
		tmp.dot_ip.f4 = rand() % 30 + 100;
		printf("insert No.%d: %d.%d.%d.%d\n",i+1, tmp.dot_ip.f1, tmp.dot_ip.f2, tmp.dot_ip.f3, tmp.dot_ip.f4);
		it = interface_init(tmp);
		path.path_id = rand() % 5 + 1;
		path.pos = rand() % 1000;
		interface_addpath(it, (void*)&path);
		up_hash_insert(ht, (void*)it);
	}

	up_hash_display(ht);
	up_hash_destroy(ht);

	return 0;
}

#endif
