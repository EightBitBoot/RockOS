/**
** @file	slab_cache.c
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Slab-allocated, generalized cache system implementations
*/

#include "slab_cache.h"

#include "common.h"
#include "libc/lib.h"
#include "mem/kmem.h"

/**
 * @brief The address mask to translate from an element to the slice it's contained in
 */
#define SMALL_SLAB_HEADER_MASK (0xFFFFC00U)
/**
 * @brief The address mask to translate from an element to the page it's contained in
 */
#define LARGE_SLAB_HEADER_MASK (0xFFFF000U)

/**
 * @brief Get the header of the slab an element is contained in from a pointer to the element
 */
#define ELEMENT_TO_SLAB_HEADER(elem) ((slab_header_t *) (((uint32_t) (elem)) & \
                                 (cache->flags & SC_INIT_LARGE_SLABS ? LARGE_SLAB_HEADER_MASK : SMALL_SLAB_HEADER_MASK)))

/**
 * @brief Prettifying macro to cast a pointer to a void** linked list item
 */
#define VOID_PTR_TO_LIST_ITEM(ptr) ((void **) (ptr))

/**
 * @brief Get the size of a cache's slabs from its flags
 */
#define SLAB_SIZE(cache) ((cache)->flags & SC_INIT_LARGE_SLABS ? (4096U) : (1024U))


// NOTE(Adin): This is a relatively costly operation so it shouldn't be
//             done laissez faire
// slab_addr + ceil(sizeof(slab_header_t) / elem_size) * elem_size

/**
 * @brief Get the address of the first element in a slab from the address of the slab
 */
#define SLAB_FIRST_ELEM(slab, elem_size) ((slab) + (((sizeof(slab_header_t) + (elem_size) - 1) / (elem_size)) * (elem_size)))

/**
 * @brief Iterate over every element in a slab
 */
#define SLAB_FOR_EACH(cache, slab, var)                            \
    for (                                                          \
        void * var = SLAB_FIRST_ELEM((slab), (cache)->elem_size);  \
        var <= ((slab) + SLAB_SIZE((cache))) - (cache)->elem_size; \
        var += (cache)->elem_size                                  \
    )

/**
 * @brief The header of each slab
 */
typedef struct slab_header
{
    struct slab_header *next_slab; // For the all slabs list
} slab_header_t;

/**
 * @brief Get a new, appropriately sized slab for a cache
 *
 * @param cache the cache to get a slab for
 *
 * @return void* the new slab
 */
static inline void *__get_slab(slab_cache_t *cache)
{
    void *new_slab = NULL;

    if (cache->flags & SC_INIT_LARGE_SLABS) {
        new_slab = _km_page_alloc(1);

        // Initialize the new slab
        // Clear the slab header area (aligned up to the next multiple of elem_size)
        // Note(Adin): The slice allocator clears newly allocated memory regions, but
        //             the page allocator doesn't.
        __memclr(new_slab, SLAB_FIRST_ELEM(new_slab, cache->elem_size) - new_slab);
    }
    else {
        new_slab = _km_slice_alloc();
    }

    return new_slab;
}

/**
 * @brief Free a cache's slab
 *
 * @param cache the owning cache of the slab
 * @param slab the slab to free
 */
static inline void __free_slab(slab_cache_t *cache, void *slab)
{
    if (cache->flags & SC_INIT_LARGE_SLABS) {
        _km_page_free(slab);
    }
    else {
        _km_slice_free(slab);
    }
}

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
status_t slab_init(slab_cache_t *cache, uint32_t element_size, uint32_t flags)
{
    // Slab sizes are both even so a right shift is /= 2
    // 4 bytes are required (per element) to hold the pointer
    // for the free element list
    if (element_size > (SLAB_SIZE(cache) >> 1) || element_size < 4) {
        return E_BAD_PARAM;
    }

    cache->elem_size = element_size;
    cache->flags = flags;
    cache->free_elements = NULL;
    cache->all_slabs = NULL;

    void *first_slab = __get_slab(cache);

    cache->all_slabs = first_slab;
    SLAB_FOR_EACH(cache, first_slab, curr) {
        slab_free(cache, curr);
    }

    return E_SUCCESS;
}

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
status_t slab_deinit(slab_cache_t *cache)
{
    for(void *curr = cache->all_slabs; curr != NULL; curr = *VOID_PTR_TO_LIST_ITEM(curr)) {
        __free_slab(cache, curr);
    }

    return E_SUCCESS;
}

/**
 * @brief Allocate a new element in the cache
 *
 * @param cache the cache to allocate from
 * @param flags any additional options for the operation

 * @return void* the newly allocated element
 */
