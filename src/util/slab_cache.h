/**
** @file	slab_cache.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Slab-allocated, generalized cache system declerations
*/

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

/**
 * @brief A slab-allocated cache of homogenously sized items
 */
typedef struct slab_cache
{
    uint32_t elem_size;  // The size of each element
    uint32_t flags;      // Options specific to each instance

    void *free_elements; // A linked list of free elements in the cache
    void *all_slabs;     // A linked list of all slabs in the cache
} slab_cache_t;

/**
 * Flag passed to slab_alloc to clear the contents of a new element before returning it
 */
#define SC_ALLOC_ZERO_MEM (0x01U)

/**
 * Flag indicating the cache uses pages as its slabs (as opposed to slices: the default)
 */
#define SC_INIT_LARGE_SLABS (0x01U)

/**
 * @brief Initialize a new slab cache
 *
 * Once initialized, a cache's parameters are fixed and must not be modified.
 *
 * @param cache the cache to initialize
 * @param element_size the size of each element in the cache
 * @param flags any options regarding the initialization or behavior of the cache
 *
 * @return status_t an error code representing the first error that occurred in the operation
 */
status_t slab_init(slab_cache_t *cache, uint32_t element_size, uint32_t flags);
/**
 * @brief Allocate a new element in the cache
 *
 * @param cache the cache to allocate from
 * @param flags any additional options for the operation

 * @return void* the newly allocated element
 */
void *slab_alloc(slab_cache_t *cache, uint32_t flags);
/**
 * @brief Free an element belonging to the cache
 *
 * @param cache the cache that owns/manages the element
 * @param element the element to be freed
 *
 * @return status_t the error status of the operation
 */
status_t slab_free(slab_cache_t *cache, void *element);
/**
 * @brief Deinitialize a cache and free all of its resources
 *
 * After deinit is called on a cache, any references to elements it formerly
 * managed are no longer valid.
 *
 * @param cache the cache to deinit
 *
 * @return status_t the error status of the operation
 */
status_t slab_deinit(slab_cache_t *cache);

//
// ---------------------------------------------TESTS-----------------------------------------------
//

#ifdef __SLAB_CACHE_TEST

uint32_t __slab_test_first_element(void);
void __slab_test_init(void);
void __slab_run_all_tests(void);

#endif // #ifdef __SLAB_CACHE_TEST

#endif // #ifndef __SLAB_CACHE_H__