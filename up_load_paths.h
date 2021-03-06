#ifndef _UP_LOAD_PATH_H_
#define _UP_LOAD_PATH_H_

#include "up_darray.h"
#include "up_hash.h"

#define INIT_PATH_SET_SIZE				1024

void drop_loops(D_array *);
int load_paths(char *, D_array **, Hash_table *, unsigned);

#endif
