/*
** @file	queues.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Queue module implementation
*/

/*
** Queue organization
** ------------------
** Our queues are self-ordering, generic queues.  A queue can contain
** any type of data.  This is accomplished through the use of intermediate
** nodes called qnodes, which contain a void* data member, allowing them
** to point to any type of integral data (integers, pointers, etc.).
** The qnode list is doubly-linked for ease of traversal.
**
** Each queue has associated with it a comparison function, which may be
** NULL.  Insertions into a queue are handled according to this function.
** If the function pointer is NULL, the queue is FIFO, and the insertion
** is always done at the end of the queue.  Otherwise, the insertion is
** ordered according to the results from the comparison function.
**
** Both of these types are visible to the rest of the system. We rely on
** the skill of those writing code for the rest of the system to not
** screw up the contents of a queue.
*/

#define	SP_KERNEL_SRC

#include "common.h"
#include "kernel.h"
#include "queues.h"

/*
** PRIVATE DEFINITIONS
*/

// how many qnodes can we get from a memory slice?
#define	N_QN_PER_SLICE	(SZ_SLICE / sizeof(qnode_t))

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// list of free qnodes
static qnode_t *_qn_freelist;

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

/**
** _qn_dealloc() - return a qnode to the free list
**
** Deallocates the supplied qnode
**
** @param qn   The qnode to be put on the free list
*/
static void _qn_dealloc( qnode_t *qn )
{
	// sanity check!
	assert1( qn != NULL );
	
	qn->next = _qn_freelist;
	_qn_freelist = qn;
}

/**
** Name:  _qn_extend
**
** Extend the set of available qnodes by allocating a slice
** of memory and carving it into qnode structures.
*/
static void _qn_extend( void )
{
	qnode_t *tmp;

	// allocate a slice of memory
	tmp = (qnode_t *) _km_slice_alloc();

	// iterate through it, deallocating each qnode
	for( int i = 0; i < N_QN_PER_SLICE; ++i, ++tmp ) {
		_qn_dealloc( tmp );
	}
}

/**
** Name:  _qn_alloc
**
** Allocate a qnode
**
** @return A pointer to the allocated node, or NULL
*/
static qnode_t *_qn_alloc( void )
{
	qnode_t *tmp;
	
	// if the list is empty, grab another slice and repopulate it
	if( _qn_freelist == NULL ) {
		_qn_extend();
	}
	
	// take the first node from the list
	tmp = _qn_freelist;
	_qn_freelist = tmp->next;

	// make sure we clean it out
	__memclr( tmp, sizeof(qnode_t) );
	
	return tmp;
}
	
/*
** PUBLIC FUNCTIONS
*/

/**
** Name:  _que_init
**
** Initialize the queue module.
*/
void _que_init( void )
{
	// reset the free list (just in case)
	_qn_freelist = NULL;

	// create the first set of qnodes for use
	_qn_extend();
	
	// all done!
	__cio_puts( " QUE" );
}

