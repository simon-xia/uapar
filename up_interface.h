#ifndef _UP_INTERFACE_H_
#define _UP_INTERFACE_H_

#include "up_ip4.h"
#include "up_darray.h"

#define INIT_INTERFACE_PATH_CNT	 5

typedef struct interface{
	ip_t			addr;
	unsigned int	path_cnt;
	D_array			*pos_on_path_set; //through position info, we can find processor and successor
}Interface;


Interface * interface_init(ip_t );
void* fetch_interface_key(void *);
void interface_update(void *, void*);
void interface_display(void *);
void interface_destroy(void *);

#endif
