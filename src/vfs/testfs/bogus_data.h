#ifndef __BOGUS_DATA_H__
#define __BOGUS_DATA_H__

#include "vfs/vfs.h"

#define BOGUS_MODE_MAX_CHILDREN 4
#define BOGUS_NUM_NODES 10

// Needed for self reference pointers
typedef struct bogus_node bogus_node_t;

struct bogus_node
{
    char *name;
    bogus_node_t *parent;
    bogus_node_t *children[BOGUS_MODE_MAX_CHILDREN];
    uint8_t num_children;
    inode_t inode;
};

extern bogus_node_t bogus_root_node;
extern bogus_node_t *bogus_all_nodes[BOGUS_NUM_NODES];

#endif // #ifndef __BOGUS_DATA_H__