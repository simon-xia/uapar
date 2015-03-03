#include "up_subnet.h"
#include "up_common.h"
#include "up_interface.h"

static void up_subnet_smaller_division_recursive(Hash_table*, Subnet *, short, Hash_table *);

Subnet* up_subnet_init(ip_t net_addr, short mask_len)
{
	Subnet *sub = (Subnet*)malloc(sizeof(Subnet));
	sub -> ip_set = up_darray_init(SUBNET_INIT_SIZE, sizeof(ip_t));
	if (!sub || !(sub->ip_set)) {
		ERROR("not enough memory\n");
		return NULL;
	}

	sub -> mask_addr = up_ip_create_mask(mask_len);
	sub -> net_addr = net_addr;
	sub -> ip_cnt = 0;
	sub -> mask_len = mask_len;
	sub -> broad_addr = up_ip_get_broadcast_addr(net_addr, sub->mask_addr);
	if (mask_len != 31)
		sub -> size = (1 << (32 - mask_len)) - 2;
	else 
		sub -> size = 2;

	return sub;
}

void up_subnet_add_ip(Subnet *sub, ip_t ip)
{
	up_darray_push(&(sub->ip_set), (void*)&ip);
	sub -> ip_cnt++;
}

void up_subnet_init_division(Hash_table *subnet_set, Hash_table *interface_set)
{
	if (!subnet_set || !interface_set) {
		ERROR("argument error\n");
		return ;
	}

	Hash_iterator *iter = up_hash_iterator_init(interface_set);
	Hash_node *tmp_node;
	Subnet *tmp_sub;
	ip_t tmp_ip, tmp_sub_ip;

	while (tmp_node = up_hash_iterator_next(iter)) {
		tmp_ip.int_ip = (unsigned)up_fetch_interface_key(tmp_node->element);
		tmp_sub_ip = up_ip_get_subnet(tmp_ip, INIT_SUBNET_MASK_LEN);

		tmp_sub = up_subnet_init(tmp_sub_ip, INIT_SUBNET_MASK_LEN);
		up_subnet_add_ip(tmp_sub, tmp_ip);

		up_hash_insert(subnet_set, (void*)tmp_sub);
	}

	up_hash_iterator_destroy(iter);
}

void up_subnet_display(void *sub)
{
	char ip_tmp[MAX_IP_ADDR_SIZE];
	char mask_tmp[MAX_IP_ADDR_SIZE];
	int i;
	printf("\tsubnet: %s\tmask: %s\tsize: %u\ttotal interface %u\n\t\t\t", ip_to_str(((Subnet*)sub) -> net_addr, ip_tmp), ip_to_str(((Subnet*)sub) -> mask_addr, mask_tmp), ((Subnet*)sub)->size, ((Subnet*)sub)->ip_cnt);
	for (i = 0; i < ((Subnet*)sub)->ip_cnt; i++)
		up_ip_display(up_darray_ith_addr(((Subnet*)sub) -> ip_set, i));	
	printf("\n");
}

// for hash table to get key
void* up_subnet_fetch_key(void* sub)
{
	return (void*)((((Subnet*)sub) -> net_addr).int_ip);
}

void up_subnet_destroy(void *sub)
{
	up_darray_destroy(((Subnet*)sub) -> ip_set);
	free(sub);
}

// merge two subnet obj, use in hash table
void up_subnet_update(void *sb_old, void *sb_new)
{
	up_darray_push(&(((Subnet*)sb_old)->ip_set),up_darray_ith_addr(((Subnet*)sb_new)->ip_set, 0));
	((Subnet*)sb_old)->ip_cnt++;
}

int up_subnet_vaild_sp2(Subnet *sub)
{
	if (sub->mask_len == 31)
		return UP_TRUE;

	return !up_darray_find(sub->ip_set, (void*)&(sub->net_addr)) && !up_darray_find(sub->ip_set, (void*)&(sub->broad_addr));
}

double up_subnet_get_integrity(Subnet *sub)
{
	if (sub->mask_len >= 30)
		return 1.0;

	return (double)(sub->ip_cnt) / sub->size;
}

