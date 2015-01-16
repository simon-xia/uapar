#include "up_path.h"
#include "up_interface.h"
#include "up_common.h"

Path *up_path_init(unsigned id)
{
	Path *path = (Path*)malloc(sizeof(Path));
	assert(path != NULL);
	path -> path_id = id;
	path -> hop_count = 0;
	path -> interface_set = up_darray_init(INIT_PATH_INTERFACE_CNT, sizeof(Interface));
	return path;
}

void up_path_add_interface(Path *path, Interface* it)
{
	up_darray_push(&(path->interface_set), (void*)it);
	path->hop_count++;
}

void up_path_display(Path *path)
{
	printf("path No.%3d:\ttotal %3d hops\n\tinterface list:\n",path->path_id,  path->hop_count);
	int i = 0;
	for (; i < path->hop_count; i++)
		interface_display(DARRAY_I_ADDR(path->interface_set, i));
}

void up_path_destroy(Path *path)
{
	up_darray_destroy(path->interface_set);
	free(path);
}
