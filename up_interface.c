#include "up_interface.h"
#include "up_path.h"
#include "up_common.h"

/*
struct interface{
	ip_t			addr;
	unsigned		path_cnt;
	D_array			*pos_on_path_set; //through position info, we can find processor and successor
};
*/

Interface * up_interface_init(ip_t addr)
{
	Interface *it = (Interface*)malloc(sizeof(Interface));
	it -> addr = addr;
	it -> path_cnt = 0;
	it -> pos_on_path_set = up_darray_init(INIT_INTERFACE_PATH_CNT, sizeof(Pos_on_path));

	return it;
}

void* up_fetch_interface_key(void *it)
{
	return (void*)((((Interface*)it) -> addr).int_ip);
}

// merge, use in hash
void up_interface_update(void *it_old, void *it_new)
{
	up_darray_push(&(((Interface*)it_old)->pos_on_path_set), up_darray_ith_addr(((Interface*)it_new)->pos_on_path_set, 0));
	((Interface*)it_old)->path_cnt++;
}

void up_interface_addpath(Interface *it, void *p)
{
	up_darray_push(&(it->pos_on_path_set), p);
	it->path_cnt++;
}

void up_interface_display(void *it)
{
	up_ip_display((void*)&(((Interface*)it)->addr));
	printf("\tpath_cnt: %u\n", ((Interface*)it) -> path_cnt);
	unsigned i = 0;
	Pos_on_path *tmp_pos;
	for (; i < ((Interface*)it)->path_cnt; i++)
	{
		tmp_pos = (Pos_on_path*)up_darray_ith_addr(((Interface*)it)->pos_on_path_set, i);
		printf("\t\t\tpath:%4d\tposition:%3d\n", tmp_pos -> path_id, tmp_pos -> pos);
	}
}

void up_interface_destroy(void *it)
{
	up_darray_destroy(((Interface*)it)->pos_on_path_set);
	free(((Interface*)it));
}

int up_interface_same_path(Interface *it1, Interface *it2)
{
	int i, j, path_id1, path_id2;
	for (i = 0; i < it1->pos_on_path_set->len; i++)
	{
		path_id1 = ((Pos_on_path*)up_darray_ith_addr(it1->pos_on_path_set, i))->path_id;
		for (j = 0; j < it2->pos_on_path_set->len; j++)
		{
			path_id2 = ((Pos_on_path*)up_darray_ith_addr(it2->pos_on_path_set, j))->path_id;
			if (path_id1 == path_id2)
				return UP_TRUE;
		}
	}
	return UP_FALSE;
}

#ifdef UNIT_TEST_INTERFACE
#include "up_test.h"

int main()
{
	Interface *it1, *it2, *it3;
	ip_t ip1, ip2, ip3;
	Pos_on_path path1, path2, path3;

	path1.path_id = 8;
	path1.pos = 7;

	path2.path_id = 99;
	path2.pos = 9;

	path3.path_id = 9;
	path3.pos = 9;

	ip_dot_assign(&ip1, 192, 168, 1, 1);
	ip_dot_assign(&ip2, 192, 168, 1, 2);
	ip_dot_assign(&ip3, 192, 168, 1, 3);

	it1 = up_interface_init(ip1);
	it2 = up_interface_init(ip2);
	it3 = up_interface_init(ip3);

	up_interface_addpath(it1, (void*)&path1);
	up_interface_addpath(it1, (void*)&path3);

	up_interface_addpath(it2, (void*)&path2);
	up_interface_addpath(it2, (void*)&path3);

	up_interface_addpath(it3, (void*)&path2);

	up_interface_display(it1);
	up_interface_display(it2);
	up_interface_display(it3);

	up_test("up_interface_same_path", up_interface_same_path(it1, it2) == UP_TRUE);
	up_test("up_interface_same_path", up_interface_same_path(it1, it3) == UP_FALSE);

	up_interface_destroy(it1);
	up_interface_destroy(it2);
	up_interface_destroy(it3);

	up_test_report();

	return 0;
}

#endif
