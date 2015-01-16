#ifndef _UP_DARRAY_H_
#define _UP_DARRAY_H_

typedef struct d_array {
	unsigned len;
	unsigned cap;
	unsigned meta_size;
	char *	 array;
}D_array;

#define DARRAY_I_ADDR(a, i) \
	((a)->array + (a)->meta_size*(i))

#define DARRAY_ELE_CMP(arr, a, b) \
	memcmp(DARRAY_I_ADDR(arr, a), DARRAY_I_ADDR(arr, b), (arr)->meta_size)

#define DARRAY_ELE_SWAP(arr, a, b) \
	do {\
	}while(0);

D_array* up_darray_init(unsigned , unsigned );
D_array* up_darray_resize(D_array *, unsigned );
void up_darray_push(D_array **, void *);
void up_darray_destroy(D_array *);
void* up_darray_get_element(D_array *, unsigned );
void* up_darray_clear(D_array *);
void up_darray_delete_one(D_array *, unsigned);
void up_darray_delete_segment(D_array *, unsigned, unsigned);
D_array* up_darray_dup(D_array *);

#endif
