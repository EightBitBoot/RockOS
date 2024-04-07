/*
** @file	queues.h
**
** @author	CSCI-452 class of 20235
**
** @brief	Queue module declarations
*/

#ifndef QUEUES_H_
#define QUEUES_H_

#include "common.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Pseudo-functions
*/

#define	QUE_LENGTH(q)	((q)->length)
#define	QUE_IS_FIFO(q)	((q)->compare == NULL)
#define	QUE_IS_EMPTY(q)	((q)->length == 0)

/*
** Types
*/

/*
** Our queue is a queue of "qnodes", which allows us to enque data of
** virtually any type.
**
** The list of qnodes is doubly-linked, allowing traversal in either
** direction.
*/

typedef struct qnode_s {
	struct qnode_s *prev;
	struct qnode_s *next;
	void *data;
} qnode_t;

/*
** The queue itself has pointers to the head and tail elements,
** the current length of the queue, and a pointer to a comparison
** function that allows the queue to be self-ordering. If not comparison
** function is specified when the queue is created, the queue behaves
** as a FIFO queue; otherwise, the comparison function is used to 
** determine the correct insertion point for new data.
*/

typedef struct queue_s {
	qnode_t *head;
	qnode_t *tail;
	int (*compare)(void*,void*);
	uint32_t length;
} queue_t;

/*
** Function pointer types (defined here for convenience)
*/

// ordering functions
typedef int (*compare_t)(void*,void*);

/*
** Globals
*/

/*
** Prototypes
*/

/**
** Name:  _que_init
**
** Initialize the queue module.
*/
void _que_init( void );

/**
** Name:  _que_create
**
** Create (reininitalize) a queue_t structure.
**
** @param q        The queue to be reinitialized
** @param compare  The ordering function for this queue, or NULL
*/
void _que_create( queue_t *q, compare_t compare );

/**
** Name:  _que_peek
**
** Peek at the first entry in a queue
**
** @param q     The queue to be examined
** @param data  (output) Where to save the value from the first queue node
**
** @return S_OK if there was an entry, S_EMPTY otherwise
*/
status_t _que_peek( queue_t *q, void **data );

/**
** Name:  _que_insert
**
** Add an entry to a queue.
**
** @param q     The queue to be manipulated
** @param data  The value to add to the queue
**
** @return The insertion status
*/
status_t _que_insert( queue_t *q, void *data );

/**
** Name:  _que_remove
**
** Remove the first entry from a queue.
**
** @param q     The queue to be manipulated
** @param data  (output) Where to save the removed data value
**
** @return The removal status
*/
status_t _que_remove( queue_t *q, void **data );

/**
** Name:  _que_remove_ptr
**
** Remove a specific entry from a queue.
**
** @param q     The queue to be manipulated
** @param data  The entry to be located and removed
**
** @return The removal status
*/
status_t _que_remove_ptr( queue_t *q, void *data );

#endif
// !SP_ASM_SRC

#endif
