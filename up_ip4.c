#include "up_common.h"
#include "up_ip4.h"

/*
union ip_t{
	struct dot_ip {
		unsigned char f4;
		unsigned char f3;
		unsigned char f2;
		unsigned char f1;	//the first field, because of Little-endian:low bit in low address
	}dot_ip;
	unsigned int int_ip;
};
*/

ip_t 
up_ip_get_subnet(ip_t ip, int sub_len)
{
	unsigned int i = (1 << (32 - sub_len)) - 1;
	ip_t ret;
	ret.int_ip = ip.int_ip & (~i);
	return ret;
}

unsigned 
up_ip_get_common_prefix(ip_t ip1, ip_t ip2)
{
	unsigned int i = ip1.int_ip^ip2.int_ip, j = 1;
	for (; j < 32; j++)
		if ((i & (1 << (32 - j))) != 0)
			break;

	return j - 1;
}

ip_t
up_ip_create_mask(short sub_len)
{
	assert(sub_len > 0 || sub_len < 32);
	ip_t ret;
	ret.int_ip = ~((1 << (32 - sub_len)) - 1);
	return ret;
}

int
up_ip_is_insubnet(ip_t ip, ip_t subnet, ip_t mask)
{
	return (ip.int_ip & mask.int_ip) == subnet.int_ip;
}

ip_t
up_ip_get_broadcast_addr(ip_t subnet, ip_t mask)
{
	ip_t ret;
	ret.int_ip = subnet.int_ip | ~mask.int_ip;
	return ret;
}

void 
up_ip_display(void* addr)
{
	return output_addr(*(ip_t*)addr, stdout);
}
 

void
output_addr(ip_t addr, FILE* fp)
{
	fprintf(fp, "%d.%d.%d.%d ", addr.dot_ip.f1, addr.dot_ip.f2, addr.dot_ip.f3, addr.dot_ip.f4);
}
/*
* Notes:
* 1. a constant char will cause segment fault
* 2. may conflict with outside string lib func strtok
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

// guarantee buf's lenth outside
char *
ip_to_str(ip_t ip, char *buf)
{
	memset(buf, 0x00, MAX_IP_ADDR_SIZE);
	sprintf(buf, "%d.%d.%d.%d", ip.dot_ip.f1, ip.dot_ip.f2, ip.dot_ip.f3, ip.dot_ip.f4);
	return buf;
}

void 
ip_dot_assign(ip_t *ip, unsigned char f1, unsigned char f2, unsigned char f3, unsigned char f4)
{
	ip->dot_ip.f1 = f1;
	ip->dot_ip.f2 = f2;
	ip->dot_ip.f3 = f3;
	ip->dot_ip.f4 = f4;
}

#ifdef UNIT_TEST_IP4
#include "up_test.h"

int main()
{
	ip_t ip, subnet, net_addr, ip3, broadcast, mask;

	ip_dot_assign(&ip, 192, 168, 1, 128);
	ip_dot_assign(&net_addr, 192, 168, 1, 0);
	ip_dot_assign(&ip3, 192, 167, 2, 3);
	ip_dot_assign(&broadcast, 192, 168, 1, 255);
	ip_dot_assign(&mask, 255, 255, 255, 0);

	up_test("up_ip_get_subnet", up_ip_get_subnet(ip, 24).int_ip == net_addr.int_ip);
	up_test("up_ip_get_common_prefix", up_ip_get_common_prefix(ip, net_addr) == 24);
	up_test("up_ip_create_mask", up_ip_create_mask(24).int_ip == mask.int_ip);
	up_test("up_ip_is_insubnet", up_ip_is_insubnet(ip, net_addr, mask) == UP_TRUE);
	up_test("up_ip_is_insubnet", up_ip_is_insubnet(ip3, net_addr, mask) == UP_FALSE);
	up_test("up_ip_get_broadcast_addr", up_ip_get_broadcast_addr(net_addr, mask).int_ip == broadcast.int_ip);
	up_test("str_to_int_ip", ip.int_ip == str_to_int_ip("192.168.1.128"));
	char tmp_ip[MAX_IP_ADDR_SIZE];
	up_test("ip_to_str",strcmp("192.168.2.1", ip_to_str(ip, tmp_ip)));

	/*
	subnet = up_ip_get_subnet(ip, 24);	
	printf("subnet:%d.%d.%d.%d\t%u\n", subnet.dot_ip.f1, subnet.dot_ip.f2, subnet.dot_ip.f3, subnet.dot_ip.f4, subnet.int_ip);
	printf("ip:%d.%d.%d.%d\t%u\n", ip.dot_ip.f1, ip.dot_ip.f2, ip.dot_ip.f3, ip.dot_ip.f4, ip.int_ip);
	printf("addr:%p\t%p.%p.%p.%p\n", &ip.int_ip, &ip.dot_ip.f4, &ip.dot_ip.f3, &ip.dot_ip.f2, &ip.dot_ip.f1);
	printf("common prefix: %d\n", up_ip_get_common_prefix(ip, subnet));
	
	ip_t mask = up_ip_create_mask(24);

	//mark %u
	printf("mask addr:%u\t%d.%d.%d.%d\n", mask.int_ip, mask.dot_ip.f1, mask.dot_ip.f2, mask.dot_ip.f3, mask.dot_ip.f4);
	broadcast = up_ip_get_broadcast_addr(subnet, mask);
	printf("broadcast addr:%u\t%d.%d.%d.%d\n", broadcast.int_ip, broadcast.dot_ip.f1, broadcast.dot_ip.f2, broadcast.dot_ip.f3, broadcast.dot_ip.f4);

	if(up_ip_is_insubnet(ip3, subnet, mask))
		printf("yes\n");
	else
		printf("no\n");
		*/

	up_test_report();
	return 0;
}

#endif
