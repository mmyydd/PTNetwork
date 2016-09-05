#ifndef _ARRAY_INCLUDED_H_
#define _ARRAY_INCLUDED_H_

struct pt_array
{
	uint32_t n_reserve;
	uint32_t n_count;

	void **values;
};


void pt_array_init(struct pt_array *arr);
void pt_array_append(struct pt_array *arr, void* value);
void pt_array_remove(struct pt_array *arr, uint32_t index);
void pt_array_remove_quick(struct pt_array *arr, uint32_t index);
void pt_array_clear(struct pt_array *arr);
#endif
