/**
** @file	bogus_data.h
**
** @author	Adin Wistreich-Tannenbaum
**
** @brief	The hardcoded backing data for testfs
*/

#ifndef __BOGUS_DATA_H__
#define __BOGUS_DATA_H__

#include "vfs/vfs.h"

#define BOGUS_MODE_MAX_CHILDREN 4
#define BOGUS_NUM_NODES 10

// Needed for self reference pointers
typedef struct bogus_node bogus_node_t;

/**
 * @brief The basic node of the hard-coded data backing testfs
 */
struct bogus_node
{
    char *name;                                      // The name of the node
    bogus_node_t *parent;                            // The parent's node
    bogus_node_t *children[BOGUS_MODE_MAX_CHILDREN]; // The node's children
    uint8_t num_children;                            // The number of children the node has
    inode_t inode;                                   // The vfs inode representing the node
    void *data;                                      // Bogus page for testing write calls
    uint32_t length;                                 // Length of data in data (will be less than full
                                                     //     allocation most of the time)
};

/**
 * @brief The root node of the testfs backing data
 */
extern bogus_node_t bogus_root_node;
/**
 * @brief All nodes backing testsfs
 */
extern bogus_node_t *bogus_all_nodes[BOGUS_NUM_NODES];

/**
 * @brief initialize the testfs backing data
 */
void __init_bogus_nodes(void);
/**
 * @brief deinitialize the testfs backing data and free any associated resources
 */
void __deinit_bogus_nodes(void);

#endif // #ifndef __BOGUS_DATA_H__