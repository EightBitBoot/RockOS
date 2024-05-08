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
    void *data; // Bogus page for testing write calls
    uint32_t length; // Length of data in data (will be less than full allocation most of the time)
};

extern bogus_node_t bogus_root_node;
extern bogus_node_t *bogus_all_nodes[BOGUS_NUM_NODES];

void __init_bogus_nodes(void);
void __deinit_bogus_nodes(void);

#endif // #ifndef __BOGUS_DATA_H__