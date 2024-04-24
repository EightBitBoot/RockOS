#ifndef __SLAB_CACHE_H__
#define __SLAB_CACHE_H__

#define SP_KERNEL_SRC
#include "common.h"

// TODO(Adin): slab_free currently moves any freed element to the unused elements list
// meaning even if a slab has 0 elements in use, its unused elements will still be in
// free_elements, meaning it cannot be freed itself (as this would break the free_elements)
// linked list.
// Instead, each slab can have its own free list and finding a free element
// is available_slabs->free_list->first
typedef struct slab_cache
{
    uint32_t elem_size;
    uint32_t flags;

    void *free_elements;
    void *all_slabs;
} slab_cache_t;

#define SC_ALLOC_ZERO_MEM (0x01U)

#define SC_INIT_LARGE_SLABS (0x01U)

status_t slab_init(slab_cache_t *cache, uint32_t element_size, uint32_t flags);
void *slab_alloc(slab_cache_t *cache, uint32_t flags);
status_t slab_free(slab_cache_t *cache, void *element);

//
// ---------------------------------------------TESTS-----------------------------------------------
//

#ifdef __SLAB_CACHE_TEST

uint32_t __slab_test_first_element(void);
void __slab_test_init(void);
void __slab_run_all_tests(void);

#endif // #ifdef __SLAB_CACHE_TEST

#endif // #ifndef __SLAB_CACHE_H__