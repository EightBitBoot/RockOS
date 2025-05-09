/**
** @file	bogus_data.c
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	The hardcoded backing data for testfs (implementation)
*/

#include "bogus_data.h"

#include "mem/kmem.h"

/**
 * I wanted to dynamically allocate these, but nooooooo, we have to go and have
 * a simple slab allocator.
*/

/**
 *
 * root (/)/
 * ├─ etc/
 * │  ├─ passwd
 * │  ├─ group
 * ├─ usr/
 * │  ├─ lib/
 * │  │  ├─ libgdi.so
 * │  ├─ bin/
 * │  │  ├─ chattr
 *
*/

// Nodes
bogus_node_t bogus_root_node;

static bogus_node_t bogus_etc_node;
static bogus_node_t bogus_group_node;
static bogus_node_t bogus_passwd_node;
static bogus_node_t bogus_usr_node;
static bogus_node_t bogus_lib_node;
static bogus_node_t bogus_libgdi_node;
static bogus_node_t bogus_bin_node;
static bogus_node_t bogus_chattr_node;

// The list of all nodes (used for fs initialization)
bogus_node_t *bogus_all_nodes[BOGUS_NUM_NODES] = {
    &bogus_root_node,
    &bogus_etc_node,
    &bogus_passwd_node,
    &bogus_group_node,
    &bogus_passwd_node,
    &bogus_usr_node,
    &bogus_lib_node,
    &bogus_libgdi_node,
    &bogus_bin_node,
    &bogus_chattr_node
};

/**
 * @brief initialize the testfs backing data
 */
void __init_bogus_nodes(void)
{
    bogus_passwd_node.data = _km_page_alloc(1);
    bogus_group_node.data = _km_page_alloc(1);

    bogus_libgdi_node.data = _km_page_alloc(1);

    bogus_chattr_node.data = _km_page_alloc(1);
}

/**
 * @brief deinitialize the testfs backing data and free any associated resources
 */
void __deinit_bogus_nodes(void)
{
    _km_page_free(bogus_passwd_node.data);
    _km_page_free(bogus_group_node.data);

    _km_page_free(bogus_libgdi_node.data);

    _km_page_free(bogus_chattr_node.data);
}

// --------------------------------- Node Data ----------------------------------

bogus_node_t bogus_root_node = {
    .name = "/",
    .parent = &bogus_root_node,
    .children = {&bogus_etc_node, &bogus_usr_node},
    .num_children = 2,
};

static bogus_node_t bogus_etc_node = {
    .name = "etc",
    .parent = &bogus_root_node,
    .children = {&bogus_passwd_node, &bogus_group_node},
    .num_children = 2,
};

static bogus_node_t bogus_passwd_node = {
    .name = "passwd",
    .parent = &bogus_etc_node
};

static bogus_node_t bogus_group_node = {
    .name = "group",
    .parent = &bogus_etc_node
};

static bogus_node_t bogus_usr_node = {
    .name = "usr",
    .parent = &bogus_root_node,
    .children = {&bogus_lib_node, &bogus_bin_node},
    .num_children = 2
};

static bogus_node_t bogus_lib_node = {
    .name = "lib",
    .parent = &bogus_usr_node,
    .children = {&bogus_libgdi_node},
    .num_children = 1
};

static bogus_node_t bogus_libgdi_node = {
    .name = "libgdi.so",
    .parent = &bogus_lib_node
};

static bogus_node_t bogus_bin_node = {
    .name = "bin",
    .parent = &bogus_usr_node,
    .children = {&bogus_chattr_node},
    .num_children = 1
};

static bogus_node_t bogus_chattr_node = {
    .name = "chattr",
    .parent = &bogus_bin_node
};