#include "up_common.h"
#include "up_alias.h"

static void up_alias_process_one_sub(Subnet *, Hash_table*, D_array *, Hash_table*, D_array **);
static void up_alias_process_two_ip(Interface *, Interface*, D_array *, Hash_table*, D_array **);

D_array* up_alias_new_pair(unsigned ip1, unsigned ip2)
{
	D_array *pair = up_darray_init(ALIAS_PAIR_INIT_SIZE, sizeof(unsigned));
	up_darray_push(&pair, (void*)&ip1);
	up_darray_push(&pair, (void*)&ip2);
	return pair;
}

int up_alias_loop_detect(Path *path, int old_index, Interface* new)
{
	Interface *tmp = *(Interface**)up_darray_ith_addr(path->interface_set, old_index);

	up_darray_set_i(path->interface_set, old_index, (void*)&new);
	int ret = up_path_loop_detect(path);
	up_darray_set_i(path->interface_set, old_index, (void*)&tmp);

	TRACE("ret: %s\n", ret ? "find loop" : "no loop");
	return ret;
}

D_array *up_alias_resolution(Hash_table *interface_set, D_array* subnet_array, Hash_table *subnet_set, D_array *path_set)
{
	TRACE("--->\n");
	if (!subnet_array)
		return NULL;
	int tmp_cnt = 1;
	D_array *alias_set = up_darray_init(ALIAS_SET_INIT_SIZE, sizeof(D_array*));
	void *tmp_addr;

	D_array_iterator *subnet_iter = up_darray_iterator_init(subnet_array);
	while(tmp_addr = up_darray_iterator_next(subnet_iter)) {
		STATE("start process subnet: No.%d\n", tmp_cnt++);
		up_alias_process_one_sub(*(Subnet**)tmp_addr, interface_set, path_set, subnet_set, &alias_set);
	}
	up_darray_iterator_destroy(subnet_iter);

	TRACE("<---\n");
	return alias_set;
}

static void up_alias_process_one_sub(Subnet *sub, Hash_table*interface_set, D_array *path_set, Hash_table *subnet_set, D_array **alias_set)
{
	TRACE("--->\n");
	TRACE("subnet: %u.%u.%u.%u\n", (sub->net_addr).dot_ip.f1, (sub->net_addr).dot_ip.f2, (sub->net_addr).dot_ip.f3, (sub->net_addr).dot_ip.f4);
	unsigned i, j;
	Interface *it_p, *it_v;
	for (i = 0; i < sub->ip_cnt; i++)
	{
		it_p = (Interface*)(up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, i)).int_ip))->element);
		for (j = i+1; j < sub->ip_cnt; j++)
		{
			it_v = (Interface*)(up_hash_lookup(interface_set, (void*)((*(ip_t*)up_darray_ith_addr(sub->ip_set, j)).int_ip))->element);
			up_alias_process_two_ip(it_p, it_v, path_set, subnet_set, alias_set);
		}
	}
	TRACE("<---\n");
}

/*
 *		ip1_ppre - ip1_pre - ip1
 *								|	
 *					ip2_next - ip2
 *
 */
