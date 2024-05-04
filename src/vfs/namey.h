#ifndef __NAMEY_H__
#define __NAMEY_H__

#define SP_KERNEL_SRC
#include "common.h"

#include "vfs.h"

#define VFS_PATH_SEP '/'

status_t namey(char *path, inode_t **result);

#endif // #ifndef __NAMEY_H__
