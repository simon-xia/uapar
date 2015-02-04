#ifndef _UP_INTERFACE_H_
#define _UP_INTERFACE_H_

#include "up_ip4.h"
#include "up_darray.h"

#define INIT_INTERFACE_PATH_CNT	 5
#define INTERFACE_HASH_SLOT_SIZE		1000

struct interface{
	ip_t			addr;
	unsigned		path_cnt;
	D_array			*pos_on_path_set; //through position info, we can find processor and successor
};

typedef struct interface Interface;

#define up_interface_int_ip(it) \
	(((it)->addr).int_ip)


Interface * up_interface_init(ip_t );
void* up_fetch_interface_key(void *);
void up_interface_update(void *, void*);
void up_interface_addpath(Interface *, void *);
void up_interface_display(void *);
void up_interface_destroy(void *);
int up_interface_same_path(Interface *, Interface *);

#endif