static void up_alias_process_two_ip(Interface *it1, Interface *it2, D_array *path_set, Hash_table*subnet_set ,D_array **alias_set)
{
	TRACE("--->\n");
	TRACE("ip1: %d.%d.%d.%d\tip2: %d.%d.%d.%d\n", (it1->addr).dot_ip.f1, (it1->addr).dot_ip.f2, (it1->addr).dot_ip.f3, (it1->addr).dot_ip.f4, (it2->addr).dot_ip.f1, (it2->addr).dot_ip.f2, (it2->addr).dot_ip.f3, (it2->addr).dot_ip.f4);

	void *tmp_addr1 = NULL, *tmp_addr2 = NULL;
	Path *tmp_path1, *tmp_path2;
	unsigned ip1_pre, ip1_ppre, ip2_next;
	Interface *it1_pre;
	int tmp_pos_it1, tmp_pos_it2;
	D_array_iterator *iter2;

	D_array_iterator *iter1 = up_darray_iterator_init(it1->pos_on_path_set);

	TRACE("ip1 path count:%u\n", it1->path_cnt);
	unsigned tmp_it1_path_counter = 1, tmp_it2_path_counter;
	while (tmp_addr1 = up_darray_iterator_next(iter1)) {

		tmp_path1 = *(Path**)up_darray_ith_addr(path_set, ((Pos_on_path*)tmp_addr1)->path_id - 1);
		tmp_pos_it1 = ((Pos_on_path*)tmp_addr1) -> pos;
		TRACE("ip1 No.%u path: id:%d\tpos:%d\n", tmp_it1_path_counter++, ((Pos_on_path*)tmp_addr1) -> path_id, ((Pos_on_path*)tmp_addr1) -> pos);

		if (tmp_pos_it1 > 1)
			it1_pre = *(Interface**)up_darray_ith_addr(tmp_path1->interface_set, tmp_pos_it1 - 2);
		else
			continue;

		ip1_pre = up_interface_int_ip(it1_pre);

		if (tmp_pos_it1 > 2)
			ip1_ppre = up_interface_int_ip(*(Interface**)up_darray_ith_addr(tmp_path1->interface_set, tmp_pos_it1 - 3));
		else
			ip1_ppre = UP_NONEXIST_IP;

		TRACE("ip1 pre: %d.%d.%d.%d\tip1 ppre: %d.%d.%d.%d\n", ((ip_t)ip1_pre).dot_ip.f1, ((ip_t)ip1_pre).dot_ip.f2, ((ip_t)ip1_pre).dot_ip.f3, ((ip_t)ip1_pre).dot_ip.f4,((ip_t)ip1_ppre).dot_ip.f1, ((ip_t)ip1_ppre).dot_ip.f2, ((ip_t)ip1_ppre).dot_ip.f3, ((ip_t)ip1_ppre).dot_ip.f4);

		if (ip1_pre != UP_ANONYMOUS_IP && !up_alias_loop_detect(tmp_path1, tmp_pos_it1 - 2, it2)) {
			
			TRACE("ip2 path count:%u\n", it2->path_cnt);
			iter2 = up_darray_iterator_init(it2->pos_on_path_set);
			tmp_it2_path_counter = 1;
			while (tmp_addr2 = up_darray_iterator_next(iter2)) {

				tmp_pos_it2 = ((Pos_on_path*)tmp_addr2) -> pos;
				tmp_path2 = *(Path**)up_darray_ith_addr(path_set, ((Pos_on_path*)tmp_addr2)->path_id - 1);

				TRACE("ip2 No.%u path: id:%d\tpos:%d\n", tmp_it2_path_counter++, ((Pos_on_path*)tmp_addr2) -> path_id, ((Pos_on_path*)tmp_addr2) -> pos);

				if (tmp_pos_it2 < tmp_path2 -> hop_count) {
					ip2_next = up_interface_int_ip(*(Interface**)up_darray_ith_addr(tmp_path2->interface_set, tmp_pos_it2));
					TRACE("ip2 next: %d.%d.%d.%d\n", ((ip_t)ip2_next).dot_ip.f1, ((ip_t)ip2_next).dot_ip.f2, ((ip_t)ip2_next).dot_ip.f3, ((ip_t)ip2_next).dot_ip.f4);
				}
				else {
					TRACE("ip2 has no successor\n");
					continue;
				}

				if (ip1_ppre != UP_NONEXIST_IP && ip1_ppre != UP_ANONYMOUS_IP) {
					if (ip1_ppre == ip2_next || up_alias_isalias(*alias_set, ip1_pre, ip2_next)) {
						up_alias_add_new_pair(alias_set, ip1_pre, up_interface_int_ip(it2));
						goto succ_add;
					}
				}
				else if (up_alias_in_same_subnet(ip1_pre, ip2_next, subnet_set)) {
						up_alias_add_new_pair(alias_set, ip1_pre, up_interface_int_ip(it2));
						goto succ_add;
				}
			}
			up_darray_iterator_destroy(iter2);
		}
	}

	up_darray_iterator_destroy(iter1);
	TRACE("<---Not added\n");
	return;

succ_add:
	up_darray_iterator_destroy(iter2);
	up_darray_iterator_destroy(iter1);
	TRACE("<---added\n");
	return;
}

