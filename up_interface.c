#include "up_interface.h"
#include "up_path.h"
#include "up_common.h"


Interface * interface_init(ip_t addr)
{
	Interface *it = (Interface*)malloc(sizeof(Interface));
	it -> addr = addr;
	it -> path_cnt = 0;
	it -> pos_on_path_set = up_darray_init(INIT_INTERFACE_PATH_CNT, sizeof(Pos_on_path));

	return it;
}

void* fetch_interface_key(void *it)
{
	return (void*)((((Interface*)it) -> addr).int_ip);
}

// merge, use in hash
void interface_update(void *it_old, void *it_new)
{
	up_darray_push(&(((Interface*)it_old)->pos_on_path_set), up_darray_get_element(((Interface*)it_new)->pos_on_path_set, 0));
	((Interface*)it_old)->path_cnt++;
}

void interface_addpath(Interface *it, void *p)
{
	up_darray_push(&(it->pos_on_path_set), p);
	it->path_cnt++;
}

void interface_display(void *it)
{
	display_addr(((Interface*)it) -> addr, stdout);
	printf("\tpath_cnt: %d\n", ((Interface*)it) -> path_cnt);
	unsigned i = 0;
	for (; i < ((Interface*)it)->pos_on_path_set->len; i++)
	{
		printf("\t\t\tpath:%4d\tposition:%3d\n", ((Pos_on_path*)up_darray_get_element(((Interface*)it)->pos_on_path_set, i))->path_id, ((Pos_on_path*)up_darray_get_element((((Interface*)it))->pos_on_path_set, i))->pos);
	}
}

void interface_destroy(void *it)
{
	up_darray_destroy(((Interface*)it)->pos_on_path_set);
	free(((Interface*)it));
}
