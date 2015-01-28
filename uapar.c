#include "up_common.h"
#include "up_load_paths.h"
#include "up_hash.h"
#include "up_path.h"
#include "up_subnet.h"
#include "up_darray.h"

Interface *up_anonymous_interface;

int main()
{
	//up_log_global_init(NULL, LOG_TRACE);
	up_log_global_init(NULL, LOG_WARNING);

	ip_t up_anonymous_ip;
	up_anonymous_ip.int_ip = UP_ANONYMOUS_IP;
	up_anonymous_interface = up_interface_init(up_anonymous_ip);

	//char *filename_set[] = {"./input_path.txt","./5k.txt", "./path_0906_cn.txt"};
	//char *filename_set[] = {"./5k.txt"};
	char *filename_set[] = {"./input_path.txt", "./5k.txt"};
	//char *filename_set[] = {"./100.txt", "./input_path.txt"};
	//char *filename = "./5k.txt";
	//char *filename;
	unsigned pre_total_path = 0, pre_total_interface = 0;
	int i;

	// a set of each path obj's addr
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

//	up_hash_display(interface_set);

	printf("\n\t#### all paths ####\n\n");
	for (i = 0; i < path_set -> len; i++)
		up_path_display((void*)(*(Path**)up_darray_ith_addr(path_set, i)));

	up_subnet_init_division(subnet_set, interface_set);
	
	STATE("Get %u subnets after initialized subnet division\n", subnet_set->node_cnt);
//	up_hash_display(subnet_set);

	up_subnet_advanced_division(&subnet_set, interface_set);

	STATE("Get %u subnets after advanced subnet division for later use\n", subnet_set->node_cnt);

//	printf("\n\n########## after division #########\n");
//	up_hash_display(subnet_set);

	D_array* subnet_array = up_hash_dump_darray(subnet_set);

	up_hash_destroy_retain_element(subnet_set);

	up_darray_sort(subnet_array, up_subnet_cmp);

	if (subnet_array) {
		printf("\n\n########## after division #########\n\ttotal subnet: %u\n", subnet_array->len);
		for (i = 0; i < subnet_array -> len; i++)
			up_subnet_display(*(Subnet**)up_darray_ith_addr(subnet_array, i));
	}

	// alias resolution
//	D_array *alias_set = up_alias_resolution(interface_set, subnet_array, path_set);


	for (i = 0; i < path_set -> len; i++)
		up_path_destroy(*(Path**)up_darray_ith_addr(path_set, i));

	if (subnet_array) {
		for (i = 0; i < subnet_array -> len; i++)
			up_subnet_destroy(*(Subnet**)up_darray_ith_addr(subnet_array, i));
		up_darray_destroy(subnet_array);
	}

	up_darray_destroy(path_set);
	up_hash_destroy(interface_set);

	up_interface_destroy(up_anonymous_interface);
	up_log_global_uninit();

	return UP_SUCC;
}
