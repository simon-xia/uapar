/*
 * 
 *						By Simon Xia
 *							Jan. 14 2015
 */
#ifndef _UP_HASH_H_
#define _UP_HASH_H_

//typedef struct d_array D_array;
#include "up_darray.h"

#define MAX_HASH_TABLE_SIZE 4096

typedef struct hash_node Hash_node;
typedef struct hash_table Hash_table;
typedef struct hash_iterator Hash_iterator;

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


Hash_table* up_hash_init(int, void* (*)(void*), void (*)(void*, void*), void* (*)(void*), void (*)(void*), void (*)(void*));
Hash_node* up_hash_lookup(Hash_table *, void *);
int up_hash_insert(Hash_table *, void *);
//static void up_hash_traverse(Hash_table *, int);
void up_hash_display(Hash_table *);
void up_hash_destroy(Hash_table *);
void up_hash_destroy_retain_element(Hash_table *);
D_array* up_hash_dump_darray(Hash_table *);

int up_hash_del_element(Hash_table *, void *);
int up_hash_del_node(Hash_table *, Hash_node *);


Hash_iterator* up_hash_iterator_init(Hash_table *);
Hash_node* up_hash_iterator_next(Hash_iterator *);
Hash_iterator* up_hash_iterator_dup(Hash_iterator *, Hash_iterator *);
void up_hash_iterator_destroy(Hash_iterator *);
void up_hash_iterator_operate(Hash_table *, void (*)(void*));

void* test_hash_func(void* );

#endif

// update inside or drop repeat outside, that a question
// I choose update inside
