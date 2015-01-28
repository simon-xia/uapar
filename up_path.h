#ifndef _UP_PATH_H_
#define _UP_PATH_H_

#include "up_darray.h"
#include "up_path.h"
#include "up_interface.h"

#define INIT_PATH_INTERFACE_CNT  16 

#define UP_INTERFACE_SUCC	0
#define UP_INTERFACE_ERR	1

typedef struct path {
	int		path_id;
	int		hop_count;
	D_array	*interface_set; // a set of Interface obj's addr
}Path;

typedef struct pos_on_path {
	int		path_id;
	int		pos;
}Pos_on_path;

Path *up_path_init(unsigned);
void up_path_add_interface(Path*, void*);
void up_path_display(void*);
void up_path_destroy(Path *);
int up_path_loop_detect(Path*);

#endif
