/*
 * 
 *						Jan.27 2015
 */
#ifndef _UP_ALIAS_H_
#define _UP_ALIAS_H_

#include "up_alias.h"
#include "up_path.h"
#include "up_darray.h"
#include "up_hash.h"
#include "up_subnet.h"

//typedef struct d_array D_array;
//typedef struct hash_table Hash_table;
//typedef struct interface Interface; 
//typedef struct path		Path;
//typedef struct subnet	Subnet;

#define ALIAS_SET_INIT_SIZE		20
#define ALIAS_PAIR_INIT_SIZE	2


D_array* up_alias_new_pair(unsigned , unsigned );
int up_alias_loop_detect(Path *path, int, Interface* );
D_array *up_alias_resolution(Hash_table *, D_array*, Hash_table *, D_array *);

int up_alias_in_same_subnet(unsigned , unsigned , Hash_table* );
void up_alias_add_new_pair(D_array **, unsigned , unsigned );
D_array* up_alias_find_ip(D_array *, unsigned );
int up_alias_isalias(D_array *, unsigned , unsigned );


#endif
