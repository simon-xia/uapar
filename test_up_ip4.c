#include "up_common.h"
#include "up_ip4.h"

int main()
{
	ip_t ip, subnet, ip2, ip3, broadcast;
	ip.dot_ip.f1 = 192;
	ip.dot_ip.f2 = 168;
	ip.dot_ip.f3 = 2;
	ip.dot_ip.f4 = 1;

	ip2.dot_ip.f1 = 192;
	ip2.dot_ip.f2 = 168;
	ip2.dot_ip.f3 = 2;
	ip2.dot_ip.f4 = 3;

	ip3.dot_ip.f1 = 192;
	ip3.dot_ip.f2 = 167;
	ip3.dot_ip.f3 = 2;
	ip3.dot_ip.f4 = 3;

	subnet = get_subnet_addr(ip, 24);	
	printf("subnet:%d.%d.%d.%d\t%u\n", subnet.dot_ip.f1, subnet.dot_ip.f2, subnet.dot_ip.f3, subnet.dot_ip.f4, subnet.int_ip);
	printf("ip:%d.%d.%d.%d\t%u\n", ip.dot_ip.f1, ip.dot_ip.f2, ip.dot_ip.f3, ip.dot_ip.f4, ip.int_ip);
	printf("addr:%p\t%p.%p.%p.%p\n", &ip.int_ip, &ip.dot_ip.f4, &ip.dot_ip.f3, &ip.dot_ip.f2, &ip.dot_ip.f1);
	printf("common prefix: %d\n", get_common_prefix(ip, subnet));
	
	ip_t mask = get_mask(24);

	//mark %u
	printf("mask addr:%u\t%d.%d.%d.%d\n", mask.int_ip, mask.dot_ip.f1, mask.dot_ip.f2, mask.dot_ip.f3, mask.dot_ip.f4);
	broadcast = get_broadcast_addr(subnet, mask);
	printf("broadcast addr:%u\t%d.%d.%d.%d\n", broadcast.int_ip, broadcast.dot_ip.f1, broadcast.dot_ip.f2, broadcast.dot_ip.f3, broadcast.dot_ip.f4);

	if(is_insubnet(ip3, subnet, mask))
		printf("yes\n");
	else
		printf("no\n");

	return 0;
}

