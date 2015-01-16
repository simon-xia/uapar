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

	memcpy(DARRAY_I_ADDR(*array, (*array)->len++), element, (*array)->meta_size);
}

void* up_darray_get_element(D_array *array, unsigned index)
{
	assert(index < array->len);
	return (void*)DARRAY_I_ADDR(array, index);
}

void* up_darray_clear(D_array *array)
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
		memcpy((void*)DARRAY_I_ADDR(array, i-step), (void*)DARRAY_I_ADDR(array, i), array->meta_size);
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

void up_darray_sort(D_array *array)
{
}

#ifdef UNIT_TEST_D_ARRAY

#include "up_interface.h"
#include "up_path.h"

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

	for (i = 0; i < int_arr->len; i++)
		printf("%d ", *(int*)up_darray_get_element(int_arr, i));
	printf("\n");

	up_darray_delete_segment(int_arr, 0,7);
	up_darray_delete_one(int_arr, 1);
	for (i = 0; i < int_arr->len; i++)
		printf("%d ", *(int*)up_darray_get_element(int_arr, i));
	printf("\n");

	for (i = 0; i < char_arr->len; i++)
		printf("%c ", *(char*)up_darray_get_element(char_arr, i));
	printf("\n");

	up_darray_destroy(int_arr);
	up_darray_destroy(char_arr);

	return 0;
}

#endif
