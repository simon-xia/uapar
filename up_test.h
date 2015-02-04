#ifndef _UP_TEST_H_
#define _UP_TEST_H_

int __test_idex = 0;
int __failed_cnt = 0;

#define up_test(name, c) \
	do {\
		if ((c)) \
			fprintf(stderr, "No.%d\ttest \033[1;32mpassed\033[0m: "name"\n", ++__test_idex);\
		else {\
			fprintf(stderr, "No.%d\ttest \033[1;31mfailed\033[0m: "name"\n", ++__test_idex); \
			__failed_cnt++; \
		} \
	}while(0)

#define up_test_report() \
	do { \
		fprintf(stderr, "\n\t\t***** %s test report *****\n\t\ttotal test %2d:\t%2d pass\t%2d fail\n\n", __FILE__, __test_idex, __test_idex - __failed_cnt, __failed_cnt); \
	}while(0)

#endif
