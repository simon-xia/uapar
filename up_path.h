#ifndef _UP_PATH_H_
#define _UP_PATH_H_

#include "up_darray.h"
#include "up_path.h"
#include "up_interface.h"

#define INIT_PATH_INTERFACE_CNT  16 

typedef struct path {
	int		path_id;
	int		hop_count;
	D_array	*interface_set;
}Path;

typedef struct pos_on_path {
	int		path_id;
	int		pos;
}Pos_on_path;

Path *up_path_init(unsigned);
void up_path_add_interface(Path*, Interface*);
void up_path_display(Path*);

#endif
