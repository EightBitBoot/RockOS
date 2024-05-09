/**
** @file	testfs.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	A hardcoded fs driver conforming to the vfs interface
*/

#ifndef __TEST_FS_H__
#define __TEST_FS_H__

#define SP_KERNEL_SRC
#include "common.h"

/**
 * @brief Initialize the filesystem driver
 *
 * @return status_t the error status of the operation
 */
status_t testfs_init(void);
/**
 * @brief Deinitialize the filesystem driver and all associated resources
 *
 * @return status_t the error status of the operation
 */
status_t testfs_deinit(void);

#endif // #ifndef __TEST_FS_H__