#include "up_darray.h"
#include "up_common.h"

D_array* up_darray_init(unsigned cap, unsigned meta_size)
{
	if (!cap || !meta_size)
		return NULL;

	D_array *array = (D_array*)malloc(sizeof(D_array));
	assert(array != NULL);
	array -> cap = cap;
	array -> len = 0;
	array -> meta_size = meta_size;
	array -> array = calloc(cap, meta_size);
	assert(array -> array);

	return array;
}

D_array* up_darray_resize(D_array *array, unsigned new_cap)
{
	D_array *new = up_darray_init(new_cap, array->meta_size);
	new->len = array->len;
	memcpy(new->array, array->array, array->meta_size*array->len);
	up_darray_destroy(array);
	return new;
}

// element's type don't match array's type is a runtime error
void up_darray_push(D_array **array, void *element)
{
	assert(*array != NULL && element != NULL);
	if ((*array)->len == (*array)->cap) {
		*array = up_darray_resize(*array, (*array)->cap * 2);
	}

	memcpy(up_darray_ith_addr(*array, (*array)->len++), element, (*array)->meta_size);
}

void* up_darray_find(D_array *array, void *element)
{
	int i;
	if ((i = up_darray_find_index(array, element)) < 0)
		return NULL;
	else
		return up_darray_ith_addr(array, i);
}

int up_darray_find_index(D_array *array, void *element)
{
	if (!array || !element) {
		WARNING("invaild arguments\n");
		return -1;
	}
	int i = 0;
	for (; i < array->len; i++)
		if (!memcmp(up_darray_ith_addr(array, i), element, array->meta_size))
			return i;

	return -1;
}

void up_darray_clear(D_array *array)
{
	memset(array->array, 0x00, array->meta_size * array -> cap);
	array->len = 0;
}

void up_darray_destroy(D_array *array)
{
	free(array->array);
	free(array);
}

void up_darray_delete_one(D_array *array, unsigned idx)
{
	return up_darray_delete_segment(array, idx, idx);
}

// delete [start_idx, end_idx]
void up_darray_delete_segment(D_array *array, unsigned start_idx, unsigned end_idx)
{
	assert(start_idx < array->len && end_idx < array->len);
	int i, step = end_idx - start_idx + 1;
	for (i = end_idx + 1; i < array->len; i++)
	{
		memcpy((void*)up_darray_ith_addr(array, i-step), (void*)up_darray_ith_addr(array, i), array->meta_size);
	}
	array->len -= step;
}

D_array* up_darray_dup(D_array *array)
{
	D_array *new = up_darray_init(array->cap, array->meta_size);
	memcpy(new->array, array->array, array->meta_size*array->len);
	new->len = array->len;
	return new;
}

void up_darray_display(D_array *array, void (*display_func)(void*))
{
	printf("\n\tarray len: %u element size: %u\n", array->len, array->meta_size);
	up_darray_iterator_operate(array, display_func);
	printf("\n");
}

D_array_iterator* up_darray_iterator_init(D_array *array)
{
	D_array_iterator *iter = (D_array_iterator*)malloc(sizeof(D_array_iterator));
	if (!iter) {
		ERROR("Not enough memory\n");
		return NULL;
	}
	iter -> array = array;
	iter -> cur = 0;
	return iter;
}

void* up_darray_iterator_next(D_array_iterator *iter)
{
	if (iter->array->len == iter->cur)
		return NULL;
	return up_darray_ith_addr(iter->array, iter->cur++);
}

void up_darray_iterator_destroy(D_array_iterator *iter)
{
	free(iter);
}

void up_darray_iterator_operate(D_array *array, void (*operate_func)(void*))
{
	D_array_iterator *iter = up_darray_iterator_init(array);
	void *tmp_ele;
	while (tmp_ele = up_darray_iterator_next(iter)) {
		operate_func(tmp_ele);
	}
	up_darray_iterator_destroy(iter);
}

void up_darray_sort(D_array *array, int (*cmp_func)(void *, void *))
{
	if (!array)
		return ;

	int i, j;
	for (i = 0; i < array->len; i++)
		for (j = i+1; j < array->len; j++)
		{
			if (cmp_func(up_darray_ith_addr(array, i), up_darray_ith_addr(array, j)) == -1)
				up_darray_ele_swap(array, i, j);
		}
}

#ifdef UNIT_TEST_D_ARRAY

#include "up_interface.h"
#include "up_path.h"
#include "up_ip4.h"

void int_display(void *ele)
{
	printf("%d ", *(int*)ele);
}

void char_display(void *ele)
{
	printf("%c ", *(char*)ele);
}

int main()
{
	D_array *int_arr = up_darray_init(1, sizeof(int));
	D_array *char_arr = up_darray_init(1, sizeof(char));

	int i = 34;

	for (; i < 126; i++)
	{
		up_darray_push(&int_arr, (void*)&i);
		up_darray_push(&char_arr, (void*)&i);
	}
	
	up_darray_display(int_arr, int_display);
	up_darray_display(char_arr, char_display);

	up_darray_delete_segment(int_arr, 0,7);
	up_darray_delete_one(int_arr, 1);

	up_darray_display(int_arr, int_display);

	up_darray_destroy(int_arr);
	up_darray_destroy(char_arr);

	D_array *ip_arr = up_darray_init(1, sizeof(ip_t));

	ip_t tmp_ip;
	tmp_ip.dot_ip.f1 = 192;
	tmp_ip.dot_ip.f2 = 168;
	tmp_ip.dot_ip.f3 = 1;

	for (i = 5; i < 100; i++)
	{
		tmp_ip.dot_ip.f4 = i;
		up_darray_push(&ip_arr, (void*)&tmp_ip);
	}

	up_darray_display(ip_arr, up_ip_display);

	up_darray_delete_segment(ip_arr, 0,7);
	up_darray_delete_one(ip_arr, 1);

	up_darray_display(ip_arr, up_ip_display);

	up_darray_destroy(ip_arr);
	return 0;
}

#endif