void *slab_alloc(slab_cache_t *cache, uint32_t flags)
{
    if(!cache->free_elements) {
        // There are no unused elements in the free list: allocate
        // and initialize a new slab

        void *new_slab = __get_slab(cache);

        // Add the slab to the list of all slabs
        ((slab_header_t *) new_slab)->next_slab = cache->all_slabs;
        cache->all_slabs = new_slab;

        SLAB_FOR_EACH(cache, new_slab, curr) {
            slab_free(cache, curr);
        }
    }

    void *new_element = cache->free_elements;
    cache->free_elements = *VOID_PTR_TO_LIST_ITEM(new_element);

    if(flags & SC_ALLOC_ZERO_MEM) {
        __memclr(new_element, cache->elem_size);
    }

    return new_element;
}

/**
 * @brief Free an element belonging to the cache
 *
 * @param cache the cache that owns/manages the element
 * @param element the element to be freed
 *
 * @return status_t the error status of the operation
 */
status_t slab_free(slab_cache_t *cache, void *element)
{
    if(!cache->free_elements) {
        *VOID_PTR_TO_LIST_ITEM(element) = NULL;
        cache->free_elements = element;
    }
    else {
        *VOID_PTR_TO_LIST_ITEM(element) = cache->free_elements;
        cache->free_elements = element;
    }

    return E_SUCCESS;
}

//
// ---------------------------------------------TESTS-----------------------------------------------
//

#ifdef __SLAB_CACHE_TEST

// Some of these functions need to be kept within the source file
// because they have compilation unit isolated preprocessor macros
// and if some are in here, then all are in here

uint32_t __slab_test_first_element(void)
{
    // NOTE(Adin): At time of writing, sizeof(slab_header_t) = 4
    // NOTE(Adin): In the future, element size will be limited to a minimum of 4
    //             (for the free-element list pointer)
    uint32_t smaller =           SLAB_FIRST_ELEM(NULL, 3);  // Expected: 6
    uint32_t smaller_multiple =  SLAB_FIRST_ELEM(NULL, 2);  // Expected: 4
    uint32_t bigger =            SLAB_FIRST_ELEM(NULL, 7);  // Expected: 7
    uint32_t bigger_multiple =   SLAB_FIRST_ELEM(NULL, 8);  // Expected: 8
    uint32_t even =              SLAB_FIRST_ELEM(NULL, sizeof(slab_header_t)); // Expected: 4

    // HAH: Just try to compile me out now!
    return smaller + smaller_multiple + bigger + bigger_multiple + even;
}

void __slab_test_init(void)
{
    // By virtue of being called in init, this also tests
    //    * __get_slab()
    //    * SLAB_FOR_EACH()
    //    * slab_free()

    slab_cache_t small_slabs_cache = {};
    slab_init(&small_slabs_cache, 7, 0);
    uint32_t small_num_elements = 0;
    void *curr_element = small_slabs_cache.free_elements;
    while(curr_element) {
        curr_element = *VOID_PTR_TO_LIST_ITEM(curr_element);
        small_num_elements++;
    }

    slab_cache_t large_slabs_cache = {};
    slab_init(&large_slabs_cache, 7, SC_INIT_LARGE_SLABS);
    uint32_t large_num_elements = 0;
    curr_element = large_slabs_cache.free_elements;
    while(curr_element) {
        curr_element = *VOID_PTR_TO_LIST_ITEM(curr_element);
        large_num_elements++;
    }

    // Useless expression to set a breakpoint on
    // Small Expected: (1024 / 7) - 1 [145]
    // Large Expected: (4096 / 7) - 1 [584]
    uint32_t foo = small_num_elements + large_num_elements;
    (void) foo;
}

void __slab_test_new_slab_alloc(void)
{
    // Through testing this also found issues with
    //    * The first slab not being added to all_slabs in slab_init()
    //    * New slabs headers being cleared _after_ they are added to
    //      all_slabs
    //    * The first slab in a large slabs cache not having its header
    //      area cleared in slab_init()

    slab_cache_t small_slabs_cache = {};
    slab_init(&small_slabs_cache, 7, 0);
    for(int i = 0; i < 146; i++) {
        slab_alloc(&small_slabs_cache, 0);
    }
    uint32_t small_num_slabs = 0;
    slab_header_t *curr_slab = small_slabs_cache.all_slabs;
    while(curr_slab) {
        small_num_slabs++;
        curr_slab = curr_slab->next_slab;
    }

    slab_cache_t large_slabs_cache = {};
    slab_init(&large_slabs_cache, 7, SC_INIT_LARGE_SLABS);
    for(int i = 0; i < 585; i++) {
        slab_alloc(&large_slabs_cache, 0);
    }
    uint32_t large_num_slabs = 0;
    curr_slab = large_slabs_cache.all_slabs;
    while(curr_slab) {
        large_num_slabs++;
        curr_slab = curr_slab->next_slab;
    }

    // Usless expression to set breakpoint on
    // Small Expected: 2
    // Large Expected: 2
    uint32_t bar = small_num_slabs + large_num_slabs;
    (void) bar;
}

void __slab_run_all_tests(void)
{
    __slab_test_first_element();
    __slab_test_init();
    __slab_test_new_slab_alloc();
}

#endif // #ifdef __SLAB_CACHE_TEST