
#include "namey.h"

#include "kern/sched.h"
#include "util/kstring.h"

static kstr_t dot_str     = KSTR_CREATE(".", 1);
static kstr_t dot_dot_str = KSTR_CREATE("..", 2);

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

        status_t lookup_ret = curr_dirent->d_inode->i_ops->lookup(curr_dirent->d_inode, new_dirent);
        // Psst. Lookup failed for some reason. Pass it on
        if(lookup_ret != S_OK) {
            *result = NULL;
            return lookup_ret;
        }

        new_dirent->parent = curr_dirent;
        _que_insert(&curr_dirent->children, new_dirent);


        if(!new_dirent->d_inode) {
            *result = NULL;
            return S_NOTFOUND;
        }

        curr_dirent = new_dirent;
    }

    *result = curr_dirent;
    return S_OK;
}

status_t namey(char *path, inode_t **result)
{
    dirent_t *found_dirent = NULL;
    status_t resolve_result = resolve_path(path, &found_dirent);

    if(resolve_result == S_OK && found_dirent && found_dirent->d_inode) {
        *result = found_dirent->d_inode;
    }

    return resolve_result;
}
