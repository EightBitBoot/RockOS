
#include "testfs.h"

#include "common.h"
#include "io/cio.h"
#include "kern/kdefs.h"
#include "vfs/vfs.h"
#include "libc/lib.h"

#include "bogus_data.h"

#include "usr/testfs_usr.h"

status_t testfs_open(inode_t *inode, kfile_t *file, uint32_t flags);
status_t testfs_close_file(kfile_t *file);
status_t testfs_file_read(kfile_t *file, void *buffer, uint32_t num_to_read, uint32_t offset, uint32_t flags, uint32_t *num_read);
status_t testfs_file_write(kfile_t *file, void *buffer, uint32_t num_bytes, uint32_t offset, uint32_t flags, uint32_t *num_written);
status_t testfs_ioctl(kfile_t *file, uint32_t action, void *data);
uint32_t testfs_get_length(kfile_t *file);

status_t testfs_iterate_shared(kfile_t *file, adinfs_dent_t* buffer, uint32_t buffer_count, uint32_t *num_written);
status_t testfs_lookup(inode_t *inode, dirent_t *dirent);

dirent_t *testfs_mount(fs_type_t *fs_type);

#define INT_MIN(a, b) ((a) < (b) ? (a) : (b))

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
    .read = testfs_file_read,
    .write = testfs_file_write,
    .ioctl = testfs_ioctl,
    .get_length = testfs_get_length,
};
static kfile_ops_t testfs_dir_ops = {
    .open = testfs_open,
    .iterate_shared = testfs_iterate_shared,
};

static fs_type_t testfs_fs_type = {
    .ft_name = "testfs",
    .mount = testfs_mount
};

status_t testfs_open(inode_t *inode, kfile_t *file, uint32_t flags)
{
    (void) flags;

    file->kf_priv = inode->i_priv;

    return S_OK;
}

status_t testfs_iterate_shared(kfile_t *file, adinfs_dent_t* buffer, uint32_t buffer_count, uint32_t *num_written)
{
    bogus_node_t *node = FILE_TO_BOGUS_NODE(file);

    if(!num_written) {
        return S_BAD_PARAM;
    }

    if(!buffer) {
        *num_written = node->num_children;
        return S_OK;
    }

    if(buffer_count < node->num_children) {
        return S_TOO_SMALL;
    }

    for(uint32_t i = 0; i < node->num_children; i++) {
        adinfs_dent_t *dent = buffer + i;
        CLEAR_PTR(dent);

        __memcpy(dent->name, node->children[i]->name, __strlen(node->children[i]->name));
        dent->type = node->inode.i_type;
    }

    *num_written = node->num_children;
    return S_OK;
}

status_t testfs_lookup(inode_t *inode, dirent_t *dirent)
{
    bogus_node_t *bogus_node = inode->i_priv;
    if(!bogus_node) {
        return S_NO_DATA;
    }

    if(bogus_node->num_children == 0) {
        return S_NO_CHILDREN;
    }

    for(int i = 0; i < bogus_node->num_children; i++) {
        bogus_node_t *child = bogus_node->children[i];
        kstr_t child_name = KSTR_LIT_CREATE(child->name);

        if(KSTR_IS_EQUAL(&child_name, &dirent->d_name)) {
            dirent->d_inode = &child->inode;
            break;
        }
    }

    return S_OK;
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

status_t testfs_file_read(kfile_t *file, void *buffer, uint32_t num_to_read, uint32_t offset, uint32_t flags, uint32_t *num_read)
{
    if(!num_read) {
        return E_BAD_PARAM;
    }

    if(!num_read) {
        *num_read = 0;
        return E_BAD_PARAM;
    }

    bogus_node_t *node = FILE_TO_BOGUS_NODE(file);
    uint32_t data_len = testfs_get_length(file);

    if(offset > data_len) {
        *num_read = 0;
        return S_EOF;
    }

    status_t result = S_OK;

    // (data_len - offset) will never underflow because of the (offset > data_len)
    // check above
    uint32_t num_actually_read = INT_MIN(data_len - offset, num_to_read);
    __memcpy(buffer, node->data + offset, num_actually_read);

    if(num_actually_read < num_to_read || data_len - offset == num_to_read) {
        result = S_EOF;
    }

    *num_read = num_actually_read;
    return result;
}

status_t testfs_file_write(kfile_t *file, void *buffer, uint32_t num_bytes, uint32_t offset, uint32_t flags, uint32_t *num_written)
{
    if(!num_written) {
        return S_BAD_PARAM;
    }

    if(!file) {
        *num_written = 0;
        return S_BAD_PARAM;
    }

    bogus_node_t *node = FILE_TO_BOGUS_NODE(file);
    if(!node->data) {
        // Not returned anywhere else so this will uniquely identify the issue
        return S_ERR;
    }

    uint32_t file_len_before = testfs_get_length(file);

    // TODO(Adin): Un-hardcode 4096
    if(file_len_before == 4096) {
        return S_NOMEM;
    }

    // TODO(Adin): Un-hardcode this
    // 4096 byte page backings for files
    // (4096 - offset) will never underflow. 2 cases:
    //     a) offset is < 4096
    //     b) offset == 4096 (read and fseek clamp the max kf_rwhead to file_length
    //        and this line itself prevents writing off the end of the file data
    //        backing)
    uint32_t num_to_write = INT_MIN(num_bytes, 4096 - offset);
    __memcpy(node->data + offset, buffer, num_to_write);

    if(offset + num_to_write > file_len_before) {
        node->length += (offset + num_to_write) - file_len_before;
    }

    *num_written = num_to_write;
    return S_OK;
}

uint32_t testfs_get_length(kfile_t *file)
{
    // TODO(Adin): Adjust this for ram-backed, writable files
    return FILE_TO_BOGUS_NODE(file)->length;
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

    kstr_t root_name = KSTR_LIT_CREATE(bogus_root_node.name);
    dirent_t *testfs_root_dirent = _vfs_allocate_dirent(&root_name);

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
    __init_bogus_nodes();

    _vfs_register_fs_type(&testfs_fs_type);
    return E_SUCCESS;
}

status_t testfs_deinit(void)
{
    __deinit_bogus_nodes();

    // TODO(Adin): Unregister fs type here
    return E_SUCCESS;
}