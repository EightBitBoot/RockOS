
#include "vfs.h"

#include "util/slab_cache.h"
#include "mem/kmem.h"

// TODO(Adin): Temp for testing, realistically the initialization
//             should probably be done somewhere else
#include "testfs/testfs.h"

dirent_t *g_root_dirent;

// ---------------------- File Private ----------------------

// slice_size / sizeof(fs_type_t *) (1024 / 4 )
#define VFS_MAX_NUM_FS_TYPES 256

// TODO(Adin): Make this a queue?
static fs_type_t **__fs_types;
static uint16_t __next_fs_type;

// page_size / sizeof(mount_t *) (4096 / 4)
#define VFS_MAX_NUM_MOUNTS 1024

// TODO(Adin): Make this a queue?
static mount_t **__mounts;
static uint32_t __next_mount;

static slab_cache_t __kfile_cache;
static slab_cache_t __superblock_cache;
static slab_cache_t __dirent_cache;

static slab_cache_t __mount_cache;
static mount_t *__vfs_allocate_mount(void);

void _vfs_init(void)
{
    // The slice allocator clears the memory for us
    __fs_types = _km_slice_alloc();
    __next_fs_type = 0;

    __mounts = _km_page_alloc(1);
    __next_mount = 0;

    // TODO(Adin): Check return statuses of cache initializations
    slab_init(&__kfile_cache, sizeof(kfile_t), SC_INIT_LARGE_SLABS);
    // Use small slabs here because there likely won't be too many in the system
    slab_init(&__superblock_cache, sizeof(super_block_t), 0);
    slab_init(&__dirent_cache, sizeof(dirent_t), SC_INIT_LARGE_SLABS);
    slab_init(&__mount_cache, sizeof(mount_t), SC_INIT_LARGE_SLABS);

    // TODO(Adin): change this to the real root fs type
    testfs_init();
    _vfs_mount_fs("/", 0);
    g_root_dirent = __mounts[0]->mnt_root;
    g_root_dirent->parent = g_root_dirent;

    // TODO(Adin): Panics for initialization failures
}

void _vfs_deinit(void)
{
    (void) slab_deinit(&__kfile_cache);
    _km_slice_free(__fs_types);
}

status_t _vfs_register_fs_type(fs_type_t *filesystem)
{
    if(__next_fs_type == VFS_MAX_NUM_FS_TYPES) {
        return S_NOMEM;
    }

    __fs_types[__next_fs_type] = filesystem;
    __next_fs_type++;

    return S_OK;
}

dirent_t *_vfs_mount_fs(char *mountpoint, uint16_t fs_type_num)
{
    if(
        fs_type_num > VFS_MAX_NUM_FS_TYPES ||
        __fs_types[fs_type_num] == NULL    ||
        __next_mount == VFS_MAX_NUM_MOUNTS
    )
    {
        return NULL;
    }

    fs_type_t *fs_type = __fs_types[fs_type_num];
    dirent_t *root_dirent = fs_type->mount(fs_type);

    mount_t *new_mount = __vfs_allocate_mount();
    new_mount->mnt_num = __next_mount;
    new_mount->mnt_fs_type = fs_type;
    new_mount->mnt_sb = root_dirent->d_inode->i_super;
    new_mount->mnt_root = root_dirent;

    __mounts[__next_mount] = new_mount;
    __next_mount++;

    // TODO(Adin): Assign root_dirent's parent

    return root_dirent;
}

kfile_t *_vfs_allocate_file(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__kfile_cache, SC_ALLOC_ZERO_MEM);
}

super_block_t *_vfs_allocate_superblock(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__superblock_cache, SC_ALLOC_ZERO_MEM);
}

dirent_t *_vfs_allocate_dirent(kstr_t *name)
{
    if(name->len > VFS_NAME_MAX) {
        return NULL;
    }

    dirent_t *dirent = slab_alloc(&__dirent_cache, SC_ALLOC_ZERO_MEM);
    _que_create(&dirent->children, NULL);
    __memcpy(dirent->d_name_backing, name->str, name->len);
    dirent->d_name.str = dirent->d_name_backing;
    dirent->d_name.len = name->len;

    return dirent;
}

void _vfs_free_file(kfile_t *file)
{
    slab_free(&__kfile_cache, file);
}

dirent_t *_vfs_dirent_lookup_child(dirent_t *parent, kstr_t *name)
{
    qnode_t *curr_node = parent->children.head;
    while(curr_node) {
        dirent_t *curr_child = curr_node->data;
        if(kstr_strcmp(&curr_child->d_name, name) == 0) {
            return curr_child;
        }

        curr_node = curr_node->next;
    }

    return NULL;
}

void _vfs_dirent_to_pathname(dirent_t *dirent, char *buffer)
{
    if(dirent != g_root_dirent) {
        _vfs_dirent_to_pathname(dirent->parent, buffer);
    }

    if(dirent->parent != g_root_dirent) {
        __strcat(buffer, "/");
    }

    // Yes I know this would fail if the filename == VFS_NAME_MAX
    __strcat(buffer, dirent->d_name_backing);
}

// --------------------------------- File Private ---------------------------------

static mount_t *__vfs_allocate_mount(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__mount_cache, SC_ALLOC_ZERO_MEM);
}