int up_alias_in_same_subnet(unsigned ip1, unsigned ip2, Hash_table* subnet_set)
{
	unsigned common_prefix_len = up_ip_get_common_prefix((ip_t)ip1, (ip_t)ip2), i;
	ip_t tmp_sub_addr;
	Hash_node *tmp_node;
	void *tmp_ip_addr;
	D_array_iterator *sub_ip_iter;

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
				goto succ_find;
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

int 
up_alias_isalias(D_array *alias_set, unsigned ip1, unsigned ip2)
{
	D_array *tmp = up_alias_find_ip(alias_set, ip1); 
	if (!tmp || !up_darray_find(tmp, (void*)&ip2))
		return UP_FALSE;
	return UP_TRUE;
}

void
up_alias_display(D_array *array)
{
	int i;
	for (i = 0; i < array->len; i++)
		up_ip_display(up_darray_ith_addr(array, i));
	printf("\n");
}

#ifdef UNIT_TEST_ALIAS
#include "up_load_paths.h"
#include "up_test.h"

Interface *up_anonymous_interface;

int main()
{
	up_log_global_init(NULL, LOG_TRACE);

	ip_t up_anonymous_ip;
	up_anonymous_ip.int_ip = UP_ANONYMOUS_IP;
	up_anonymous_interface = up_interface_init(up_anonymous_ip);

	char *filename_set[] = {"./two_test_path.txt"};

	unsigned pre_total_path = 0, pre_total_interface = 0;
	int i;

	D_array *path_set = up_darray_init(INIT_PATH_SET_SIZE, sizeof(Path*));

	Hash_table *interface_set = up_hash_init(INTERFACE_HASH_SLOT_SIZE, test_hash_func, up_interface_update, up_fetch_interface_key, up_interface_destroy, up_interface_display);

	Hash_table *subnet_set = up_hash_init(SUBNET_HASH_SLOT_SIZE, test_hash_func, up_subnet_update, up_subnet_fetch_key, up_subnet_destroy, up_subnet_display);

	for (i = 0; i < sizeof(filename_set)/sizeof(filename_set[0]); i++)
	{
		if (load_paths(filename_set[i], &path_set, interface_set, pre_total_path) == UP_ERR) {
			ERROR("fail to load paths from %s\n", filename_set[i]);
			return UP_ERR;
		}
		STATE("read %s finished, %u paths, %u Interfaces\n", filename_set[i], path_set->len - pre_total_path, interface_set->node_cnt - pre_total_interface);
		pre_total_path = path_set->len;
		pre_total_interface = interface_set->node_cnt;
	}
	STATE("== all data read finished ==\n\t\t %u paths\t %u interface\n",path_set->len, interface_set->node_cnt);

	up_hash_display(interface_set);

	printf("\n\t#### all paths ####\n\n");
	for (i = 0; i < path_set -> len; i++)
		up_path_display((void*)(*(Path**)up_darray_ith_addr(path_set, i)));

	ip_t test_ip1, test_ip2, test_ip3, test_ip4;
	ip_dot_assign(&test_ip1, 62, 157, 251, 17);
	ip_dot_assign(&test_ip2, 202, 125, 128, 157);
	ip_dot_assign(&test_ip3, 202, 125, 128, 153);
	ip_dot_assign(&test_ip4, 111, 0, 1, 11);

	Interface *test_it = up_interface_init(test_ip1);
	Path *test_path = *(Path**)up_darray_ith_addr(path_set, 0);
	up_path_display(test_path);
	for (i = 0; i < test_path->hop_count; i++)
		if (i == 5)
			up_test("loop detect", up_alias_loop_detect(test_path, i, test_it) == UP_FALSE);
		else
			up_test("loop detect", up_alias_loop_detect(test_path, i, test_it) == UP_TRUE);
	up_path_display(test_path);
	up_interface_destroy(test_it);

	up_subnet_init_division(subnet_set, interface_set);
	
	STATE("Get %u subnets after initialized subnet division\n", subnet_set->node_cnt);
	up_hash_display(subnet_set);

	up_test("up_alias_in_same_subnet", up_alias_in_same_subnet(test_ip2.int_ip, test_ip3.int_ip, subnet_set)== UP_TRUE);
	up_test("up_alias_in_same_subnet", up_alias_in_same_subnet(test_ip1.int_ip, test_ip3.int_ip, subnet_set)== UP_FALSE);
	up_test("up_alias_in_same_subnet", up_alias_in_same_subnet(test_ip4.int_ip, test_ip3.int_ip, subnet_set)== UP_FALSE);

	D_array *alias_set = up_darray_init(ALIAS_SET_INIT_SIZE, sizeof(D_array*));
	up_alias_add_new_pair(&alias_set, test_ip2.int_ip, test_ip3.int_ip);

	printf("\n\n########## alias resolution results #########\n\ttotal alias: %u\n", alias_set->len);
	for (i = 0; i < alias_set -> len; i++)
		up_alias_display(*(D_array**)up_darray_ith_addr(alias_set, i));

	up_test("up_alias_isalias", up_alias_isalias(alias_set, test_ip2.int_ip, test_ip3.int_ip) == UP_TRUE);
	up_test("up_alias_isalias", up_alias_isalias(alias_set, test_ip1.int_ip, test_ip3.int_ip) == UP_FALSE);

	// alias resolution
	 
	/*
	D_array *alias_set = up_alias_resolution(interface_set, subnet_array, subnet_set, path_set);
	if (alias_set) {
	}
	*/

	for (i = 0; i < path_set -> len; i++)
		up_path_destroy(*(Path**)up_darray_ith_addr(path_set, i));


	up_darray_destroy(path_set);
	up_hash_destroy(interface_set);
	up_hash_destroy(subnet_set);

	up_interface_destroy(up_anonymous_interface);
	up_log_global_uninit();

	up_test_report();
	return UP_SUCC;
}

#endif
