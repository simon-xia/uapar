#include "up_alias.h"
#include "up_darray.h"
#include "up_hash.h"
#include "up_subnet.h"

D_array* up_alias_new_pair(unsigned ip1, unsigned ip2)
{
	D_array *pair = up_darray_init(ALIAS_PAIR_INIT_SIZE, sizeof(unsigned));
	up_darray_push(&pair, (void*)&ip1);
	up_darray_push(&pair, (void*)&ip2);
	return pair;
}

int up_alias_loop_detect(Path *path, Interface* old, Interface* new)
{
	int old_index = up_darray_find_index(path->interface_set, up_interface_int_ip(old));
	Interface *tmp = path->interface_set[old_index];

	path->interface_set[old_index] = new;
	int ret = up_path_loop_detect(path);
	path->interface_set[old_index] = tmp;

	return ret;
}

D_array *up_alias_resolution(Hash_table *interface_set, Hash_table *subnet_set, D_array *path_set)
{
	D_array *alias_set = up_darray_init(ALIAS_SET_INIT_SIZE, sizeof(D_array*));
	Hash_node *tmp_node;
	Subnet *tmp_subnet;

	Hash_iterator *subnet_iter = up_hash_iterator_init(subnet_set);
	while(tmp_node = up_hash_iterator_next(subnet_iter)) {
		tmp_subnet = (Subnet*)(tmp_node->element);
		up_alias_process_one_sub(tmp_subnet, interface_set, path_set, alias_set);
	}
	up_hash_iterator_destroy(subnet_iter);

	return alias_set;
}

static void up_alias_process_one_sub(Subnet *sub, Hash_table*interface_set, D_array *path_set, D_array *alias_set)
{
	unsigned i, j;
	Interface *it_p, *it_v;
	for (i = 0; i < sub->ip_cnt; i++)
	{
		it_p = (Interface*)(up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, i)).int_ip))->element);
		for (j = 0; j < sub->ip_cnt; j++)
		{
			it_v = (Interface*)(up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, j)).int_ip))->element);
			up_alias_process_two_ip(it_p, it_v, path_set, alias_set);
		}
	}
}

static void up_alias_process_two_ip(Interface *it_p, Interface *it_v, D_array *path_set, D_array *alias_set)
{
	void *tmp_addr = NULL, *tmp_addr2 = NULL;
	Path *tmp_path_p, *tmp_path_v;
	unsigned ip_p_pre, ip_p_ppre, ip_v_next;
	Interface *it_p_pre;
	int tmp_pos_itp;

	D_array_iterator *iter_p = up_darray_iterator_init(it_p->pos_on_path_set);

	while (tmp_addr = up_darray_iterator_next(iter_p)) {

		tmp_path_p = *(Path**)up_darray_ith_addr(path_set, ((Pos_on_path*)tmp_addr)->path_id - 1);
		tmp_pos_itp = ((Pos_on_path*)tmp_addr) -> pos;
		it_p_pre = *(Interface**)up_darray_ith_addr(tmp_path_p->interface_set, tmp_pos_itp - 2);
		ip_p_pre = up_interface_int_ip(it_p_pre);
		ip_p_ppre = up_interface_int_ip(*(Interface**)up_darray_ith_addr(tmp_path_p->interface_set, tmp_pos_itp - 3));

		if (ip_p_pre != UP_ANONYMOUS_IP && up_alias_loop_detect(tmp_path_p, it_p_pre, it_v)) {
			
			D_array_iterator *iter_v = up_darray_iterator_init(it_v->pos_on_path_set);
			while (tmp_addr2 = up_darray_iterator_next(iter_v)) {

				tmp_path_v = *(Path**)up_darray_ith_addr(path_set, ((Pos_on_path*)tmp_addr2)->path_id - 1);
				tmp_pos_itv = ((Pos_on_path*)tmp_addr) -> pos;
				ip_v_next = up_interface_int_ip(*(Interface**)up_darray_ith_addr(tmp_path_v->interface_set, tmp_pos_itv));

				if (ip_p_ppre != UP_ANONYMOUS_IP) {
					if (ip_p_ppre == ip_v_next || up_alias_isalias(alias_set, ip_p_pre, ip_v_next)) {
						up_alias_add_new_pair(alias_set, ip_p_pre, up_interface_int_ip(it_v));
						goto: succ;
					}
				}
				else if (up_alias_under_same_subnet(ip_p_pre, ip_v_next)) {
						up_alias_add_new_pair(alias_set, ip_p_pre, up_interface_int_ip(it_v));
						goto: succ;
				}
			}
			up_darray_iterator_destroy(iter_v);
		}
	}

	up_darray_iterator_destroy(iter_p);
	return;

succ:
	up_darray_iterator_destroy(iter_v);
	up_darray_iterator_destroy(iter_p);
}

