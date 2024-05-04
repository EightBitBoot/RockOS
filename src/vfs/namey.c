
#include "namey.h"

#include "kern/sched.h"
#include "util/kstring.h"

status_t namey(char *path, inode_t **result)
{
    kstr_t kpath = KSTR_LIT_CREATE(path);

    if(kpath.len == 0) {
        return E_BAD_PARAM;
    }

    inode_t *curr_inode = _current->cwd->d_inode;

    // Yes I know this doesn't account for leading spaces
    // No I don't care atm
    if(kpath.str[0] == '/') {
        curr_inode = g_root_mount->mnt_sb->sb_root_inode;
    }

    kstr_t curr_component;
    kstr_strtok_context_t strtok_ctx = {};
    dirent_t curr_dent = {};
    while((curr_component = kstr_strtok(&kpath, VFS_PATH_SEP, &strtok_ctx)).str != NULL) {
        if(!curr_inode->i_ops->lookup) {
            return E_NOT_A_DIR;
        }

        // TODO(Adin): Check for path component > VFS_NAME_MAX

        // TODO(Adin): Check the dcache here first, before going to the inode
        // TODO(Adin): Cross fs boundaries (mount points) here

        // TODO(Adin): When there is a real dent cache this needs to copy to the dent's backing
        curr_dent.d_name = curr_component;
        status_t lookup_ret = curr_inode->i_ops->lookup(curr_inode, &curr_dent);

        if(lookup_ret == E_NOT_FOUND) {
            // TODO(Adin): Add negative dirent cache entry
        }

        // Psst. Lookup failed for some reason. Pass it on
        if(lookup_ret != E_SUCCESS) {
            return lookup_ret;
        }

        curr_inode = curr_dent.d_inode;
    }

    *result = curr_inode;
    return E_SUCCESS;
}
