/*
 * Uapar is a large scale IP alias resolution tool, which is based on APAR and make a few improvements
 *	
 * Version:1.0
 *	
 *								By  
 *									Simon Xia @ UESTC
 *								simonxiaOMG@gmail.com
 */
#include "up_common.h"
#include "up_load_paths.h"
#include "up_hash.h"
#include "up_path.h"
#include "up_subnet.h"
#include "up_darray.h"
#include "up_alias.h"

#ifndef DEBUG_MAIN
Interface *up_anonymous_interface;

typedef struct up_cfg {
	Log_level log_level;
	char *log_file;
	char **inputfile;
	unsigned inputfile_cnt;
}up_cfg;

void up_cfg_init(up_cfg *cfg)
{
	cfg -> log_level = LOG_WARNING;
	cfg -> log_file = NULL;
	cfg -> inputfile = NULL;
	cfg -> inputfile_cnt = 0;
}

void up_usage()
{
	fprintf(stderr, "Usage: ./uapar [OPTION]...\n \
\t-i:\tinput files \033[1;31m(cannot be omitted)\033[0m\n \
\t-l:\tlog level(1~5), default: 3:\n \
\t\t\t*1 LOG_TRACE\n \
\t\t\t*2 LOG_DEBUG\n \
\t\t\t*3 LOG_WARNING\n \
\t\t\t*4 LOG_ERROR\n \
\t\t\t*5 LOG_STATE\n \
\t-o:\tlog file name (default: stderr)\n");
}


int main(int argc, char **argv)
{
	int i = 1, j;
	up_cfg global_cfg;

	up_cfg_init(&global_cfg);

	for (; i < argc; i++)
	{
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'l':
					global_cfg.log_level = atoi(argv[i+1]);
					break;
				case 'o':
					global_cfg.log_file = argv[i+1];
					break;
				case 'i':
					global_cfg.inputfile = &argv[i+1];
					for (j = i+1; j < argc && argv[j][0] != '-'; j++)
						global_cfg.inputfile_cnt++;
					break;
				default:
						up_usage();
						return UP_ERR; 
			}
		}
	}

	if (up_log_global_init(global_cfg.log_file, global_cfg.log_level) == UP_ERR) {
		fprintf(stderr, "fail to initialize log!\n");
		return UP_ERR;
	}
	if (!global_cfg.inputfile) {
		ERROR("No input files!\n");
		return UP_ERR;
	}

	TRACE("config: log level %d\tlog file:%s\tinput count:%d\n",global_cfg.log_level, global_cfg.log_file, global_cfg.inputfile_cnt);
	for (i = 0; i < global_cfg.inputfile_cnt; i++)
		TRACE("%s\n", global_cfg.inputfile[i]);

	ip_t up_anonymous_ip;
	up_anonymous_ip.int_ip = UP_ANONYMOUS_IP;
	up_anonymous_interface = up_interface_init(up_anonymous_ip);

	unsigned pre_total_path = 0, pre_total_interface = 0;

	// a set of each path obj's addr
	D_array *path_set = up_darray_init(INIT_PATH_SET_SIZE, sizeof(Path*));

	Hash_table *interface_set = up_hash_init(INTERFACE_HASH_SLOT_SIZE, test_hash_func, up_interface_update, up_fetch_interface_key, up_interface_destroy, up_interface_display);

	Hash_table *subnet_set = up_hash_init(SUBNET_HASH_SLOT_SIZE, test_hash_func, up_subnet_update, up_subnet_fetch_key, up_subnet_destroy, up_subnet_display);

	STATE("start to read data...\n");
	for (i = 0; i < global_cfg.inputfile_cnt; i++)
	{
		if (load_paths(global_cfg.inputfile[i], &path_set, interface_set, pre_total_path) == UP_ERR) {
			ERROR("fail to load paths from %s\n", global_cfg.inputfile[i]);
			return UP_ERR;
		}
		STATE("read %s finished, %u paths, %u Interfaces\n", global_cfg.inputfile[i], path_set->len - pre_total_path, interface_set->node_cnt - pre_total_interface);
		pre_total_path = path_set->len;
		pre_total_interface = interface_set->node_cnt;
	}
	STATE("== all data read finished ==\n\t\t %u paths\t %u interface\n",path_set->len, interface_set->node_cnt);

//	up_hash_display(interface_set);

	/*
	printf("\n\t#### all paths ####\n\n");
	for (i = 0; i < path_set -> len; i++)
		up_path_display((void*)(*(Path**)up_darray_ith_addr(path_set, i)));
		*/

	up_subnet_init_division(subnet_set, interface_set);
	
	STATE("Get %u subnets after initialized subnet division\n", subnet_set->node_cnt);
//	up_hash_display(subnet_set);

	up_subnet_advanced_division(&subnet_set, interface_set);

	STATE("Get %u subnets after advanced subnet division for later use\n", subnet_set->node_cnt);

//	up_hash_display(subnet_set));

	D_array* subnet_array = up_hash_dump_darray(subnet_set);

	up_darray_sort(subnet_array, up_subnet_cmp);

	if (subnet_array) {
		printf("\n\n########## after division #########\n\ttotal subnet: %u\n", subnet_array->len);
		for (i = 0; i < subnet_array -> len; i++)
			up_subnet_display(*(Subnet**)up_darray_ith_addr(subnet_array, i));
	}

	// alias resolution
	STATE("start to resolve alias...\n");
	D_array *alias_set = up_alias_resolution(interface_set, subnet_array, subnet_set, path_set);

	STATE("alias resolution finished\n");
	if (alias_set) {
		printf("\n\n########## alias resolution results #########\n\ttotal alias: %u\n", alias_set->len);
		for (i = 0; i < alias_set -> len; i++)
			up_alias_display(*(D_array**)up_darray_ith_addr(alias_set, i));
	}

	
	if (alias_set) {
		for (i = 0; i < alias_set -> len; i++)
			up_darray_destroy(*(D_array**)up_darray_ith_addr(alias_set, i));
	}
	up_darray_destroy(alias_set);

	up_hash_destroy_retain_element(subnet_set);

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
#endif
