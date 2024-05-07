
#include "testfs.h"

#include "common.h"
#include "io/cio.h"
#include "kern/kdefs.h"
#include "vfs/vfs.h"
#include "libc/lib.h"

#include "bogus_data.h"

#include "usr/testfs_usr.h"

status_t testfs_open(inode_t *inode, kfile_t *file);
status_t testfs_close_file(kfile_t *file);
status_t testfs_ioctl(kfile_t *file, uint32_t action, void *data);

int testfs_iterate_shared(kfile_t *file, adinfs_dent_t buffer[], uint32_t buffer_count);
status_t testfs_lookup(inode_t *inode, dirent_t *dirent);

dirent_t *testfs_mount(fs_type_t *fs_type);

#define FILE_TO_BOGUS_NODE(file_ptr) ((bogus_node_t *)(file_ptr)->kf_priv)

// -------------------------------------------------------
// Quick! It's the OPs!

static inode_ops_t testfs_inode_file_ops = {};
static inode_ops_t testfs_inode_dir_ops = {
    .lookup = testfs_lookup,
};

static kfile_ops_t testfs_file_ops = {
    .open = testfs_open,
    .close = testfs_close_file,
    .ioctl = testfs_ioctl,
};
static kfile_ops_t testfs_dir_ops = {
    .open = testfs_open,
    .iterate_shared = testfs_iterate_shared,
};

static fs_type_t testfs_fs_type = {
    .ft_name = "testfs",
    .mount = testfs_mount
};

status_t testfs_open(inode_t *inode, kfile_t *file)
{
    file->kf_priv = inode->i_priv;

    return E_SUCCESS;
}

int testfs_iterate_shared(kfile_t *file, adinfs_dent_t buffer[], uint32_t buffer_count)
{
    bogus_node_t *bogus_parent = file->kf_priv;
    if(!bogus_parent) {
        return E_NO_DATA;
    }

    int num_written = 0;
    for(int i = 0; i < buffer_count && i < bogus_parent->num_children; i++) {
        __memcpy(buffer[i].name, bogus_parent->children[i]->name, __strlen(bogus_parent->children[i]->name));
        num_written++;
    }

    return num_written;
}

status_t testfs_lookup(inode_t *inode, dirent_t *dirent)
{
    bogus_node_t *bogus_node = inode->i_priv;
    if(!bogus_node) {
        return E_NO_DATA;
    }

    if(bogus_node->num_children == 0) {
        return E_NO_CHILDREN;
    }

    for(int i = 0; i < bogus_node->num_children; i++) {
        bogus_node_t *child = bogus_node->children[i];
        kstr_t child_name = KSTR_LIT_CREATE(child->name);

        if(KSTR_IS_EQUAL(&child_name, &dirent->d_name)) {
            dirent->d_inode = &child->inode;
            return E_SUCCESS;
        }
    }

    return E_NOT_FOUND;
}

status_t testfs_close_file(kfile_t *file)
{
    __cio_printf("Closing testfs file (in driver) %s\n", ((bogus_node_t *)file->kf_priv)->name);

    return S_OK;
}

status_t testfs_ioctl(kfile_t *file, uint32_t action, void *data)
{
    if(action != TESTFS_SAY_HI) {
        return S_BAD_ACTION;
    }

    if(!data) {
        return S_BAD_PARAM;
    }

    __cio_printf(
        "Test fs says hi from file %s. The caller of fioctl would also you to know %s\n",
        FILE_TO_BOGUS_NODE(file)->name, (char *)data
    );

    return S_OK;
}

static super_block_t *testfs_super_block = NULL;

dirent_t *testfs_mount(fs_type_t *fs_type)
{
    // There will only ever be one instance of this filesystem
    // regardless of how many times it is mounted
    if(!testfs_super_block) {
        testfs_super_block = _vfs_allocate_superblock();
    }
    // Init the super block
    testfs_super_block->sb_fstype = &testfs_fs_type;
    _que_create(&(testfs_super_block->sb_inodes), NULL);
    testfs_super_block->sb_root_inode = &bogus_root_node.inode;

    // Init the inodes in the bogus nodes
    for(int i = 0; i < BOGUS_NUM_NODES; i++) {
        bogus_node_t *curr_node = bogus_all_nodes[i];
        inode_t *curr_inode = &curr_node->inode;

        curr_inode->i_num = i;
        curr_inode->i_super = testfs_super_block;

        if(curr_node->num_children > 0) {
            curr_inode->i_ops = &testfs_inode_dir_ops;
            curr_inode->i_file_ops = &testfs_dir_ops;
            curr_inode->i_type = S_TYPE_DIR;
        }
        else {
            curr_inode->i_ops = &testfs_inode_file_ops;
            curr_inode->i_file_ops = &testfs_file_ops;
            curr_inode->i_type = S_TYPE_FILE;
        }

        curr_inode->i_priv = curr_node;

        _que_insert(&testfs_super_block->sb_inodes, curr_inode);
    }

    dirent_t *testfs_root_dirent = _vfs_allocate_dirent();

    // Init the root dirent
    testfs_root_dirent->d_inode = &bogus_root_node.inode;
    testfs_root_dirent->d_name.str = bogus_root_node.name;
    testfs_root_dirent->d_name.len = __strlen(bogus_root_node.name);

    return testfs_root_dirent;
}

// -------------------------------------------------------
// Create me

status_t testfs_init(void)
{
    _vfs_register_fs_type(&testfs_fs_type);
    return E_SUCCESS;
}

status_t testfs_deinit(void)
{
    // TODO(Adin): Unregister fs type here
    return E_SUCCESS;
}