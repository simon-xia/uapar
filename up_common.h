#ifndef _UP_COMMON_H_
#define _UP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "up_log.h"

#define UP_FLOAT_ERROR		(1e-5)

#define UP_TRUE				1
#define UP_FALSE			0

#define UP_ERR				1
#define UP_SUCC				0

#define up_float_equal(f1, f2) \
	(fabs((f1) - (f2)) < UP_FLOAT_ERROR ? 1 : 0)

#define up_memxor(des, src, l) \
	do {\
		int __i; \
		char *__a = (char*)(des), *__b = (char*)(src);\
		for (__i = 0; __i < (l); __i++) {\
			*__a++ ^= *__b++; \
		}\
	}while(0)

#endif
