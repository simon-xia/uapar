#include "up_common.h"
#include "up_ip4.h"

ip_t 
get_subnet_addr(ip_t ip, int sub_len)
{
	unsigned int i = (1 << (32 - sub_len)) - 1;
	ip_t ret;
	ret.int_ip = ip.int_ip & (~i);
	return ret;
}

unsigned int
get_common_prefix(ip_t ip1, ip_t ip2)
{
	unsigned int i = ip1.int_ip^ip2.int_ip, j = 1;
	for (; j < 32; j++)
		if ((i & (1 << (32 - j))) != 0)
			break;

	return j - 1;
}

ip_t
get_mask(int sub_len)
{
	assert(sub_len > 0 || sub_len < 32);
	ip_t ret;
	ret.int_ip = ~((1 << (32 - sub_len)) - 1);
	return ret;
}

int
is_insubnet(ip_t ip, ip_t subnet, ip_t mask)
{
	return (ip.int_ip & mask.int_ip) == subnet.int_ip;
}

ip_t
get_broadcast_addr(ip_t subnet, ip_t mask)
{
	ip_t ret;
	ret.int_ip = subnet.int_ip | ~mask.int_ip;
	return ret;
}

void
display_addr(ip_t addr, FILE* fp)
{
	fprintf(fp, "%d.%d.%d.%d ", addr.dot_ip.f1, addr.dot_ip.f2, addr.dot_ip.f3, addr.dot_ip.f4);
}
/*
* Notes:
* 1. a constant char will cause segment fault
* 2. may conflict with outside strtok
*/
/*
unsigned
str_to_int_ip(char *str)
{
	char *field = strtok(str, ".");
	unsigned tmp_ip = 0;
	while (field) {
		tmp_ip = tmp_ip * 256 + atoi(field); 
		field = strtok(NULL, ".");
	}
	return tmp_ip;
}
*/

unsigned
str_to_int_ip(char *str)
{
	char *field_start, *field_end = strchr(str, '.'), field[4]= {0};
	for (field_start = str; !isdigit(*field_start); field_start++) ;
	unsigned tmp_ip = 0;
	while (field_end) {
		memcpy(field, field_start, field_end - field_start);
		tmp_ip = tmp_ip * 256 + atoi(field); 
		memset(field, 0x00, 4);
		field_start = field_end + 1;
		field_end = strchr(field_end + 1, '.');
	}

	// last field = =||
	for (field_end = field_start; isdigit(*field_start) && field_end - str < strlen(str); field_end++) ;
	memcpy(field, field_start, field_end - field_start);
	tmp_ip = tmp_ip * 256 + atoi(field); 

	return tmp_ip;
}

#ifdef UNIT_TEST_IP4

int main()
{
	char *tmp_addr = "192.168.1.1"; 
	ip_t ip;
	ip.int_ip = str_to_int_ip(tmp_addr);
	display_addr(ip, stdout);
	return 0;
}

#endif
