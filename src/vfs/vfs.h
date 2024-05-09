/**
** @file	vfs.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Types and public interface to the vfs subsystem
*/

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

/**
 * @brief An entry in the dirent cache
 *
 * This cache is purely in-memory and exists only to speed up path resolution.
 * It is also responsible for tying inodes to their respective name and location
 * in the vfs hirearchy.
 */
struct dirent
{
    kstr_t d_name;                     // Filename
    char d_name_backing[VFS_NAME_MAX]; // String memory for storing filename
    inode_t *d_inode;                  // The inode represented

    dirent_t *parent;                  // The parent in the vfs hirearchy
    queue_t children;                  // The children in the vfs hirearchy
};

// TODO(Adin): Make this dynamic (eg. accessed via a function call) when the
//             dirent cache system is made

/**
 * @brief The dirent corresponding to the root of the vfs ("/")
 */
extern dirent_t *g_root_dirent;

// ----------------------------------------------------------------------------

/**
 * @brief A mount within the system
 */
struct mount
{
    uint32_t mnt_num;       // The mount's unique identifier

    fs_type_t *mnt_fs_type; // The filesystem type mounted
    super_block_t *mnt_sb;  // The super block of the mount

    dirent_t *mnt_don;      // Dirent the mount is mounted on (NULL for root mount)NULL
    dirent_t *mnt_root;     // Root direntry of the mount
};

// NOTE(Adin): The root mount was previously its own variable but for it it will
//             be assumed to be the first mount created (mount #0)

// ----------------------------------------------------------------------------

// TODO(Adin): Remove / expand me?
#define FS_TYPE_NAME_MAX 50

/**
 * @brief A type of filesystem (registerd by drivers)
 */
struct fs_type
{
    char *ft_name;                   // The name of the filesystem type
    dirent_t *(*mount)(fs_type_t *); // The function to mount a filesystem of this type

    void *fst_priv;                  // Private, driver assigned, data associated with this fs type
};

// ----------------------------------------------------------------------------

/**
 * @brief An object representing a loaded filesystem
 *
 * As the same filesystem can be mounted multiple times, these
 * can be shared between mounts.
 */
struct super_block
{
    fs_type_t *sb_fstype;   // The filesystem type of the super block
    queue_t sb_inodes;      // The actual inodes in the fs instance
    inode_t *sb_root_inode; // The root inode of this fs

    void *sb_priv;          // Private, driver assigned, data associated with this fs type
};

// ----------------------------------------------------------------------------

/**
 * @brief An object with the a filesystem (file, directory, or device)
 */
struct inode
{
    inum_t i_num;           // Per-filesystem unique inode number
    uint32_t i_mnt_num;     // Mount number (makes unique id when combined with i_num)
    uint8_t i_type;         // 1 of S_TYPE_FILE, S_TYPE_DIR, S_TYPE_DEV (from common.h)

    // Read/write locking fields
    uint32_t i_nr_readers;  // The number of processes with this inode open for reading
    bool_t i_has_writer;    // Whether this inode currently has a writer

    super_block_t *i_super; // The superblock of the inode

    inode_ops_t *i_ops;      // Driver-implemented operations performed on this inode
    kfile_ops_t *i_file_ops; // Driver-implemented operations performed on files opened from this inode

    void *i_priv;            // Per-inode private data assigned by fs driver
};

/**
 * @brief Operations the vfs can perform on this inode which are implemented by drivers
 */
struct inode_ops
{
    // Check if the inode has a child with the name in dirent_t
    // if it does, put a pointer to the child's inode into dirent
    // otherwise leave it at / set it to NULL
    status_t (*lookup)(inode_t *inode, dirent_t *dirent);
};

// -----------------------------------------------------------------------------

/**
 * @brief The representation of an open file
 */
struct kfile
{
    inode_t *kf_inode;  // The inode the file is representing
    uint32_t kf_rwhead; // The offset into the file's data to perform read and writes at

    uint8_t kf_mode;    // The opened mode of the file (reading, writing, or both)

    kfile_ops_t *kf_ops; // Driver-implemented operations performed on this file
    void *kf_priv;       // Per-kfile private data assigned by fs driver
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
    int (*iterate_shared)(kfile_t *file, adinfs_dent_t *buffer, uint32_t buffer_count, uint32_t *num_written, uint32_t flags);

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

/**
 * @brief Initialize the vfs
 */
void _vfs_init(void);
/**
 * @brief Unitialize the vfs and free associated resources
 */
void _vfs_deinit(void);

/**
 * @brief Register a new filesystem type
 *
 * @param filesystem the filesystem type to register
 *
 * @return status_t the error status of the operation
 */
status_t _vfs_register_fs_type(fs_type_t *filesystem);
/**
 * @brief Mount a filesystem at a specific mountpoint
 *
 * @param mountpoint the path to mount the filesystem on
 * @param fs_type_num the idenifier of the registerd filesystem type to mount
 *
 * @return dirent_t* the root dirent_t of the newly mounted filesystem
 */
dirent_t *_vfs_mount_fs(char *mountpoint, uint16_t fs_type_num);

/**
 * @brief Allocate a new kfile_t struct
 *
 * @return kfile_t* the newly allocated kfile_t
 */
kfile_t *_vfs_allocate_file(void);
/**
 * @brief Allocate a new superblock_t struct
 *
 * @return super_block_t* the newly allocated superblock_t
 */
super_block_t *_vfs_allocate_superblock(void);
/**
 * @brief Allocate a new dirent_t struct
 *
 * @param name the name of the new dirent_t
 *
 * @return dirent_t* the newly allocated dirent_t
 */
dirent_t *_vfs_allocate_dirent(kstr_t *name);

/**
 * @brief Find the dirent named name in the children of parent
 *
 * @param parent the parent dirent whose children are to be searched
 * @param name the name of the target child
 *
 * @return dirent_t* the parent's child named name or NULL if one doesn't exit
 */
dirent_t *_vfs_dirent_lookup_child(dirent_t *parent, kstr_t *name);
/**
 * @brief Get the absolute path of a dirent_t
 *
 * @param dirent the dirent to get the path of
 * @param buffer the destination buffer for the path
 */
void _vfs_dirent_to_pathname(dirent_t *dirent, char *buffer);

/**
 * @brief Free a kfile_t
 *
 * @param file the file to free
 */
void _vfs_free_file(kfile_t *file);

#endif // #ifndef __VFS_H__