/**
** Name:  _que_create
**
** Create (reininitalize) a queue_t structure.
**
** @param q        The queue to be reinitialized
** @param compare  The ordering function for this queue, or NULL
*/
void _que_create( queue_t *q, compare_t compare )
{
	// sanity check
	assert( q != NULL );
	
	q->head = q->tail = NULL;
	q->length = 0;
	q->compare = compare;
}

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
status_t _que_insert( queue_t *q, void *data )
{
	// sanity check: q
	assert1( q != NULL );
	
	// allocate a qnode for our use
	qnode_t *qn = _qn_alloc();
	if( qn == NULL ) {
		return S_NOMEM;
	}
	
	// got one; populate it
	qn->data = data;
	
	/*
	** Three cases:
	**  1) queue is empty
	**  2) queue is fifo
	**  3) queue is ordered
	*/

	// if it's empty, this is the first entry

	if( QUE_IS_EMPTY(q) ) {
		q->head = q->tail = qn;
		q->length = 1;
		return S_OK;
	}
	
	// not empty, so we must find the insertion point
	
	if( QUE_IS_FIFO(q) ) {
	
		// not a FIFO queue, so just add at the end

		qn->prev = q->tail;
		q->tail->next = qn;
		q->tail = qn;
	
	} else {
	
		// OK, it's an ordered queue.

		qnode_t *prev = NULL;
		qnode_t *curr = q->head;
		
		/*
		** Loop until either we run off the end of the queue, OR
		** we find an entry that should come after the new value.
		**
		** compare(new,old):
		**   < 0 --> 'new' <  'old', so we stop
		**   = 0 --> 'new' == 'old', so continue
		**   > 0 --> 'new' >  'old', so continue
		*/

		while( curr != NULL && q->compare(data,curr->data) >= 0 ) {
			prev = curr;
			curr = curr->next;
		}
		
		// sanity check - both pointers can't be NULL
		assert1( !(prev == NULL && curr == NULL) );
		
		// set the backward and forward pointers for the new node

		qn->prev = prev;   // predecessor to the new node
		qn->next = curr;   // successor to the new node
		
		// connect the predecessor to this node
		if( prev == NULL ) {
			// insert at beginning
			q->head = qn;
		} else {
			prev->next = qn;
		}
		
		// connect the successor to this node
		if( curr == NULL ) {
			// append at end
			// prev->next = qn;
			q->tail = qn;
		} else {
			curr->prev = qn;
		}
				
	}
	
	// finally, indicate that we've extended the queue
	q->length += 1;

	return S_OK;
}

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
status_t _que_peek( queue_t *q, void **data )
{
	// sanity check: q != NULL, data != NULL
	assert1( q != NULL );
	assert1( data != NULL );
	
	// can't peek into an empty queue
	if( QUE_IS_EMPTY(q) ) {
		return S_EMPTY;
	}
	
	// there's at least one entry, so return its data field
	*data = q->head->data;

	return S_OK;
}

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
status_t _que_remove( queue_t *q, void **data )
{
	// sanity check q and data
	assert1( q != NULL );
	assert1( data != NULL );
	
	// can't remove from an empty queue
	if( QUE_IS_EMPTY(q) ) {
		return S_EMPTY;
	}
	
	// pull the first node out of the list
	qnode_t *qn = q->head;
	assert1( qn != NULL );
	
	// the successor is now the head node
	q->head = qn->next;

	if( q->head != NULL ) {

		// fix the predecessor pointer for the new head node
		q->head->prev = NULL;

		// and update the queue length
		q->length -= 1;

	} else {

		// no new head node? the list is now empty!
		assert1( q->length == 1 );
		q->tail = NULL;
		q->length = 0;

	}
	
	// return the data field
	*data = qn->data;

	// free the qnode
	_qn_dealloc( qn );
	
	return S_OK;
}

/**
** Name:  _que_remove_ptr
**
** Remove a specific entry from a queue. This is like _que_remove(),
** but we must locate the entry to be removed from the list.
**
** Also, we don't return the removed data field, because the caller
** must already have it (because it was supplied to use in the call).
**
** @param q     The queue to be manipulated
** @param data  The entry to be located and removed
**
** @return The removal status
*/
status_t _que_remove_ptr( queue_t *q, void *data )
{
	// sanity check q
	assert1( q != NULL );
	
	// empty queue?
	if( QUE_IS_EMPTY(q) ) {
		return S_EMPTY;
	}
	
	// find the victim
	qnode_t *qn = q->head;
	while( qn != NULL ) {
		if( data == qn->data ) {
			break;
		} else {
			qn = qn->next;
		}
	}
	
	// qn == NULL means we didn't find the entry
	
	if( qn == NULL ) {
		return S_NOTFOUND;
	}
	
	/*
	** OK, we found the entry, so we need to unlink it by making
	** the predecessor's 'next' link point to the successor of
	** the node we're removing, and the successor's 'prev' link
	** point to the predecessor node.
	**
	** The node being removed could be any of these:
	**   1) the first node in the queue
	**   2) a node in the middle of the queue
	**   3) the last node in the queue
	**
	** It could also be all three, if the queue only has one
	** thing in it.
	*/
	
	if( qn->prev == NULL ) {
		// case 1: we're removing the first node from the queue
		q->head = qn->next;
	} else {
		// case 2 or 3
		qn->prev->next = qn->next;
	}
	
	if( qn->next == NULL ) {
		// case 3: we're removing the last node
		q->tail = qn->prev;
	} else {
		// case 1 or 2
		qn->next->prev = qn->prev;
	}
	
	if( q->head == NULL ) {

		// the queue must now be empty
		assert1( q->tail == NULL );
		assert1( q->length == 1 );
		q->length = 0;

	} else {

		// there must have been at least 2 things in the queue,
		// so the queue cannot be empty now
		assert1( q->tail != NULL );
		assert1( q->length > 1 );
		q->length -= 1;

	}
	
	// free the qnode
	_qn_dealloc( qn );
	
	return S_OK;
}
