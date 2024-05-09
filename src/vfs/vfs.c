/**
** @file	vfs.c
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Types and public interface to the vfs subsystem (implementation)
*/

#include "vfs.h"

#include "util/slab_cache.h"
#include "mem/kmem.h"

// TODO(Adin): Temp for testing, realistically the initialization
//             should probably be done somewhere else
#include "testfs/testfs.h"

dirent_t *g_root_dirent;

// ---------------------- Object Stores ----------------------

// slice_size / sizeof(fs_type_t *) (1024 / 4 )
/**
 * @brief The maximum number of filesystem types allowed in the system
 */
#define VFS_MAX_NUM_FS_TYPES 256

// TODO(Adin): Make this a queue?
static fs_type_t **__fs_types;
static uint16_t __next_fs_type;

// page_size / sizeof(mount_t *) (4096 / 4)
/**
 * @brief The maximum number of mounts allowed in the system
 */
#define VFS_MAX_NUM_MOUNTS 1024

// TODO(Adin): Make this a queue?

static mount_t **__mounts;
static uint32_t __next_mount;

static slab_cache_t __kfile_cache;
static slab_cache_t __superblock_cache;
static slab_cache_t __dirent_cache;

static slab_cache_t __mount_cache;

/**
 * @brief Allocate a new mount_t
 *
 * @return mount_t* the newly allocated mount_t
 */
static mount_t *__vfs_allocate_mount(void);

/**
 * @brief Initialize the vfs
 */
void _vfs_init(void)
{
    // The slice allocator clears the memory for us
    __fs_types = _km_slice_alloc();
    __next_fs_type = 0;

    __mounts = _km_page_alloc(1);
    __next_mount = 0;

    // Initialize caches used
    // TODO(Adin): Check return statuses of cache initializations
    slab_init(&__kfile_cache, sizeof(kfile_t), SC_INIT_LARGE_SLABS);
    // Use small slabs here because there likely won't be too many in the system
    slab_init(&__superblock_cache, sizeof(super_block_t), 0);
    slab_init(&__dirent_cache, sizeof(dirent_t), SC_INIT_LARGE_SLABS);
    slab_init(&__mount_cache, sizeof(mount_t), SC_INIT_LARGE_SLABS);

    // Initialize and mount the test filesystem
    // TODO(Adin): change this to the real root fs type
    testfs_init();
    _vfs_mount_fs("/", 0);
    g_root_dirent = __mounts[0]->mnt_root;
    g_root_dirent->parent = g_root_dirent;

    // TODO(Adin): Panics for initialization failures
}

/**
 * @brief Unitialize the vfs and free associated resources
 */
void _vfs_deinit(void)
{
    (void) slab_deinit(&__kfile_cache);
    _km_slice_free(__fs_types);
}

/**
 * @brief Register a new filesystem type
 *
 * @param filesystem the filesystem type to register
 *
 * @return status_t the error status of the operation
 */
status_t _vfs_register_fs_type(fs_type_t *filesystem)
{
    if(__next_fs_type == VFS_MAX_NUM_FS_TYPES) {
        return S_NOMEM;
    }

    __fs_types[__next_fs_type] = filesystem;
    __next_fs_type++;

    return S_OK;
}

/**
 * @brief Mount a filesystem at a specific mountpoint
 *
 * @param mountpoint the path to mount the filesystem on
 * @param fs_type_num the idenifier of the registerd filesystem type to mount
 *
 * @return dirent_t* the root dirent_t of the newly mounted filesystem
 */
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

/**
 * @brief Allocate a new kfile_t struct
 *
 * @return kfile_t* the newly allocated kfile_t
 */
kfile_t *_vfs_allocate_file(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__kfile_cache, SC_ALLOC_ZERO_MEM);
}

/**
 * @brief Allocate a new superblock_t struct
 *
 * @return super_block_t* the newly allocated superblock_t
 */
super_block_t *_vfs_allocate_superblock(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__superblock_cache, SC_ALLOC_ZERO_MEM);
}

/**
 * @brief Allocate a new dirent_t struct
 *
 * @param name the name of the new dirent_t
 *
 * @return dirent_t* the newly allocated dirent_t
 */
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

/**
 * @brief Find the dirent named name in the children of parent
 *
 * @param parent the parent dirent whose children are to be searched
 * @param name the name of the target child
 *
 * @return dirent_t* the parent's child named name or NULL if one doesn't exit
 */
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

/**
 * @brief Get the absolute path of a dirent_t
 *
 * @param dirent the dirent to get the path of
 * @param buffer the destination buffer for the path
 */
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

/**
 * @brief Allocate a new mount_t
 *
 * @return mount_t* the newly allocated mount_t
 */
static mount_t *__vfs_allocate_mount(void)
{
    // TODO(Adin): Reference counting
    return slab_alloc(&__mount_cache, SC_ALLOC_ZERO_MEM);
}