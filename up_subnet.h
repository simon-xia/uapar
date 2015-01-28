#ifndef _UP_SUBNET_H_
#define _UP_SUBNET_H_

#include "up_ip4.h"
#include "up_hash.h"
#include "up_darray.h"

#define SUBNET_HASH_SLOT_SIZE	100
#define SUBNET_INIT_SIZE		4
#define INIT_SUBNET_MASK_LEN	24
#define SUBNET_INTEGRITY_RATIO	0.5f

typedef struct subnet {
	//int net_id;
	unsigned size;
	short	mask_len;
	ip_t	net_addr;
	ip_t	broad_addr;
	ip_t	mask_addr;
	D_array *ip_set;
	unsigned ip_cnt;
}Subnet;

Subnet* up_subnet_init(ip_t, short);
void up_subnet_add_ip(Subnet *, ip_t);
void up_subnet_display(void*);
void* up_subnet_fetch_key(void*);
void up_subnet_destroy(void *);
void up_subnet_update(void *, void *);
Subnet *up_subnet_dup(Subnet *);

void up_subnet_init_division(Hash_table *, Hash_table *);
void up_subnet_advanced_division(Hash_table **, Hash_table* );
void up_subnet_smaller_division(Hash_table* , Subnet *, Hash_table *);
static void up_subnet_smaller_division_recursive(Hash_table*, Subnet *, short, Hash_table *);
double up_subnet_get_integrity(Subnet *);
int up_subnet_check_correctness(Subnet *, Hash_table *);
int up_subnet_vaild_sp2(Subnet *);

void up_subnet_drop_tiny(Hash_table *);

int up_subnet_cmp(void *, void *);
#endif
