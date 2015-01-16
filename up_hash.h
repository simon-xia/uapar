/*
 * 
 *						By Simon Xia
 *							Jan. 14 2015
 */
#ifndef _UP_HASH_H_
#define _UP_HASH_H_

#define MAX_HASH_TABLE_SIZE 4096

typedef struct hash_node {
	void *element;		//key is contained in element
	struct hash_node *next;
}Hash_node;

typedef struct hash_table {
	unsigned int slot_size;
	unsigned int node_cnt;
	Hash_node **table_entry;

	void* (*hash_func)(void*);
	void* (*fetch_key)(void*);
	void  (*free_element)(void*);
	void  (*update_element)(void*, void*);
	void  (*display_element)(void*);

}Hash_table;


Hash_table* up_hash_init(int, void* (*)(void*), void (*)(void*, void*), void* (*)(void*), void (*)(void*), void (*)(void*));
Hash_node* up_hash_lookup(Hash_table *, void *);
int up_hash_insert(Hash_table *, void *);
static void up_hash_traverse(Hash_table *, int);
void up_hash_display(Hash_table *);
void up_hash_destroy(Hash_table *);

void* test_hash_func(void* );

#endif
