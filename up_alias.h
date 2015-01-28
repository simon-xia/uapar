/*
 * 
 *						Jan.27 2015
 */
#ifndef _UP_ALIAS_H_
#define _UP_ALIAS_H_

#define ALIAS_SET_INIT_SIZE		20
#define ALIAS_PAIR_INIT_SIZE	2


D_array* up_alias_new_pair(unsigned , unsigned );
int up_alias_loop_detect(Path *path, Interface* , Interface* );
D_array *up_alias_resolution(Hash_table *, Hash_table *, D_array *);
static void up_alias_process_one_sub(Subnet *, Hash_table*, D_array *, D_array *);

static void up_alias_process_two_ip(Interface *, Interface*, D_array *, D_array *);
int up_alias_under_same_subnet(unsigned , unsigned , Hash_table* );
void up_alias_add_new_pair(D_array **, unsigned , unsigned );
D_array* up_alias_find_ip(D_array *, unsigned );
int up_alias_isalias(D_array *, unsigned , unsigned );

#endif
