/**
** @file	namey.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Functions for translating from string paths to fs objects
*/

#ifndef __NAMEY_H__
#define __NAMEY_H__

#define SP_KERNEL_SRC
#include "common.h"

#include "vfs.h"

/**
 * @brief The seperator used to delimit path components
 */
#define VFS_PATH_SEP '/'

/**
 * @brief Translate from a string path to a direntry_t
 *
 * On failure (e.g. the file doesn't exist), NULL is returned in result and
 * the appropriate status is returned.
 *
 * @param path the path to resolve
 * @param result the dirent_t pointed to by path or NULL on failure
 *
 * @return status_t the error status of the function
 */
status_t resolve_path(char *path, dirent_t **result);
/**
 * @brief Translate from a string path to an inode_t
 *
 * On failure (e.g. the file doesn't exist), NULL is returned in result and
 * the appropriate status is returned.
 *
 * @param path the path to resolve
 * @param result the inode_t pointed to by path or NULL on failure
 *
 * @return status_t the error status of the function
 */
status_t namey(char *path, inode_t **result);

#endif // #ifndef __NAMEY_H__