static void up_subnet_smaller_division_recursive(Hash_table* sub_set, Subnet *sub, short mask_len, Hash_table *interface_set)
{
	if (up_subnet_vaild_sp2(sub) && up_subnet_get_integrity(sub) >= SUBNET_INTEGRITY_RATIO && up_subnet_check_correctness(sub, interface_set)) {
		if (mask_len == INIT_SUBNET_MASK_LEN) {
			Subnet *sub_copy = up_subnet_dup(sub);
			up_hash_insert(sub_set, (void*)sub_copy);
		}
		up_hash_insert(sub_set, (void*)sub);
		// continue divi
		
		return ;
	}
	else if (sub->ip_cnt >= 3) {
		int i;
		ip_t new_net_addr, tmp_ip;
		new_net_addr.int_ip = sub->net_addr.int_ip | (1 << (32 - sub->mask_len - 1));
		Subnet *sub1 = up_subnet_init(sub->net_addr, mask_len+1);
		Subnet *sub2 = up_subnet_init(new_net_addr, mask_len+1);

		for (i = 0; i < sub->ip_cnt; i++)
		{
			tmp_ip = *(ip_t*)up_darray_ith_addr(sub->ip_set, i);
			if (up_ip_is_insubnet(tmp_ip, new_net_addr, up_ip_create_mask(mask_len+1))) 
				up_subnet_add_ip(sub2, tmp_ip);
			else 
				up_subnet_add_ip(sub1, tmp_ip);
		}

		up_subnet_smaller_division_recursive(sub_set, sub1, mask_len+1, interface_set);
		up_subnet_smaller_division_recursive(sub_set, sub2, mask_len+1, interface_set);
	}

	if (mask_len != INIT_SUBNET_MASK_LEN) {
		up_subnet_destroy((void*)sub);
	}
}

void up_subnet_smaller_division(Hash_table* sub_set, Subnet *sub, Hash_table *interface_set)
{
	return up_subnet_smaller_division_recursive(sub_set, sub, INIT_SUBNET_MASK_LEN, interface_set);
}

int up_subnet_check_correctness(Subnet *sub, Hash_table *interface_set)
{
	int i = 0, j;
	Interface *it1, *it2;
	for (; i < sub->ip_cnt; i++)
	{
		it1 = up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, i)).int_ip))->element;
		for (j = i+1; j < sub->ip_set->len; j++)
		{
			it2 = up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, j)).int_ip))->element;
			if (up_interface_same_path(it1, it2))
				return UP_FALSE;
		}
	}
	return UP_TRUE;
}

// used after init up_subnet_init_division()
void up_subnet_advanced_division(Hash_table **subnet_set, Hash_table* interface_set)
{

//	up_hash_display(*subnet_set);

	Hash_table *tmp_sub_set = up_hash_init(SUBNET_HASH_SLOT_SIZE, test_hash_func, up_subnet_update, up_subnet_fetch_key, up_subnet_destroy, up_subnet_display);
	Hash_node *tmp_node;

	Hash_iterator *iter = up_hash_iterator_init(*subnet_set);
	while (tmp_node = up_hash_iterator_next(iter)) {
		up_subnet_smaller_division(tmp_sub_set, (Subnet*)(tmp_node->element), interface_set);
	}
	up_hash_iterator_destroy(iter);


	//replace subnet_set
	up_hash_destroy(*subnet_set);
	*subnet_set = tmp_sub_set;

	// drop /30 /31 subnets which only contain one or zero iterface, PS: why zero?
	up_subnet_drop_tiny(*subnet_set);
}

// drop subnet only contain 1 interface
void up_subnet_drop_tiny(Hash_table *subnet_set)
{
	Hash_iterator *iter = up_hash_iterator_init(subnet_set);
	Hash_iterator *pre_iter = up_hash_iterator_init(subnet_set);
	Hash_node *iter_tmp;
	while (iter_tmp = up_hash_iterator_next(iter)) {
		if (((Subnet*)(iter_tmp -> element))->ip_cnt <= 1) {
			char ip_tmp[MAX_IP_ADDR_SIZE];
			TRACE("del: %s\n", ip_to_str(((Subnet*)(iter_tmp->element))->net_addr, ip_tmp));
			up_hash_del_node(subnet_set, iter_tmp);
			iter = up_hash_iterator_dup(iter, pre_iter);
			continue;
		}
		pre_iter = up_hash_iterator_dup(pre_iter, iter);
	}
	up_hash_iterator_destroy(iter);
	up_hash_iterator_destroy(pre_iter);
}

Subnet *up_subnet_dup(Subnet *old)
{
	Subnet *new = (Subnet*)malloc(sizeof(Subnet));
	if (!new) {
		ERROR("Not enough memory\n");
		return NULL;
	}
	memcpy(new, old, sizeof(Subnet));
	new->ip_set = up_darray_dup(old->ip_set);
	return new;
}

/*
 * compare two subnets' reliablity
 * ret val: 1 for sub1 > sub2, 0 for equal, -1 for <
 */
int up_subnet_cmp(void *sub1, void *sub2)
{
	if ((*(Subnet**)sub1)->mask_len == (*(Subnet**)sub2)->mask_len) {
		double f1 = up_subnet_get_integrity(*(Subnet**)sub1);
		double f2 = up_subnet_get_integrity(*(Subnet**)sub2);
		if (up_float_equal(f1, f2)) {
			return 0;
		}
		else
			return f1 > f2 ? 1 : -1;
	}
	else 
		return (*(Subnet**)sub1)->mask_len > (*(Subnet**)sub2)->mask_len ? 1 : -1;
}
