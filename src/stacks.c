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
#include "stacks.h"

#include "bootstrap.h"

// also need the prototype for fake_exit()
void fake_exit( void );

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** When doing static stack allocation, we preallocate N_PROCS + 1 stacks
** (one for each process, plus one for the kernel), and we preload the
** "free stacks" list with these. This allows _stk_alloc() and
** _stk_dealloc() to work the same way regardless of how the stacks are
** actually allocated.
*/

#ifdef STATIC_STACKS
static stack_t _stacks[N_PROCS+1];
#endif

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

/*
** Module initialization
*/

/**
** Name:	_stk_init()
**
** Initializes the stack module.
**
** Dependencies:
**    Cannot be called before kmem is initialized (if using dynamic alloc)
**    Must be called before interrupt handling has begun
**    Must be called before any process creation can be done
*/
void _stk_init( void )
{
	// reset the "free stacks" pool
	_free_stacks = NULL;

#ifdef STATIC_STACKS
	/*
	** For STATIC_STACKS, we have a fixed-sized pool of NPROCS+1
	** stack_t, and we start by putting them all on the free list.
	**
	** We don't need to do this if we're using dynamic storage.
	*/
	for( int i = 0; i < (N_PROCS + 1); ++i ) {
		_stk_dealloc( &_kstacks[i] );
	}
#endif

	// allocate the kernel stack
	_kstack = _stk_alloc();
	assert( _kstack != NULL );

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
** @return pointer to a "clean" stack, or NULL
*/
stack_t *_stk_alloc( void )
{
	uint32_t *new;

	if( _free_stacks == NULL ) {

#ifdef STATIC_STACKS
		// this is a serious problem if we're not using
		// dynamic allocation of stack space!
		assert( _free_stacks != NULL );
#endif

		// must allocate a new stack
		new = (uint32_t *) _km_page_alloc( PGS_PER_STACK );

	} else {

		// can re-use an existing stack
		new = _free_stacks;
		_free_stacks = (uint32_t *) *new;
	}

	// if we succeeded, clean up the space for the caller
	if( new != NULL ) {
		__memclr( new, sizeof(stack_t) );
	}

	// return the proper stack
	return (stack_t *) new;
}

/**
** _stk_dealloc() - deallocate a stack
**
** @param stk   The stack to be returned to the free list
*/
void _stk_dealloc( stack_t *stk )
{
	// sanity check
	assert1( stk != NULL );

	uint32_t *tmp = (uint32_t *) stk;

	// link it into the free list
	*tmp = (uint32_t) _free_stacks;
	_free_stacks = tmp;
}

/**
** _stk_setup - set up the stack for a new process
**
** @param stk    - The stack to be set up
** @param entry  - Entry point for the new process
** @param args   - Argument vector to be put in place
**
** @return A pointer to the context_t on the stack, or NULL
*/
context_t *_stk_setup( stack_t *stk, uint32_t entry, char *args[] )
{

	/*
	** We must duplicate the argv array here, because the first
	** thing we do before setting up the new stack for the user
	** is to clear out the old contents.  This is a potential
	** problem if we are called via the _sys_exec() system call,
	** because "args" is actually on the user stack that we're
	** about to clear!
	**
	** Figure out how many arguments & argument chars there are.
	*/

	int argbytes = 0;
	int argc = 0;

	for( argc = 0; args[argc] != NULL; ++argc ) {
		argbytes += __strlen( args[argc] ) + 1;
	}

	// Round up the byte count to the next multiple of four.
	argbytes = (argbytes + 3) & 0xfffffffc;

#if TRACING_STACK
	__cio_printf( "=== _stk_setup(%08x,%08x) %d args:",
			(uint32_t) stk, entry, argc );
	for( int i = 0; i < argc; ++i ) {
		__cio_printf( " '%s'", args[i] );
	}
	__cio_putchar( '\n' );
#endif

	/*
	** Allocate the arrays.  We are safe using dynamic arrays here
	** because we're using the OS stack, not the user stack.
	**
	** We want the argstrings and argv arrays to contain all zeroes.
	** The C standard states, in section 6.7.8, that
	**
	**   "21 If there are fewer initializers in a brace-enclosed list
	**       than there are elements or members of an aggregate, or
	**       fewer characters in a string literal used to initialize an
	**       array of known size than there are elements in the array,
	**       the remainder of the aggregate shall be initialized
	**       implicitly the same as objects that have static storage
	**       duration."
	**
	** Sadly, because we're using variable-sized arrays, we can't
	** rely on this, so we have to call __memclr() instead. :-(  In
	** truth, it doesn't really cost us much more time, but it's an
	** annoyance.
	*/

	char argstrings[ argbytes ];
	char *argv[ argc + 1 ];

	CLEAR( argstrings );
	CLEAR( argv );

	// Next, duplicate the argument strings, and create pointers to
	// each one in our argv.
	char *tmp = argstrings;
	for( int i = 0; i < argc; ++i ) {
		int nb = __strlen(args[i]) + 1; // bytes (incl. NUL) in this string
		__strcpy( tmp, args[i] );   // add to our buffer
		argv[i] = tmp;              // remember where it was
		tmp += nb;                  // move on
	}

	// trailing NULL pointer
	argv[argc] = NULL;

#if TRACING_STACK
	__cio_puts( "=== buffer: '" );
	for( int i = 0; i < argbytes; ++i ) {
		__put_char_or_code( argstrings[i] );
	}
	__cio_printf( "'\n=== _stk_setup, temp %d args (%d bytes):",
			argc, argbytes );
	for( int i = 0; i <= argc; ++i ) {
		__cio_printf( " [%d] ", i );
		if( argv[i] ) {
			__cio_printf( "'%s'", argv[i] );
		} else {
			__cio_puts( "NULL" );
		}
	}
	__cio_putchar( '\n' );
#endif

	// Now that we have duplicated the strings, we can clear out the
	// old contents of the stack.
	__memclr( stk, sizeof(stack_t) );

	/*
	** Set up the initial stack contents for a (new) user process.
	**
	** We reserve one longword at the bottom of the stack to hold a
	** pointer to where argv is on the stack.
	** 
	** Above that, we simulate a call from fake_exit() with an
	** argument vector by pushing the arguments and then the argument
	** count.  We follow this up by pushing the address of the entry point
	** of fake_exit() as a "return address".  Above that, we place a
	** context_t area that is initialized with the standard initial register
	** contents.
	**
	** The low end of the stack will contain these values:
	**
	**      esp ->  context      <- context save area
	**              ...          <- context save area
	**              context      <- context save area
	**              fake_exit    <- return address for faked call to main()
	**              argc         <- argument count for main()
	**         /->  argv         <- argv pointer for main()
	**         |     ...         <- argv array w/trailing NULL
	**         |     ...         <- argv character strings
	**         \--- ptr          <- last word in stack
	**
	** Stack alignment rules for the SysV ABI i386 supplement dictate that
	** the 'argc' parameter must be at an address that is a multiple of 16;
	** see below for more information.
	*/

	// Pointer to the last word in stack.
	uint32_t *ptr = ((uint32_t *)( stk + 1 )) - 1;

	// Pointer to where the arg strings should be filled in.
	char *strings = (char *) ( (uint32_t) ptr - argbytes );

	// back the pointer up to the nearest word boundary; because we're
	// moving toward location 0, the nearest word boundary is just the
	// next smaller address whose low-order two bits are zeroes
	strings = (char *) ((uint32_t) strings & 0xfffffffcU);

	// Copy over the argv strings.
	__memcpy( (void *)strings, argstrings, argbytes );

	/*
	** Next, we need to copy over the argv pointers.  Start by
	** determining where 'argc' should go.
	**
	** Stack alignment is controlled by the SysV ABI i386 supplement,
	** version 1.2 (June 23, 2016), which states in section 2.2.2:
	**
	**   "The end of the input argument area shall be aligned on a 16
	**   (32 or 64, if __m256 or __m512 is passed on stack) byte boundary.
	**   In other words, the value (%esp + 4) is always a multiple of 16
	**   (32 or 64) when control is transferred to the function entry
	**   point. The stack pointer, %esp, always points to the end of the
	**   latest allocated stack frame."
	**
	** Isn't technical documentation fun?  Ultimately, this means that
	** the first parameter to main() should be on the stack at an address
	** that is a multiple of 16.
	**
	** The space needed for argc, argv, and the argv array itself is
	** argc + 3 words (argc+1 for the argv entries, plus one word each
	** for argc and argv).  We back up that much from 'strings'.
	*/

	int nwords = argc + 3;
	uint32_t *acptr = ((uint32_t *) strings) - nwords;

	/*
	** Next, back up until we're at a multiple-of-16 address. Because we
	** are moving to a lower address, its upper 28 bits are identical to
	** the address we currently have, so we can do this with a bitwise
	** AND to just turn off the lower four bits.
	*/

	acptr = (uint32_t *) ( ((uint32_t)acptr) & 0xfffffff0 );

	// copy in 'argc'
	*acptr = argc;

	// next, 'argv', which follows 'argc'; 'argv' points to the
	// word that follows it in the stack
	uint32_t *avptr = acptr + 2;
	*(acptr+1) = (uint32_t) avptr;

	/*
	** Next, we copy in all argc+1 pointers.  This is complicated slightly
	** by the need to adjust the pointers; they currently point into the
	** local argstrings array.  We do this by adding the distance between
	** the start of the argstrings array and the duplicate of that data on
	** the stack.
	*/

	// Calculate the distance between the two argstring arrays.
	int32_t distance = strings - &argstrings[0];

	// Adjust and copy the string pointers.
	for( int i = 0; i <= argc; ++i ) {
		if( argv[i] != NULL ) {
			// an actual pointer - adjust it and copy it in
			*avptr = ((uint32_t) argv[i]) + distance;
		} else {
			// end of the line!
			*avptr = NULL;
		}
		++avptr;
	}

	/*
	** We now have to set up the stack so it looks like the user main()
	** function was called from a 'startup' function. We do this by
	** pushing a "return address" that points into our fake_exit() 
	** function; this ensures that a main() which just returns will go
	** back into code that will properly call exit().
	*/

	// return address will be pushed right above 'argc' on the stack
	avptr = acptr - 1;
	*avptr = (uint32_t) fake_exit;

	/*
	** Now, we need to set up the initial context for the executing
	** process.
	**
	** When this process is dispatched, the context restore code will
	** pop all the saved context information off the stack, leaving the
	** "return address" on the stack as if the main() for the process
	** had been "called" from the fake_exit() function.  When main()
	** returns, it will "return" to the entry point of fake_exit(),
	** which will then call exit().
	*/

	// Locate the context save area on the stack.
	context_t *ctx = ((context_t *) avptr) - 1;

	/*
	** We cleared the entire stack earlier, so all the context
	** fields currently contain zeroes.  We now need to fill in
	** all the important fields.
	*/

	ctx->eflags = DEFAULT_EFLAGS;    // IE enabled, PPL 0
	ctx->eip = entry;                // initial EIP
	ctx->cs = GDT_CODE;              // segment registers
	ctx->ss = GDT_STACK;
	ctx->ds = ctx->es = ctx->fs = ctx->gs = GDT_DATA;

	/*
	** Return the new context pointer to the caller.  It will be our
	** caller's responsibility to schedule this process.
	*/
	
	return( ctx );
}

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
