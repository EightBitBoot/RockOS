/**
** @file	namey.c
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	Functions for translating from string paths to fs objects (implementation)
*/

#include "namey.h"

#include "kern/sched.h"
#include "util/kstring.h"

static kstr_t dot_str     = KSTR_CREATE(".", 1);
static kstr_t dot_dot_str = KSTR_CREATE("..", 2);

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
status_t resolve_path(char *path, dirent_t **result)
{
    kstr_t kpath = KSTR_LIT_CREATE(path);

    if(kpath.len == 0) {
        return E_BAD_PARAM;
    }

    dirent_t *curr_dirent = _current->cwd;

    // Yes I know this doesn't account for leading spaces
    // No I don't care atm
    if(kpath.str[0] == '/') {
        curr_dirent = g_root_dirent;
    }

    kstr_t curr_component;
    kstr_strtok_context_t strtok_ctx = {};
    while((curr_component = kstr_strtok(&kpath, VFS_PATH_SEP, &strtok_ctx)).str != NULL) {
        if(!curr_dirent->d_inode->i_ops->lookup) {
            return S_NOT_A_DIR;
        }

        // TODO(Adin): Check for path component > VFS_NAME_MAX

        // Handle relative path components
        if(KSTR_IS_EQUAL(&curr_component, &dot_str)) {
            continue;
        }

        if(KSTR_IS_EQUAL(&curr_component, &dot_dot_str)) {
            // Will always work becuase root's parent is itself
            curr_dirent = curr_dirent->parent;
            continue;
        }

        dirent_t *cached_dirent = _vfs_dirent_lookup_child(curr_dirent, &curr_component);


#if 0
        // Debugging print to be used with test_vfs.c:test_fd_assignment()
        if(cached_dirent) {
            __cio_printf("dirent cache hit: %s\n", (cached_dirent->d_inode ? "positive" : "negative"));
        }
#endif

        // The path component is in the direntry cache: handle if it is
        // positive or negative
        if(cached_dirent && cached_dirent->d_inode) {
            curr_dirent = cached_dirent;
            continue;
        }

        if(cached_dirent && !cached_dirent->d_inode) {
            // Negative entry: fail the search
            *result = NULL;
            return S_NOTFOUND;
        }

        // TODO(Adin): Cross fs boundaries (mount points) here

        // --- Name not in the direntry cache: put it there ---

        dirent_t *new_dirent = _vfs_allocate_dirent(&curr_component);

        // If the component isn't found by lookup, it leaves new_dirent->d_inode as null:
        // representing a negative cache entry
        status_t lookup_ret = curr_dirent->d_inode->i_ops->lookup(curr_dirent->d_inode, new_dirent);
        // Psst. Lookup failed for some reason. Pass it on
        if(lookup_ret != S_OK) {
            *result = NULL;
            return lookup_ret;
        }

        // Insert the new dirent in to the tree hirearchy
        new_dirent->parent = curr_dirent;
        _que_insert(&curr_dirent->children, new_dirent);


        // Lookup returned a negative dirent: fail the search
        if(!new_dirent->d_inode) {
            *result = NULL;
            return S_NOTFOUND;
        }

        curr_dirent = new_dirent;
    }

    *result = curr_dirent;
    return S_OK;
}

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
status_t namey(char *path, inode_t **result)
{
    dirent_t *found_dirent = NULL;
    status_t resolve_result = resolve_path(path, &found_dirent);

    if(resolve_result == S_OK && found_dirent && found_dirent->d_inode) {
        *result = found_dirent->d_inode;
    }

    return resolve_result;
}
