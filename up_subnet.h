#ifndef _UP_SUBNET_H_
#define _UP_SUBNET_H_

#include "up_ip4.c"
#include "up_interface.h.c"

typedef struct subnet {
	int net_id;
	int size;
	ip_t	net_addr;
	ip_t	broad_addr;
	ip_t	mask_addr;
	Interface *interface_set;
	int interface_count;
}Subnet;

#endif
