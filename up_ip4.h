#ifndef _UP_IP4_H_
#define _UP_IP4_H_

#include "up_common.h"

#define MAX_IP_ADDR_SIZE 16

union ip_t{
	struct dot_ip {
		unsigned char f4;
		unsigned char f3;
		unsigned char f2;
		unsigned char f1;	//the first field, because of Little-endian:low bit in low address
	}dot_ip;
	unsigned int int_ip;
};
typedef union ip_t ip_t;

ip_t up_ip_get_subnet(ip_t, int);
unsigned up_ip_get_common_prefix(ip_t, ip_t);
ip_t up_ip_create_mask(short);
int up_ip_is_insubnet(ip_t, ip_t, ip_t);
ip_t up_ip_get_broadcast_addr(ip_t, ip_t);
void up_ip_display(void*);
void output_addr(ip_t, FILE*);
unsigned str_to_int_ip(char *);
char * ip_to_str(ip_t, char*);
void ip_dot_assign(ip_t *, unsigned char , unsigned char , unsigned char , unsigned char );

#endif
