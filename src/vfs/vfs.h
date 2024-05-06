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

struct dirent
{
    kstr_t d_name;
    char d_name_backing[VFS_NAME_MAX];
    inode_t *d_inode;
};

// ----------------------------------------------------------------------------

struct mount
{
    uint32_t mnt_num;
    fs_type_t *mnt_fs_type;
    dirent_t *mnt_don; // Dirent the mount is mounted on
    super_block_t *mnt_sb; // The super block of the mount
};

extern mount_t *g_root_mount; // The mount for the root of the fs
extern queue_t mounts; // The list of currently installed mounts

// ----------------------------------------------------------------------------

// TODO(Adin): Remove / expand me?
#define FS_TYPE_NAME_MAX 50

struct fs_type
{
    char *ft_name;
    dirent_t *(*mount)(fs_type_t *); // TODO(Adin): Add more params here as needed

    void *ft_priv;
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
    uint32_t kf_wrhead; // Read/write head: currently unused

    kfile_ops_t *kf_ops;
    void *kf_priv;
};

struct kfile_ops
{
    // Fill out a file structure from the inode
    status_t (*open)(inode_t *inode, kfile_t *file); // Here because linux defines it here
    // Iterate over the contents of a directory: querying the fs driver instead of the dirent cache
    // Returns the number of buffer entries written
    // TODO(Adin): Change this so it doesn't need the userspace buffer to be passed
    int (*iterate_shared)(kfile_t *file, adinfs_dent_t buffer[], uint32_t buffer_count); // TODO(Adin): Add more params as needed
};

// -----------------------------------------------------------------------------

void _vfs_init(void);
void _vfs_deinit(void);
kfile_t *_vfs_allocate_file(void);

#endif // #ifndef __VFS_H__
