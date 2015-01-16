#ifndef _UP_IP4_H_
#define _UP_IP4_H_

typedef union ip_t{
	struct dot_ip {
		unsigned char f4;
		unsigned char f3;
		unsigned char f2;
		unsigned char f1;	//the first field, because of Little-endian:low bit in low address
	}dot_ip;
	unsigned int int_ip;
}ip_t;

ip_t get_mask(int sub_len);

ip_t get_subnet_addr(ip_t ip, int sub_len);

ip_t get_broadcast_addr(ip_t subnet, ip_t mask);

#endif
