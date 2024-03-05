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

#include "kernel.h"

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

/*
** When doing static stack allocation, we don't keep a
** free list of available stacks. Instead, we allocate
** one stack per PCB, and when a given PCB is allocated,
** we select the corresponding stack from the array of
** preallocated stacks.
*/

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
	(void) stk;
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

/*
** We keep a free list of stacks as a singly-linked list by making
** each stk[0] point to stk[0] in the next free stack.
*/

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
	_kstack = _stk_alloc( NULL );
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
** @param pcb   Pointer to the PCB that will "own" this stack (ignored)
**
** @return pointer to a "clean" stack, or NULL
*/
stack_t *_stk_alloc( pcb_t *pcb )
{
	uint32_t *new;

	// we don't use this, but keep it so that the stack
	// module interface is identical to that of the
	// static allocation version
	(void) pcb;

	if( _free_stacks == NULL ) {

		// must allocate a new stack
		new = (uint32_t *) _km_page_alloc( PGS_PER_STACK );

	} else {

		// can re-use an existing one
		new = _free_stacks;
		_free_stacks = (uint32_t *) *new;

	}

	// if we succeeded, clear the space
	if( new != NULL ) {
		__memclr( new, sizeof(stack_t) );
	}

	// return whatever we got to the caller
	return (stack_t *) new;
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
	*stk = (uint32_t) _free_stacks;
	_free_stacks = stk;
}

#endif
// STATIC_STACKS

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

// buffer sizes (rounded up a bit)
#define HBUFSZ      48
#define CBUFSZ      24

void _stk_dump( const char *msg, stack_t *stk, uint32_t limit )
{
	int words = STACK_WORDS;
	int eliding = 0;
	char oldbuf[HBUFSZ], buf[HBUFSZ], cbuf[CBUFSZ];
	uint32_t addr = (uint32_t ) stk;
	uint32_t *sp = (uint32_t *) stk;
	char hexdigits[] = "0123456789ABCDEF";

	// if a limit was specified, dump only that many words

	if( limit > 0 ) {
		words = limit;
		if( (words & 0x3) != 0 ) {
			// round up to a multiple of four
			words = (words & 0xfffffffc) + 4;
		}
		// skip to the new starting point
		sp += (STACK_WORDS - words);
		addr = (uint32_t) sp;
	}

	__cio_puts( "*** stack" );
	if( msg != NULL ) {
		__cio_printf( " (%s):\n", msg );
	} else {
		__cio_puts( ":\n" );
	}

	/**
	** Output lines begin with the 8-digit address, followed by a hex
	** interpretation then a character interpretation of four words:
	**
	** aaaaaaaa*..xxxxxxxx..xxxxxxxx..xxxxxxxx..xxxxxxxx..cccc.cccc.cccc.cccc
	**
	** Output lines that are identical except for the address are elided;
	** the next non-identical output line will have a '*' after the 8-digit
	** address field (where the '*' is in the example above).
	*/

	oldbuf[0] = '\0';

	while( words > 0 ) {
		register char *bp = buf;   // start of hex field
		register char *cp = cbuf;  // start of character field
		uint32_t start_addr = addr;

		// iterate through the words for this line

		for( int i = 0; i < 4; ++i ) {
			register uint32_t curr = *sp++;
			register uint32_t data = curr;

			// convert the hex representation

			// two spaces before each entry
			*bp++ = ' ';
			*bp++ = ' ';

			for( int j = 0; j < 8; ++j ) {
				uint32_t value = (data >> 28) & 0xf;
				*bp++ = hexdigits[value];
				data <<= 4;
			}

			// now, convert the character version
			data = curr;

			// one space before each entry
			*cp++ = ' ';

			for( int j = 0; j < 4; ++j ) {
				uint32_t value = (data >> 24) & 0xff;
				*cp++ = (value >= ' ' && value < 0x7f) ? (char) value : '.';
				data <<= 8;
			}
		}
		*bp = '\0';
		*cp = '\0';
		words -= 4;
		addr += 16;

		// if this line looks like the last one, skip it

		if( __strcmp(oldbuf,buf) == 0 ) {
			++eliding;
			continue;
		}

		// it's different, so print it

		// start with the address
		__cio_printf( "%08x%c", start_addr, eliding ? '*' : ' ' );
		eliding = 0;

		// print the words
		__cio_printf( "%s %s\n", buf, cbuf );

		// remember this line
		__memcpy( (uint8_t *) oldbuf, (uint8_t *) buf, HBUFSZ );
	}
}
