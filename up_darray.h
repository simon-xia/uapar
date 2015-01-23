#ifndef _UP_DARRAY_H_
#define _UP_DARRAY_H_

typedef struct d_array {
	unsigned len;
	unsigned cap;
	unsigned meta_size;
	char *	 array;
}D_array;

typedef struct d_array_iterator {
	D_array *array;
	unsigned cur;
}D_array_iterator;

#define up_darray_ith_addr(a, i) \
	((a)->array + (a)->meta_size*(i))

#define up_darray_ele_cmp_direct(arr, a, b) \
	memcmp(up_darray_ith_addr(arr, a), up_darray_ith_addr(arr, b), (arr)->meta_size)

#define up_darray_ele_swap(arr, a, b) \
	do {\
		if ((a) == (b)) \
			break;\
		void *__addr_a = up_darray_ith_addr(arr, a);\
		void *__addr_b = up_darray_ith_addr(arr, b);\
		up_memxor((__addr_b), (__addr_a), ((arr)->meta_size));	\
		up_memxor((__addr_a), (__addr_b), ((arr)->meta_size));	\
		up_memxor((__addr_b), (__addr_a), ((arr)->meta_size));	\
	}while(0);

D_array* up_darray_init(unsigned , unsigned );
D_array* up_darray_resize(D_array *, unsigned );
void up_darray_push(D_array **, void *);
void* up_darray_find(D_array *, void*);
void* up_darray_clear(D_array *);
void up_darray_destroy(D_array *);
void up_darray_delete_one(D_array *, unsigned);
void up_darray_delete_segment(D_array *, unsigned, unsigned);
D_array* up_darray_dup(D_array *);

D_array_iterator* up_darray_iterator_init(D_array *);
void* up_darray_iterator_next(D_array_iterator *);
void up_darray_iterator_destroy(D_array_iterator *);
void up_darray_iterator_operate(D_array *, void (*)(void*));

#endif
