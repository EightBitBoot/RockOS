/*
** @file	stacks.h
**
** @author	CSCI-452 class of 20235
**
** @brief	Stack module declarations
**
** If compiled with the symbol STATIC_STACKS defined, this module
** uses a static array of stack_t, and selects the stack for a
** process based on the PCB's position in the _processes array
** in the process module.
**
** If compiled without that symbol, this module dynamically allocates
** stacks for processes as needed, and keeps deallocated stacks in
** a free list for quick re-use.
**
** Note: the dependencies for the non-static version are different
** due to its need for working dynamic storage.
*/

#ifndef STACKS_H_
#define STACKS_H_

#include "common.h"

#include "kmem.h"

/*
** General (C and/or assembly) definitions
*/

// stacks are 16KB
#define	PGS_PER_STACK	4
#define	SZ_STACK		(PGS_PER_STACK * SZ_PAGE)
#define	STACK_WORDS		(SZ_STACK / sizeof(uint32_t))

#ifndef SP_ASM_SRC

/*
** Start of C-only definitions
*/

/*
** Types
*/

// our stack type
typedef uint32_t stack_t[STACK_WORDS];

// NOW we can include procs.h to get pcb_t

#include "procs.h"

/*
** Globals
*/

/*
** Prototypes
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
**    Cannot be called before kmem is initialized (dynamic allocation)
**    Must be called before interrupt handling has begun
**    Must be called before any process creation can be done
*/
void _stk_init( void );

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
stack_t *_stk_alloc( pcb_t *pcb );

/**
** _stk_dealloc() - deallocate a stack
**
** @param stk   The stack to be returned to the free list
*/
void _stk_dealloc( uint32_t *stk );

/**
** _stk_dump(msg,stk,lim)
**
** Dumps the contents of a stack to the console.  Assumes the stack
** is a multiple of four words in length.
**
** @param msg   An optional message to print before the dump
** @param stk   The stack to dump out
** @param lim   Limit on the number of words to dump (0 for all)
*/

void _stk_dump( const char *msg, stack_t *stk, uint32_t limit );

#endif
// !SP_ASM_SRC

#endif
