#include "up_ip4.h"
#include "up_hash.h"
#include "up_path.h"
#include "up_darray.h"
#include "up_interface.h"
#include "up_common.h"
#include "up_load_paths.h"

#define MAX_INPUT_BUF_SIZE  512
#define TMP_ADDR_SET_SIZE   8
#define MAX_IP_ADDR_LEN		16

int drop_loops(D_array *array)
{
	int i, j, loop_start; 

start:
	loop_start = 0;
	for (i = 0; i < array->len; i++)
		for (j = i+1; j < array->len; j++)
			if (!DARRAY_ELE_CMP(array, i, j)) {
				up_darray_delete_segment(array, i, j-1);
				goto start;
			}
}

int load_paths(char *filename, D_array **path_set, Hash_table *it_set)
{
	char tmpbuf[MAX_INPUT_BUF_SIZE] = {0}, tmp_ip[MAX_IP_ADDR_LEN], *ip, *ip_pre;
	char *split_char = " \t\n";
	int flag_path_completion = 1, path_count = 1;
	ip_t tmp_it_addr;
	unsigned tmp_int_ip;
	Hash_node *tmp_node;
	Interface *tmp_it;
	Pos_on_path tmp_it_pos;

	D_array *tmp_addrset = up_darray_init(TMP_ADDR_SET_SIZE, sizeof(ip_t));

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		perror("Open failed");
		return 1;
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
			printf("output: %s\n", ip);
			tmp_int_ip = str_to_int_ip(ip);
			up_darray_push(&tmp_addrset, (void*)&tmp_int_ip);
			ip = strtok(NULL, split_char);
		}

		if (flag_path_completion) {
			// process path loop
			drop_loops(tmp_addrset);
			printf("\t\t == read path %d finished ==\n", path_count);
			Path* tmp_path = up_path_init(path_count);
			
			int k;
			for (k = 0; k < tmp_addrset->len; k++)
			{
				tmp_it_addr.int_ip = *(unsigned*)up_darray_get_element(tmp_addrset, k);
				printf("node %2d:%u\t", k, tmp_it_addr.int_ip);
				display_addr(tmp_it_addr, stdout);
				printf("\n");

				tmp_it_pos.path_id = path_count;
				tmp_it_pos.pos = k + 1; // position count from 1

				tmp_node = up_hash_lookup(it_set, (void*)tmp_it_addr.int_ip);
				if (tmp_node) {
					tmp_it = (Interface*)(tmp_node -> element);
					interface_addpath(tmp_it, (void*)&tmp_it_pos);
				}
				else {
					tmp_it = interface_init(tmp_it_addr); 
					interface_addpath(tmp_it, (void*)&tmp_it_pos);
					up_hash_insert(it_set, (void*)tmp_it);	
				}

				up_path_add_interface(tmp_path, tmp_it);

			}
			up_darray_clear(tmp_addrset);
			up_darray_push(path_set, (void*)tmp_path);

			path_count++;
		}

		memset(tmpbuf, 0x00, MAX_INPUT_BUF_SIZE);
	}

	return 0;
}

int main()
{
	char *filename = "./input_path.txt";

	D_array *path_set = up_darray_init(INIT_PATH_SET_SIZE, sizeof(Path));
	Hash_table *interface_set = up_hash_init(INTERFACE_HASH_BUCKET_SIZE, test_hash_func, interface_update, fetch_interface_key, interface_destroy, interface_display);

	load_paths(filename, &path_set, interface_set);

	up_hash_display(interface_set);

	int i;
	for (i = 0; i < path_set -> len; i++)
		up_path_display((Path*)DARRAY_I_ADDR(path_set, i));


	for (i = 0; i < path_set -> len; i++)
		up_path_destroy((Path*)DARRAY_I_ADDR(path_set, i));

	up_darray_destroy(path_set);
	up_hash_destroy(interface_set);
	return 0;
}
