/*
** @file	stacks.c
**
** @author	CSCI-452 class of 20235
**
** @brief	Stack module implementation
**
** If compiled with the symbol STATIC_STACKS defined, this module
** uses a static array of stack_t, and selects the stack for a 
** process based on the PCB's position in the _processes array
** in the process module.
**
** If compiled without that symbol, this module dynamically allocates
** stacks for processes as needed, and keeps deallocated stacks in
** a free list for quick re-use.
*/

#define	SP_KERNEL_SRC

#include "common.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

#ifdef STATIC_STACKS

/*
*************************************
*************************************
** STATIC STACK VERSION            **
**                                 **
** This set of declarations are in **
** use if stack allocation is done **
** at compilation time.            **
*************************************
*************************************
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// the pool of stacks - one per possible process
static stack_t _stacks[N_PROCS];

/*
** PUBLIC GLOBAL VARIABLES
*/

// kernel stack
stack_t _kstack;

// kernel stack pointer
uint32_t *_kesp;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** Module initialization
*/

/**
** Name:	_stk_init()
**
** Initializes the stack module.
**
** Dependencies:
**    Must be called before interrupt handling has begun
**    Must be called before any process creation can be done
*/
void _stk_init( void )
{
	// reset the "free stacks" pool
	_free_stacks = NULL;

	// initial kernel stack pointer
	_kesp = ((uint32_t *)(_kstack + 1)) - 1;

	__cio_puts( " STK" );
}

/*
** Stack manipulation
*/

/**
** Name:	_stk_alloc()
**
** Allocate a stack.
**
** @param pcb   Pointer to the PCB that will "own" this stack
**
** @return pointer to a "clean" stack, or NULL
*/
stack_t *_stk_alloc( pcb_t *pcb )
{
	// sanity check
	assert1( pcb != NULL );

	// determine which stack to use
	int ix = pcb - _processes;

	// clean it up for the caller
	uint32_t *stk = &_processes[ix];

	__memclr( stk, sizeof(stack_t) );

	// return the proper stack
	return stk;
}

/**
** _stk_dealloc() - deallocate a stack
**
** @param stk   The stack to be returned to the free list
*/
void _stk_dealloc( uint32_t *stk )
{
	// not really much to do here!
}

#else

/*
*************************************
*************************************
** DYNAMIC STACK VERSION           **
**                                 **
** This set of declarations are in **
** use if stack allocation is done **
** at run time.                    **
*************************************
*************************************
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// free list of available stacks
static uint32_t *_free_stacks;

/*
** PUBLIC GLOBAL VARIABLES
*/

// kernel stack
stack_t *_kstack;

// kernel stack pointer
uint32_t *_kesp;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/**
** Name:	_stk_init()
**
** Initializes the stack module.
**
** Dependencies:
**    Cannot be called before kmem is initialized
**    Must be called before interrupt handling has begun
**    Must be called before any process creation can be done
*/
void _stk_init( void )
{
	// reset the "free stacks" pool
	_free_stacks = NULL;

	// kernel stack
	_kstack = _stk_alloc();
	assert( _kstack != NULL );

	// initial kernel stack pointer
	_kesp = ((uint32_t *)(_kstack + 1)) - 1;

	__cio_puts( " STK" );
}

/**
** Name:	_stk_alloc()
**
** Allocate a stack.
**
** @return pointer to a "clean" stack, or NULL
*/
stack_t *_stk_alloc( void )
{
	uint32_t *new;

	if( _free_stacks == NULL ) {

		// must allocate a new stack
		new = (uint32_t *) _km_page_alloc( PGS_PER_STACK );

	} else {

		// can re-use an existing one
		new = _free_stacks;
		_free_stacks = *new;

	}

	// if we succeeded, clear the space
	if( new != NULL ) {
		__memclr( new, sizeof(stack_t) );
	}

	// return whatever we got to the caller
	return new;
}

/**
** _stk_dealloc() - deallocate a stack
**
** @param stk   The stack to be returned to the free list
*/
void _stk_dealloc( uint32_t *stk )
{
	// sanity check
	assert1( stk != NULL );

	// link it into the free list
	*stk = _free_stacks;
	_free_stacks = stk;
}

#endif

#endif
// !SP_ASM_SRC

#endif