int up_alias_under_same_subnet(unsigned ip1, unsigned ip2, Hash_table* subnet_set)
{
	unsigned common_prefix_len = up_ip_get_common_prefix((ip_t)ip1, (ip_t)ip2), i;
	ip_t tmp_sub_addr;
	Hash_node *tmp_node;
	void *tmp_ip_addr;
	D_array *sub_ip_iter;

	for (i = common_prefix_len; i > 0; i--)
	{
		tmp_sub_addr = up_ip_get_subnet((ip_t)ip1, i);
		tmp_node = up_hash_lookup(subnet_set, (void*)(tmp_sub_addr.int_ip));
		if (!tmp_node)
			continue;

		sub_ip_iter = up_darray_iterator_init(((Subnet*)(tmp_node -> element))->ip_set);
		while (tmp_ip_addr = up_darray_iterator_next(sub_ip_iter)) {
			if (*(unsigned*)tmp_ip_addr == ip2)
				goto succ_find;
		}
		up_darray_iterator_destroy(sub_ip_iter);
	}
	
	return UP_FALSE;

succ_find:
	up_darray_iterator_destroy(sub_ip_iter);
	return UP_TRUE;
}

void up_alias_add_new_pair(D_array **alias_set, unsigned ip1, unsigned ip2)
{
	D_array *tmp;
	if (tmp = up_alias_find_ip(*alias_set, ip1)) {
		up_darray_push(&tmp, (void*)&ip2);
	}
	else if (tmp = up_alias_find_ip(*alias_set, ip2)) {
		up_darray_push(&tmp, (void*)&ip2);
	}
	else {
		D_array *new_pair = up_alias_new_pair(ip1, ip2);
		up_darray_push(alias_set, (void*)&new_pair);
	}
}

D_array* up_alias_find_ip(D_array *alias_set, unsigned ip)
{
	D_array_iterator *slot_iter = up_darray_iterator_init(alias_set);
	D_array_iterator *pair_iter;
	void *tmp_slot_addr = NULL, *tmp_pair_addr = NULL;

	while (tmp_slot_addr = up_darray_iterator_next(slot_iter)) {
		pair_iter = up_darray_iterator_init(*(D_array**)tmp_slot_addr);
		while (tmp_pair_addr = up_darray_iterator_next(pair_iter)) {
			if (ip == *(unsigned*)tmp_pair_addr)
				goto: succ_find;
		}
		up_darray_iterator_destroy(pair_iter);
	}
	up_darray_iterator_destroy(slot_iter);

	return NULL;

succ_find:
	up_darray_iterator_destroy(pair_iter);
	up_darray_iterator_destroy(slot_iter);
	return *(D_array**)tmp_slot_addr;
}

int up_alias_isalias(D_array *alias_set, unsigned ip1, unsigned ip2)
{
	D_array *tmp = up_alias_find_ip(alias_set, ip1); 
	if (!tmp || !up_darray_find(tmp, (void*)&ip2))
		return UP_FALSE;
	return UP_TRUE;
}
