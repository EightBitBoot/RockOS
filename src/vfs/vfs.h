#ifndef __VFS_H__
#define __VFS_H__

#define SP_KERNEL_SRC
#include "common.h"
#include "util/queues.h"
#include "util/kstring.h"

// Forward Type Declarations

typedef struct dirent       dirent_t;
typedef struct mount        mount_t;
typedef struct fs_type      fs_type_t;
typedef struct super_block  super_block_t;

typedef struct inode_ops    inode_ops_t;
typedef struct inode        inode_t;

typedef struct kfile_ops    kfile_ops_t;
typedef struct kfile        kfile_t;

// ----------------------------------------------------------------------------

// TODO(Adin): Move the dirent cache system to its own file (if needed)
struct dirent
{
    kstr_t d_name;
    char d_name_backing[VFS_NAME_MAX];
    inode_t *d_inode;

    // TODO(Adin): Parent and children
};

// TODO(Adin): Make this dynamic (eg. accessed via a function call) when the
//             dirent cache system is made
extern dirent_t *g_root_dirent;

// ----------------------------------------------------------------------------

struct mount
{
    uint32_t mnt_num;

    fs_type_t *mnt_fs_type;
    super_block_t *mnt_sb; // The super block of the mount

    dirent_t *mnt_don; // Dirent the mount is mounted on (NULL for root mount)NULL
    dirent_t *mnt_root; // Root direntry of the mount
};

// NOTE(Adin): The root mount was previously its own variable but for it it will
//             be assumed to be the first mount created (mount #0)

// ----------------------------------------------------------------------------

// TODO(Adin): Remove / expand me?
#define FS_TYPE_NAME_MAX 50

struct fs_type
{
    char *ft_name;
    dirent_t *(*mount)(fs_type_t *); // TODO(Adin): Add more params here as needed

    void *fst_priv;
};

// ----------------------------------------------------------------------------

struct super_block
{
    fs_type_t *sb_fstype; // The filesystem type of the super block
    queue_t sb_inodes; // The actual inodes in the fs instance
    inode_t *sb_root_inode; // The root inode of this fs

    void *sb_priv;
};

// ----------------------------------------------------------------------------

struct inode
{
    inum_t i_num; // Per-filesystem unique inode number
    uint32_t i_mnt_num; // Mount number (makes unique id when combined with i_num)
    uint8_t i_type; // 1 of S_TYPE_FILE, S_TYPE_DIR, S_TYPE_DEV (from common.h)

    uint32_t i_nr_readers;
    bool_t i_has_writer;

    super_block_t *i_super;

    inode_ops_t *i_ops;
    kfile_ops_t *i_file_ops;

    void *i_priv; // Per-inode private data assigned by fs driver
};

struct inode_ops
{
    // Check if the inode has a child with the name in dirent_t
    // if it does, put a pointer to the child's inode into dirent
    // otherwise leave it at / set it to NULL
    status_t (*lookup)(inode_t *inode, dirent_t *dirent);
};

// -----------------------------------------------------------------------------

struct kfile
{
    inode_t *kf_inode;
    uint32_t kf_rwhead;

    uint8_t kf_mode;

    kfile_ops_t *kf_ops;
    void *kf_priv;
};

struct kfile_ops
{
    // Fill out a file structure from the inode
    status_t (*open)(inode_t *inode, kfile_t *file, uint32_t flags); // Here because linux defines it here

    // Optional hook for drivers to perform cleanup code when a file is closed
    // If this isn't present, the file will close normally and be freed
    // This is equivalent to linux's file->release() function
    status_t (*close)(kfile_t *file);

    // Iterate over the contents of a directory: querying the fs driver instead of the dirent cache
    // Returns the number of buffer entries written
    int (*iterate_shared)(kfile_t *file, adinfs_dent_t *buffer, uint32_t buffer_count, uint32_t *num_written);

    // Read data from a file into a userspace-supplied buffer. Because this isn't the fanciest vfs in the world, (and not intended
    // to support the fanciest fs drivers either) read / write heads are handled by the vfs instead of the fs driver itself. As a
    // consequence, the vfs passes the offset in instead of requesting it from the driver. In the case where the user is trying to
    // read off the end of the file, the fs driver should return an error status and set the number of bytes read to 0.
    status_t (*read)(kfile_t *file, void *buffer, uint32_t num_to_read, uint32_t offset, uint32_t flags, uint32_t *num_read);

    // Same as read but in the opposite direction (writing past the end of a file will extend its length)
    status_t (*write)(kfile_t *file, void *buffer, uint32_t num_to_write, uint32_t offset, uint32_t flags, uint32_t *num_written);

    // A generic, optional callback to perform a specific action on a file (or the inode it represents)
    // This is completely fs driver dependent and, unlike posix, there are no default actions; meaning if the
    // driver doesn't support it, the fioctl syscall will return E_NOT_SUPPORTED.
    status_t (*ioctl)(kfile_t *file, uint32_t action, void *data);

    // Get the length of a file. This isn't _strictly_ required, but without it, fseek will fail (which is
    // pretty unusual for a vfs) so you better implement it in your fs drivers. Yeah I'm looking at you. I
    // see you. And I'm disappointed as hell. (This isn't needed or used for device files [they are
    // automatically skipped in the fseek syscall])
    uint32_t (*get_length)(kfile_t *file);
};

// -----------------------------------------------------------------------------

void _vfs_init(void);
void _vfs_deinit(void);

status_t _vfs_register_fs_type(fs_type_t *filesystem);
dirent_t *_vfs_mount_fs(char *mountpoint, uint16_t fs_type_num);

kfile_t *_vfs_allocate_file(void);
super_block_t *_vfs_allocate_superblock(void);
dirent_t *_vfs_allocate_dirent(void);

void _vfs_free_file(kfile_t *file);

#endif // #ifndef __VFS_H__
