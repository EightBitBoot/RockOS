
#include "vfs.h"

#include "util/slab_cache.h"

mount_t *g_root_mount;

static slab_cache_t __kfile_cache;

void _vfs_init(void)
{
    uint32_t status = slab_init(&__kfile_cache, sizeof(kfile_t), SC_INIT_LARGE_SLABS);
    (void) status; // Take that compiler!
    // TODO(Adin): Panics for initialization failures
}

void _vfs_deinit(void)
{
    (void) slab_deinit(&__kfile_cache);
}

kfile_t *_vfs_allocate_file(void)
{
    // TODO(Adin): Reference counting
    // TODO(Adin): Null check on allocate
    return slab_alloc(&__kfile_cache, SC_ALLOC_ZERO_MEM);
}