#include "up_ip4.h"
#include "up_path.h"
#include "up_interface.h"
#include "up_common.h"
#include "up_load_paths.h"

#define MAX_INPUT_BUF_SIZE  1024
#define TMP_ADDR_SET_SIZE   8
#define MAX_IP_ADDR_LEN		16

int drop_loops(D_array *array)
{
	int i, j; 
	// drop head and tail
	up_darray_delete_one(array, 0);
	up_darray_delete_one(array, array->len - 1);

start:
	for (i = 0; i < array->len; i++)
		for (j = i+1; j < array->len; j++)
			if (!up_darray_ele_cmp_direct(array, i, j)) {
				up_darray_delete_segment(array, i, j-1);
				goto start;
			}
}

int load_paths(char *filename, D_array **path_set, Hash_table *it_set)
{
	char tmpbuf[MAX_INPUT_BUF_SIZE] = {0}, tmp_ip[MAX_IP_ADDR_LEN], *ip, *ip_pre;
	char *split_char = " \t\n";
	int flag_path_completion = 1, path_count = 1;
	unsigned tmp_int_ip;
	Hash_node *tmp_node;

	D_array *tmp_addrset = up_darray_init(TMP_ADDR_SET_SIZE, sizeof(ip_t));

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		ERROR("Open input file failed:%s\n", strerror(errno));
		return UP_ERR;
	}
	
	memset(tmp_ip, 0x00, MAX_IP_ADDR_LEN);
	while(fgets(tmpbuf, MAX_INPUT_BUF_SIZE, fp)) {
		// may read an incomplete path
		if (strchr(tmpbuf, '\n'))
			flag_path_completion = 1;
		else
			flag_path_completion = 0;

		// previous time cut tail, cut head now
		if (tmp_ip[0] != 0x00) {
			for (ip = tmpbuf; isdigit(*ip) || *ip == '.'; ip++) ;
			strncpy(tmp_ip + strlen(tmp_ip), tmpbuf, ip - tmpbuf);
			printf("cat:%s\n", tmp_ip);

			tmp_int_ip = str_to_int_ip(tmp_ip);
			up_darray_push(&tmp_addrset, (void*)&tmp_int_ip);

			memset(tmpbuf, 0x20, ip - tmpbuf); // filled with blank
			memset(tmp_ip, 0x00, MAX_IP_ADDR_LEN);
		}

		//cut tail
		if (!flag_path_completion) {
			//find last piece ip start position
			for (ip = tmpbuf + MAX_INPUT_BUF_SIZE - 2; isdigit(*ip) || *ip == '.'; ip--) ;
			// mark
			strncpy(tmp_ip, ip + 1, tmpbuf + MAX_INPUT_BUF_SIZE - 2 - ip);
			//ignore last incomplete piece ip
			memset(ip, 0x00, tmpbuf + MAX_INPUT_BUF_SIZE - 2 - ip);
		}

		ip = strtok(tmpbuf, split_char);

		while (ip) {
			//printf("output: %s\n", ip);
			tmp_int_ip = str_to_int_ip(ip);
			up_darray_push(&tmp_addrset, (void*)&tmp_int_ip);
			//STATE("read ip: %s\n", ip);
			ip = strtok(NULL, split_char);
		}

		if (flag_path_completion) {
			drop_loops(tmp_addrset);
			up_read_one_complete_path(path_count, tmp_addrset, path_set, it_set);
			STATE(" == read path %d finished ==\n", path_count);
			path_count++;
			up_darray_clear(tmp_addrset);
		}

		memset(tmpbuf, 0x00, MAX_INPUT_BUF_SIZE);
	}
	
	up_darray_destroy(tmp_addrset);
	return UP_SUCC;
}

static int up_read_one_complete_path(int path_cnt, D_array *tmp_addrset, D_array **path_set, Hash_table*it_set)
{
	int k;
	ip_t tmp_it_addr;
	Pos_on_path tmp_it_pos;
	Interface *tmp_it;
	Path* tmp_path = up_path_init(path_cnt);

	for (k = 0; k < tmp_addrset->len; k++)
	{
		tmp_it_addr.int_ip = *(unsigned*)up_darray_ith_addr(tmp_addrset, k);
		/*
		   printf("node %2d:%u\t", k, tmp_it_addr.int_ip);
		   up_ip_display((void*)tmp_it_addr);
		   printf("\n");
		   */

		tmp_it_pos.path_id = path_cnt;
		tmp_it_pos.pos = k + 1; // position count from 1

		tmp_it = up_interface_init(tmp_it_addr); 
		up_interface_addpath(tmp_it, (void*)&tmp_it_pos);

		up_hash_insert(it_set, (void*)tmp_it);	

		up_path_add_interface(tmp_path, (void*)&tmp_it);

	}
	TRACE("get path object[%p]: id:%d hops:%d interface_set:%p\n", tmp_path, tmp_path -> path_id, tmp_path -> hop_count, tmp_path->interface_set);

	up_darray_push(path_set, (void*)&tmp_path);//mark
}

#ifdef UNIT_TEST_LOAD_PATHS
#include "up_subnet.h"

int main()
{
	up_log_global_init(NULL, LOG_TRACE);
	//up_log_global_init(NULL, LOG_WARNING);

	char *filename = "./input_path.txt";
	//char *filename = "./zhuo.txt";
	int i;

	// a set of each path obj's addr
	D_array *path_set = up_darray_init(INIT_PATH_SET_SIZE, sizeof(Path*));
	Hash_table *interface_set = up_hash_init(INTERFACE_HASH_SLOT_SIZE, test_hash_func, up_interface_update, up_fetch_interface_key, up_interface_destroy, up_interface_display);

	Hash_table *subnet_set = up_hash_init(SUBNET_HASH_SLOT_SIZE, test_hash_func, up_subnet_update, up_subnet_fetch_key, up_subnet_destroy, up_subnet_display);

	load_paths(filename, &path_set, interface_set);

	up_hash_display(interface_set);

	for (i = 0; i < path_set -> len; i++)
		up_path_display((void*)(*(Path**)up_darray_ith_addr(path_set, i)));

//	up_subnet_init_division(subnet_set, interface_set);
	
	//up_hash_display(subnet_set);
//	up_subnet_advanced_division(&subnet_set, interface_set);

//	printf("\n\n############### after del #########\n");
//	up_hash_display(subnet_set);


	for (i = 0; i < path_set -> len; i++)
		up_path_destroy(*(Path**)up_darray_ith_addr(path_set, i));

	up_darray_destroy(path_set);
	up_hash_destroy(interface_set);
	up_hash_destroy(subnet_set);


	up_log_global_uninit();
	return 0;
}

#endif
