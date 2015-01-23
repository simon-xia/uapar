#include "up_path.h"
#include "up_interface.h"
#include "up_common.h"

Path *up_path_init(unsigned id)
{
	Path *path = (Path*)malloc(sizeof(Path));
	assert(path != NULL);
	path -> path_id = id;
	path -> hop_count = 0;
	path -> interface_set = up_darray_init(INIT_PATH_INTERFACE_CNT, sizeof(Interface*));
	return path;
}

// add interface obj's addr into path obj
void up_path_add_interface(Path *path, void* it_addr)
{
	up_darray_push(&(path->interface_set), (void*)it_addr);
	path->hop_count++;
}

void up_path_display(void *path)
{
	printf("\033[1;32m path No.%d: \033[0m \ttotal %d hops\tinterface list:\n",((Path*)path)->path_id, ((Path*)path)->hop_count);
	int i = 0;
	for (; i < ((Path*)path)->hop_count; i++)
		up_interface_display(*(Interface**)up_darray_ith_addr(((Path*)path)->interface_set, i));		//mark
}

void up_path_destroy(Path *path)
{
	up_darray_destroy(path->interface_set);
	free(path);
}
